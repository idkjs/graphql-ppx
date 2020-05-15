// Generated by BUCKLESCRIPT, PLEASE EDIT WITH CARE
'use strict';


var Raw = { };

var query = "query   {\ndogOrHuman  {\n__typename\n...on Dog   {\nname  \nbarkVolume  \n}\n\n}\n\n}\n";

function parse(value) {
  var value$1 = value.dogOrHuman;
  var typename = value$1["__typename"];
  var tmp = typename === "Dog" ? /* `Dog */[
      3406428,
      {
        name: value$1.name,
        barkVolume: value$1.barkVolume
      }
    ] : /* `FutureAddedValue */[
      -31101740,
      value$1
    ];
  return {
          dogOrHuman: tmp
        };
}

function serialize(value) {
  var value$1 = value.dogOrHuman;
  var dogOrHuman;
  if (value$1[0] >= 3406428) {
    var value$2 = value$1[1];
    var value$3 = value$2.barkVolume;
    var value$4 = value$2.name;
    dogOrHuman = {
      __typename: "Dog",
      name: value$4,
      barkVolume: value$3
    };
  } else {
    dogOrHuman = value$1[1];
  }
  return {
          dogOrHuman: dogOrHuman
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

exports.MyQuery$prime = MyQuery$prime;
exports.MyQuery = MyQuery;
/* No side effect */
