open Migrate_parsetree;
open Graphql_ppx_base;
open Result_structure;
open Generator_utils;
open Ast_408;
open Asttypes;
open Parsetree;
open Ast_helper;
open Extract_type_definitions;
open Output_bucklescript_utils;

type operation_type = Graphql_ast.operation_type;
type operation_options = {has_required_variables: bool};

type definition =
  | Fragment
  | Operation(operation_type, operation_options);

module StringSet = Set.Make(String);
module VariableFinderImpl = {
  type t = ref(StringSet.t);
  let make_self = _ => ref(StringSet.empty);

  include Traversal_utils.AbstractVisitor;

  let enter_variable_value = (self, _, v) =>
    self := StringSet.add(v.Source_pos.item, self^);

  let from_self = (self: t): StringSet.t => self^;
};

module VariableFinder = Traversal_utils.Visitor(VariableFinderImpl);

let find_variables = (config, document) => {
  let ctx = Traversal_utils.make_context(config, document);
  VariableFinderImpl.from_self(VariableFinder.visit_document(ctx, document));
};

let pretty_print = (query: string): string => {
  let indent = ref(1);
  let str =
    query
    |> String.split_on_char('\n')
    |> List.map(l => String.trim(l))
    |> List.filter(l => l != "")
    |> List.map(line => {
         let prevIndent = indent^;
         String.iter(
           fun
           | '{' => indent := indent^ + 1
           | '}' => indent := indent^ - 1
           | _ => (),
           line,
         );

         let currIndent = prevIndent < indent^ ? prevIndent : indent^;
         let currIndent = currIndent < 1 ? 1 : currIndent;
         let line = String.make(currIndent * 2, ' ') ++ line;
         line;
       })
    |> String.concat("\n");
  str ++ "\n";
};

let compress_parts = (parts: array(Graphql_printer.t)) => {
  Graphql_printer.(
    parts
    |> Array.to_list
    |> List.fold_left(
         (acc, curr) => {
           switch (acc, curr) {
           | ([String(s1), ...rest], String(s2)) => [
               String(s1 ++ s2),
               ...rest,
             ]
           | (acc, curr) => [curr, ...acc]
           }
         },
         [],
       )
    |> List.rev
    |> Array.of_list
  );
};

let make_fragment_query = f => {
  Exp.ident({
    Location.txt: Longident.parse(f ++ ".query"),
    loc: Location.none,
  });
};

let emit_printed_template_query = (parts: array(Graphql_printer.t), config) => {
  open Graphql_printer;

  let fragment_query_refs =
    switch (config.fragment_in_query) {
    | Include =>
      parts
      |> Array.fold_left(
           acc =>
             fun
             | String(_) => acc
             | FragmentNameRef(_) => acc
             | FragmentQueryRef(f) => [f, ...acc],
           [],
         )
      |> List.rev
    | Exclude => []
    };

  let query =
    Array.fold_left(
      acc =>
        fun
        | String(s) => acc ++ s
        | FragmentNameRef(f) => {
            acc ++ f;
          }
        | FragmentQueryRef(_) => acc,
      "",
      parts,
    );
  let fragment_names =
    fragment_query_refs
    |> List.mapi((i, _frag) => "frag_" ++ string_of_int(i));
  let fragments = fragment_names |> List.map(name => {"${" ++ name ++ "}\n"});

  [query, ...fragments] |> List.fold_left((acc, el) => acc ++ el, "");
};

let emit_printed_query = (parts, config) => {
  open Graphql_printer;
  let make_string = s => {
    Exp.constant(Parsetree.Pconst_string(s, None));
  };
  let join = (part1, part2) => {
    Ast_helper.(
      Exp.apply(
        Exp.ident({Location.txt: Longident.parse("^"), loc: Location.none}),
        [(Nolabel, part1), (Nolabel, part2)],
      )
    );
  };

  let query =
    Array.fold_left(
      acc =>
        fun
        | String(s) => acc ++ s
        | FragmentNameRef(f) => acc ++ f
        | FragmentQueryRef(_) => acc,
      "",
      parts,
    );

  let fragment_query_refs =
    switch (config.fragment_in_query) {
    | Include =>
      parts
      |> Array.fold_left(
           acc =>
             fun
             | String(_) => acc
             | FragmentNameRef(_) => acc
             | FragmentQueryRef(f) => [make_fragment_query(f), ...acc],
           [],
         )
      |> List.rev
    | Exclude => []
    };

  fragment_query_refs
  |> List.fold_left((acc, el) => join(acc, el), make_string(query));
};

let rec emit_json = config =>
  Ast_408.(
    fun
    | `Assoc(vs) => {
        let pairs =
          Ast_helper.(
            Exp.array(
              vs
              |> List.map(((key, value)) =>
                   Exp.tuple([
                     Exp.constant(Pconst_string(key, None)),
                     emit_json(config, value),
                   ])
                 ),
            )
          );
        %expr
        Js.Json.object_(Js.Dict.fromArray([%e pairs]));
      }
    | `List(ls) => {
        let values = Ast_helper.Exp.array(List.map(emit_json(config), ls));
        %expr
        Js.Json.array([%e values]);
      }
    | `Bool(true) => [%expr Js.Json.boolean(true)]
    | `Bool(false) => [%expr Js.Json.boolean(false)]
    | `Null => [%expr Obj.magic(Js.Undefined.empty)]
    | `String(s) => [%expr
        Js.Json.string([%e Ast_helper.Exp.constant(Pconst_string(s, None))])
      ]
    | `Int(i) => [%expr
        Js.Json.number(
          [%e
            Ast_helper.Exp.constant(Pconst_float(string_of_int(i), None))
          ],
        )
      ]
    | `StringExpr(parts) => [%expr
        Js.Json.string([%e emit_printed_query(parts, config)])
      ]
  );

let wrap_template_tag = (~import=?, ~location=?, ~template_tag=?, source) => {
  switch (import, location, template_tag) {
  | (None, Some(location), _)
  | (Some("default"), Some(location), _) =>
    "require(\"" ++ location ++ "\")" ++ "`\n" ++ source ++ "`"
  | (Some(import), Some(location), _) =>
    "require(\"" ++ location ++ "\")." ++ import ++ "`\n" ++ source ++ "`"
  | (_, _, Some(template_tag)) => template_tag ++ "`\n" ++ source ++ "`"
  | _ => source
  };
};

let wrap_structure_raw = contents => {
  Str.extension((
    {txt: "raw", loc: Location.none},
    PStr([
      {
        pstr_desc:
          Pstr_eval(
            Exp.constant(Parsetree.Pconst_string(contents, None)),
            [],
          ),
        pstr_loc: Location.none,
      },
    ]),
  ));
};

let constraint_on_type = (exp, type_name) => {
  Exp.constraint_(
    exp,
    base_type_name(
      switch (type_name) {
      | None => "string"
      | Some(type_name) => type_name
      },
    ),
  );
};

let wrap_raw = contents => {
  Exp.extension((
    {txt: "raw", loc: Location.none},
    PStr([
      {
        pstr_desc:
          Pstr_eval(
            Exp.constant(Parsetree.Pconst_string(contents, None)),
            [],
          ),
        pstr_loc: Location.none,
      },
    ]),
  ));
};

let make_printed_query = (config, document) => {
  let source = Graphql_printer.print_document(config.schema, document);
  switch (Ppx_config.output_mode()) {
  | Ppx_config.Apollo_AST =>
    Ast_serializer_apollo.serialize_document(source, document)
    |> emit_json(config)

  | Ppx_config.String =>
    switch (config.template_tag) {
    | (template_tag, location, import)
        when template_tag != None || location != None =>
      open Graphql_printer;
      // the only way to emit a template literal for now, using the bs.raw
      // extension
      let fragments =
        source
        |> Array.fold_left(
             acc =>
               fun
               | String(_) => acc
               | FragmentNameRef(_) => acc
               | FragmentQueryRef(f) => [f, ...acc],
             [],
           )
        |> List.rev;

      let template_tag =
        wrap_template_tag(
          ~template_tag?,
          ~location?,
          ~import?,
          pretty_print(emit_printed_template_query(source, config)),
        );

      constraint_on_type(
        switch (config.fragment_in_query, fragments) {
        | (Exclude, _)
        | (_, []) => wrap_raw(template_tag)
        | (Include, fragments) =>
          let fragment_names =
            fragments |> List.mapi((i, _frag) => "frag_" ++ string_of_int(i));
          let frag_fun =
            "("
            ++ (
              List.tl(fragment_names)
              |> List.fold_left(
                   (acc, el) => acc ++ ", " ++ el,
                   List.hd(fragment_names),
                 )
            )
            ++ ") => ";
          Exp.apply(
            wrap_raw(frag_fun ++ template_tag),
            fragments |> List.map(f => (Nolabel, make_fragment_query(f))),
          );
        },
        config.template_tag_return_type,
      );

    | (_, _, _) => emit_printed_query(source, config)
    }
  };
};

let signature_module = (name, signature) => {
  {
    psig_loc: Location.none,
    psig_desc:
      Psig_module({
        pmd_loc: Location.none,
        pmd_name: {
          txt: Generator_utils.capitalize_ascii(name),
          loc: Location.none,
        },
        pmd_type: Mty.signature(signature),
        pmd_attributes: [],
      }),
  };
};

let wrap_module = (~loc as _, ~module_type=?, name: string, contents) => {
  let loc = Location.none;
  {
    pstr_desc:
      Pstr_module({
        pmb_name: {
          txt: Generator_utils.capitalize_ascii(name),
          loc,
        },
        pmb_expr: {
          switch (module_type) {
          | Some(module_type) => {
              pmod_desc:
                Pmod_constraint(Mod.structure(contents), module_type),
              pmod_loc: loc,
              pmod_attributes: [],
            }
          | None => Mod.structure(contents)
          };
        },

        pmb_attributes: [],
        pmb_loc: loc,
      }),
    pstr_loc: loc,
  };
};

let wrap_query_module =
    (~loc as _, ~module_type=?, definition, name, contents, config) => {
  let loc = Location.none;
  let module_name = "Inner";
  let funct =
    switch (config.extend) {
    | Some(funct) => Some(funct)
    | None =>
      switch (definition) {
      | Fragment => Ppx_config.extend_fragment()
      | Operation(Query, {has_required_variables: false}) =>
        switch (Ppx_config.extend_query_no_required_variables()) {
        | Some(extension) => Some(extension)
        | None => Ppx_config.extend_query()
        }
      | Operation(Query, _) => Ppx_config.extend_query()
      | Operation(Mutation, {has_required_variables: false}) =>
        switch (Ppx_config.extend_mutation_no_required_variables()) {
        | Some(extension) => Some(extension)
        | None => Ppx_config.extend_mutation()
        }
      | Operation(Mutation, _) => Ppx_config.extend_mutation()
      | Operation(Subscription, {has_required_variables: false}) =>
        switch (Ppx_config.extend_subscription_no_required_variables()) {
        | Some(extension) => Some(extension)
        | None => Ppx_config.extend_subscription()
        }
      | Operation(Subscription, _) => Ppx_config.extend_subscription()
      }
    };

  let contents =
    switch (funct) {
    | Some(funct) => [
        wrap_module(~loc, module_name, contents),
        Str.include_(
          Incl.mk(Mod.ident({txt: Longident.parse(module_name), loc})),
        ),
        Str.include_(
          Incl.mk(
            Mod.apply(
              Mod.ident({txt: Longident.parse(funct), loc}),
              Mod.ident({txt: Longident.parse(module_name), loc}),
            ),
          ),
        ),
      ]
    | None => contents
    };

  switch (name) {
  | Some(name) => [wrap_module(~module_type?, ~loc, name, contents)]
  | None => contents
  };
};

let generate_operation_interface = (config, variable_defs, res_structure) => {
  let raw_types =
    Output_bucklescript_types.generate_type_signature_items(
      config,
      res_structure,
      true,
      None,
      None,
    );
  let types =
    Output_bucklescript_types.generate_type_signature_items(
      config,
      res_structure,
      false,
      None,
      None,
    );
  let raw_arg_types =
    Output_bucklescript_types.generate_arg_type_signature_items(
      true,
      config,
      variable_defs,
    );
  let arg_types =
    Output_bucklescript_types.generate_arg_type_signature_items(
      false,
      config,
      variable_defs,
    );
  let extracted_args = extract_args(config, variable_defs);
  let serialize_variable_signatures =
    Output_bucklescript_serializer.generate_serialize_variables_signature(
      extracted_args,
    );

  [
    [signature_module("Raw", List.append(raw_types, raw_arg_types))],
    types,
    arg_types,
    [
      [%sigi:
        let query: [%t
          base_type_name(
            switch (config.template_tag_return_type) {
            | Some(return_type) => return_type
            | None => "string"
            },
          )
        ]
      ],
      [%sigi: let parse: Raw.t => t],
      [%sigi: let serialize: t => Raw.t],
    ],
    serialize_variable_signatures,
  ]
  |> List.concat;
};

let generate_operation_implementation =
    (config, variable_defs, has_error, operation, res_structure) => {
  Output_bucklescript_docstrings.reset();
  let parse_fn =
    Output_bucklescript_parser.generate_parser(
      config,
      [],
      Graphql_ast.Operation(operation),
      res_structure,
    );
  let serialize_fn =
    Output_bucklescript_serializer.generate_serializer(
      config,
      [],
      Graphql_ast.Operation(operation),
      None,
      res_structure,
    );
  let types =
    Output_bucklescript_types.generate_type_structure_items(
      config,
      res_structure,
      false,
      None,
      None,
    );
  // Add to internal module
  Output_bucklescript_docstrings.for_operation(config, operation);
  let raw_types =
    Output_bucklescript_types.generate_type_structure_items(
      config,
      res_structure,
      true,
      None,
      None,
    );
  let arg_types =
    Output_bucklescript_types.generate_arg_type_structure_items(
      false,
      config,
      variable_defs,
    );
  let raw_arg_types =
    Output_bucklescript_types.generate_arg_type_structure_items(
      true,
      config,
      variable_defs,
    );
  let extracted_args = extract_args(config, variable_defs);
  let serialize_variable_functions =
    Output_bucklescript_serializer.generate_serialize_variables(
      config,
      extracted_args,
    );
  let variable_constructors =
    Output_bucklescript_serializer.generate_variable_constructors(
      config,
      extracted_args,
    );
  let has_required_variables = has_required_variables(extracted_args);

  let contents =
    if (has_error) {
      [[%stri let parse: Raw.t => t = value => [%e parse_fn]]];
    } else {
      let printed_query =
        make_printed_query(config, [Graphql_ast.Operation(operation)]);

      List.concat([
        List.concat([
          [[%stri [@ocaml.warning "-32"]]],
          [
            wrap_module(
              ~loc=Location.none,
              "Raw",
              List.append(raw_types, raw_arg_types),
            ),
          ],
          types,
          arg_types,
          [
            Output_bucklescript_docstrings.(
              make_let("query", printed_query, query_docstring)
            ),
          ],
          [
            Output_bucklescript_docstrings.(
              make_let(
                "parse",
                [%expr (value: Raw.t) => ([%e parse_fn]: t)],
                parse_docstring,
              )
            ),
          ],
          [
            Output_bucklescript_docstrings.(
              make_let(
                "serialize",
                [%expr (value: t) => ([%e serialize_fn]: Raw.t)],
                serialize_docstring,
              )
            ),
          ],
          [serialize_variable_functions],
          switch (variable_constructors) {
          | None => [[%stri let makeVariables = () => ()]]
          | Some(c) => [c]
          },
          has_required_variables
            ? [] : [[%stri let makeDefaultVariables = () => makeVariables()]],
          [
            [%stri external unsafe_fromJson: Js.Json.t => Raw.t = "%identity"],
          ],
          [[%stri external toJson: Raw.t => Js.Json.t = "%identity"]],
          [
            [%stri
              external variablesToJson: Raw.t_variables => Js.Json.t =
                "%identity"
            ],
          ],
          Output_bucklescript_docstrings.get_module(),
        ]),
      ]);
    };

  let name =
    switch (operation) {
    | {item: {o_name: Some({item: name})}} => Some(name)
    | _ => None
    };
  (
    Operation(
      operation.item.o_type,
      {has_required_variables: has_required_variables},
    ),
    name,
    contents,
    operation.span |> config.map_loc |> conv_loc,
  );
};

let generate_fragment_module =
    (
      config,
      name,
      required_variables,
      has_error,
      fragment,
      type_name,
      res_structure,
    ) => {
  Output_bucklescript_docstrings.reset();
  let parse_fn =
    Output_bucklescript_parser.generate_parser(
      config,
      [],
      Graphql_ast.Fragment(fragment),
      res_structure,
    );
  let serialize_fn =
    Output_bucklescript_serializer.generate_serializer(
      config,
      [],
      Graphql_ast.Fragment(fragment),
      None,
      res_structure,
    );
  let types =
    Output_bucklescript_types.generate_type_structure_items(
      config,
      res_structure,
      false,
      type_name,
      Some((
        fragment.item.fg_type_condition.item,
        fragment.item.fg_name.span,
      )),
    );
  // Add to internal module
  Output_bucklescript_docstrings.for_fragment_root(config, fragment);
  let raw_types =
    Output_bucklescript_types.generate_type_structure_items(
      config,
      res_structure,
      true,
      None,
      Some((
        fragment.item.fg_type_condition.item,
        fragment.item.fg_name.span,
      )),
    );

  let rec make_labeled_fun = body =>
    fun
    | [] => body
    | [(name, type_, span, type_span), ...tl] => {
        let loc = config.map_loc(span) |> conv_loc;
        let type_loc = config.map_loc(type_span) |> conv_loc;
        Ast_helper.(
          Exp.fun_(
            ~loc,
            Labelled(name),
            None,
            Pat.constraint_(
              Pat.var({txt: "_" ++ name, loc: type_loc}),
              Typ.variant(
                [
                  {
                    prf_desc:
                      Rtag(
                        {
                          txt:
                            Output_bucklescript_parser.type_name_to_words(
                              type_,
                            ),
                          loc: type_loc,
                        },
                        true,
                        [],
                      ),
                    prf_loc: type_loc,
                    prf_attributes: [],
                  },
                ],
                Closed,
                None,
              ),
            ),
            make_labeled_fun(body, tl),
          )
        );
      };

  let contents =
    if (has_error) {
      [[%stri let parse = (_vars, value) => [%e parse_fn]]];
    } else {
      let printed_query =
        make_printed_query(config, [Graphql_ast.Fragment(fragment)]);
      let verify_parse =
        make_labeled_fun(
          Exp.fun_(
            Labelled("fragmentName"),
            None,
            Pat.constraint_(
              Pat.var({txt: "_" ++ name, loc: Location.none}),
              Typ.variant(
                [
                  {
                    prf_desc: Rtag({txt: name, loc: Location.none}, true, []),
                    prf_loc: Location.none,
                    prf_attributes: [],
                  },
                ],
                Closed,
                None,
              ),
            ),
            [%expr (value: Raw.t) => parse(value)],
          ),
          required_variables,
        );

      let type_name = base_type_name(Option.get_or_else("t", type_name));

      // Add to internal module
      Output_bucklescript_docstrings.for_fragment(config, fragment);
      List.concat(
        List.concat([
          [
            [[%stri [@ocaml.warning "-32"]]],
            [wrap_module(~loc=Location.none, "Raw", raw_types)],
            types,
            [
              Output_bucklescript_docstrings.(
                make_let("query", printed_query, query_docstring)
              ),
            ],
            [
              Output_bucklescript_docstrings.(
                make_let(
                  "parse",
                  [%expr (value: Raw.t) => ([%e parse_fn]: [%t type_name])],
                  parse_docstring,
                )
              ),
            ],
            [@metaloc fragment.span |> config.map_loc |> conv_loc]
            [[%stri let verifyArgsAndParse = [%e verify_parse]]],
            [
              Output_bucklescript_docstrings.(
                make_let(
                  "serialize",
                  [%expr
                    (value: [%t type_name]) => ([%e serialize_fn]: Raw.t)
                  ],
                  serialize_docstring,
                )
              ),
            ],
            [
              [%stri
                let verifyName = [%e
                  Ast_helper.(
                    Exp.function_([
                      Exp.case(
                        Pat.variant(name, None),
                        Exp.construct(
                          {txt: Longident.Lident("()"), loc: Location.none},
                          None,
                        ),
                      ),
                    ])
                  )
                ]
              ],
            ],
            [
              [%stri
                external unsafe_fromJson: Js.Json.t => Raw.t = "%identity"
              ],
            ],
            [[%stri external toJson: Raw.t => Js.Json.t = "%identity"]],
            Output_bucklescript_docstrings.get_module(),
          ],
        ]),
      );
    };

  (
    Fragment,
    Some(Generator_utils.capitalize_ascii(name)),
    contents,
    fragment.span |> config.map_loc |> conv_loc,
  );
};

let generate_definition = config =>
  fun
  | Def_operation({
      variable_definitions: vdefs,
      has_error,
      operation,
      inner: structure,
    }) =>
    generate_operation_implementation(
      config,
      vdefs,
      has_error,
      operation,
      structure,
    )
  | Def_fragment({
      name,
      req_vars,
      has_error,
      fragment,
      type_name,
      inner: structure,
    }) =>
    generate_fragment_module(
      config,
      name,
      req_vars,
      has_error,
      fragment,
      type_name,
      structure,
    );

let generate_modules = (config, module_name, module_type, operations) => {
  switch (operations) {
  | [] => []
  | [operation] =>
    switch (generate_definition(config, operation)) {
    | (definition, Some(name), contents, loc) =>
      wrap_query_module(
        ~loc,
        ~module_type?,
        definition,
        switch (config.inline, module_name) {
        | (true, _) => None
        | (_, Some(name)) => Some(name)
        | (_, None) => Some(name)
        },
        contents,
        config,
      )
    | (definition, None, contents, loc) =>
      wrap_query_module(
        ~loc,
        ~module_type?,
        definition,
        module_name,
        contents,
        config,
      )
    }

  | operations =>
    let contents =
      operations
      |> List.map(generate_definition(config))
      |> List.mapi((i, (definition, name, contents, loc)) =>
           switch (name) {
           | Some(name) =>
             wrap_query_module(
               ~loc,
               definition,
               Some(name),
               contents,
               config,
             )

           | None =>
             wrap_query_module(
               ~loc,
               definition,
               Some("Untitled" ++ string_of_int(i)),
               contents,
               config,
             )
           }
         )
      |> List.concat;
    switch (module_name) {
    | Some(module_name) => [
        wrap_module(~module_type?, ~loc=Location.none, module_name, contents),
      ]
    | None => contents
    };
  };
};
