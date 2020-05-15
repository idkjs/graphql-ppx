// Generated by BUCKLESCRIPT, PLEASE EDIT WITH CARE
'use strict';


var Raw = { };

var query = "query   {\nvariousScalars  {\nstring  \nint  \n}\n\n}\n";

function parse(value) {
  var value$1 = value.variousScalars;
  var value$2 = value$1.int;
  var value$3 = value$1.string;
  return {
          variousScalars: {
            string: value$3,
            int: value$2
          }
        };
}

function serialize(value) {
  var value$1 = value.variousScalars;
  var value$2 = value$1.int;
  var value$3 = value$1.string;
  return {
          variousScalars: {
            string: value$3,
            int: value$2
          }
        };
}

var Z__INTERNAL = {
  graphql_module: 0
};

var MyQuery$prime = {
  Raw: Raw,
  query: query,
  parse: parse,
  serialize: serialize,
  Z__INTERNAL: Z__INTERNAL
};

var MyQuery = {
  Raw: Raw,
  query: query,
  parse: parse,
  serialize: serialize,
  Z__INTERNAL: Z__INTERNAL,
  self: MyQuery$prime
};

var Raw$1 = { };

var query$1 = "query   {\nvariousScalars  {\nnullableString  \n}\n\n}\n";

function parse$1(value) {
  var value$1 = value.variousScalars;
  var value$2 = value$1.nullableString;
  var nullableString = (value$2 == null) ? undefined : value$2;
  return {
          variousScalars: {
            nullableString: nullableString
          }
        };
}

function serialize$1(value) {
  var value$1 = value.variousScalars;
  var value$2 = value$1.nullableString;
  var nullableString = value$2 !== undefined ? value$2 : null;
  return {
          variousScalars: {
            nullableString: nullableString
          }
        };
}

var Z__INTERNAL$1 = {
  graphql_module: 0
};

var OneFieldQuery$prime = {
  Raw: Raw$1,
  query: query$1,
  parse: parse$1,
  serialize: serialize$1,
  Z__INTERNAL: Z__INTERNAL$1
};

var OneFieldQuery = {
  Raw: Raw$1,
  query: query$1,
  parse: parse$1,
  serialize: serialize$1,
  Z__INTERNAL: Z__INTERNAL$1,
  self: OneFieldQuery$prime
};

var query$2 = "fragment Fragment on VariousScalars   {\nstring  \nint  \n}\n";

var Raw$2 = { };

function parse$2(value) {
  var value$1 = value.int;
  var value$2 = value.string;
  return {
          string: value$2,
          int: value$1
        };
}

function serialize$2(value) {
  var value$1 = value.int;
  var value$2 = value.string;
  return {
          string: value$2,
          int: value$1
        };
}

var name = "Fragment";

var Z__INTERNAL$2 = {
  graphql: 0,
  graphql_module: 0
};

var Fragment$prime = {
  query: query$2,
  Raw: Raw$2,
  parse: parse$2,
  serialize: serialize$2,
  name: name,
  Z__INTERNAL: Z__INTERNAL$2
};

var Fragment = {
  query: query$2,
  Raw: Raw$2,
  parse: parse$2,
  serialize: serialize$2,
  name: name,
  Z__INTERNAL: Z__INTERNAL$2,
  self: Fragment$prime
};

var Raw$3 = { };

var query$3 = "query   {\nvariousScalars  {\n...Fragment   \n}\n\n}\nfragment Fragment on VariousScalars   {\nstring  \nint  \n}\n";

function parse$3(value) {
  var value$1 = value.variousScalars;
  return {
          variousScalars: parse$2(value$1)
        };
}

function serialize$3(value) {
  var value$1 = value.variousScalars;
  return {
          variousScalars: serialize$2(value$1)
        };
}

var Z__INTERNAL$3 = {
  graphql_module: 0
};

var Untitled1$prime = {
  Raw: Raw$3,
  query: query$3,
  parse: parse$3,
  serialize: serialize$3,
  Z__INTERNAL: Z__INTERNAL$3
};

var Untitled1 = {
  Raw: Raw$3,
  query: query$3,
  parse: parse$3,
  serialize: serialize$3,
  Z__INTERNAL: Z__INTERNAL$3,
  self: Untitled1$prime
};

var ExternalFragmentQuery$prime = {
  Fragment$prime: Fragment$prime,
  Fragment: Fragment,
  Untitled1$prime: Untitled1$prime,
  Untitled1: Untitled1
};

var ExternalFragmentQuery = {
  Fragment$prime: Fragment$prime,
  Fragment: Fragment,
  Untitled1$prime: Untitled1$prime,
  Untitled1: Untitled1,
  self: ExternalFragmentQuery$prime
};

var Raw$4 = { };

var query$4 = "query   {\ndogOrHuman  {\n__typename\n...on Dog   {\nname  \nbarkVolume  \n}\n\n}\n\n}\n";

function parse$4(value) {
  var value$1 = value.dogOrHuman;
  var typename = value$1["__typename"];
  var tmp;
  if (typename === "Dog") {
    var value$2 = value$1.barkVolume;
    var value$3 = value$1.name;
    tmp = /* `Dog */[
      3406428,
      {
        name: value$3,
        barkVolume: value$2
      }
    ];
  } else {
    tmp = /* `FutureAddedValue */[
      -31101740,
      value$1
    ];
  }
  return {
          dogOrHuman: tmp
        };
}

function serialize$4(value) {
  var value$1 = value.dogOrHuman;
  var tmp;
  if (value$1[0] >= 3406428) {
    var value$2 = value$1[1];
    var value$3 = value$2.barkVolume;
    var value$4 = value$2.name;
    tmp = {
      __typename: "Dog",
      name: value$4,
      barkVolume: value$3
    };
  } else {
    tmp = value$1[1];
  }
  return {
          dogOrHuman: tmp
        };
}

var Z__INTERNAL$4 = {
  graphql_module: 0
};

var InlineFragmentQuery$prime = {
  Raw: Raw$4,
  query: query$4,
  parse: parse$4,
  serialize: serialize$4,
  Z__INTERNAL: Z__INTERNAL$4
};

var InlineFragmentQuery = {
  Raw: Raw$4,
  query: query$4,
  parse: parse$4,
  serialize: serialize$4,
  Z__INTERNAL: Z__INTERNAL$4,
  self: InlineFragmentQuery$prime
};

var query$5 = "fragment DogFragment on Dog   {\nname  \nbarkVolume  \n}\n";

var Raw$5 = { };

function parse$5(value) {
  var value$1 = value.barkVolume;
  var value$2 = value.name;
  return {
          name: value$2,
          barkVolume: value$1
        };
}

function serialize$5(value) {
  var value$1 = value.barkVolume;
  var value$2 = value.name;
  return {
          name: value$2,
          barkVolume: value$1
        };
}

var name$1 = "DogFragment";

var Z__INTERNAL$5 = {
  graphql: 0,
  graphql_module: 0
};

var DogFragment$prime = {
  query: query$5,
  Raw: Raw$5,
  parse: parse$5,
  serialize: serialize$5,
  name: name$1,
  Z__INTERNAL: Z__INTERNAL$5
};

var DogFragment = {
  query: query$5,
  Raw: Raw$5,
  parse: parse$5,
  serialize: serialize$5,
  name: name$1,
  Z__INTERNAL: Z__INTERNAL$5,
  self: DogFragment$prime
};

var Raw$6 = { };

var query$6 = "query   {\ndogOrHuman  {\n__typename\n...on Dog   {\n...DogFragment   \n}\n\n}\n\n}\nfragment DogFragment on Dog   {\nname  \nbarkVolume  \n}\n";

function parse$6(value) {
  var value$1 = value.dogOrHuman;
  var typename = value$1["__typename"];
  var tmp = typename === "Dog" ? /* `Dog */[
      3406428,
      parse$5(value$1)
    ] : /* `FutureAddedValue */[
      -31101740,
      value$1
    ];
  return {
          dogOrHuman: tmp
        };
}

function serialize$6(value) {
  var value$1 = value.dogOrHuman;
  return {
          dogOrHuman: value$1[0] >= 3406428 ? serialize$5(value$1[1]) : value$1[1]
        };
}

var Z__INTERNAL$6 = {
  graphql_module: 0
};

var Untitled1$prime$1 = {
  Raw: Raw$6,
  query: query$6,
  parse: parse$6,
  serialize: serialize$6,
  Z__INTERNAL: Z__INTERNAL$6
};

var Untitled1$1 = {
  Raw: Raw$6,
  query: query$6,
  parse: parse$6,
  serialize: serialize$6,
  Z__INTERNAL: Z__INTERNAL$6,
  self: Untitled1$prime$1
};

var UnionExternalFragmentQuery$prime = {
  DogFragment$prime: DogFragment$prime,
  DogFragment: DogFragment,
  Untitled1$prime: Untitled1$prime$1,
  Untitled1: Untitled1$1
};

var UnionExternalFragmentQuery = {
  DogFragment$prime: DogFragment$prime,
  DogFragment: DogFragment,
  Untitled1$prime: Untitled1$prime$1,
  Untitled1: Untitled1$1,
  self: UnionExternalFragmentQuery$prime
};

exports.MyQuery$prime = MyQuery$prime;
exports.MyQuery = MyQuery;
exports.OneFieldQuery$prime = OneFieldQuery$prime;
exports.OneFieldQuery = OneFieldQuery;
exports.ExternalFragmentQuery$prime = ExternalFragmentQuery$prime;
exports.ExternalFragmentQuery = ExternalFragmentQuery;
exports.InlineFragmentQuery$prime = InlineFragmentQuery$prime;
exports.InlineFragmentQuery = InlineFragmentQuery;
exports.UnionExternalFragmentQuery$prime = UnionExternalFragmentQuery$prime;
exports.UnionExternalFragmentQuery = UnionExternalFragmentQuery;
/* No side effect */
