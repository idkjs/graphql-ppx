// Generated by BUCKLESCRIPT, PLEASE EDIT WITH CARE
'use strict';

var Js_json = require("bs-platform/lib/js/js_json.js");
var Caml_option = require("bs-platform/lib/js/caml_option.js");

function parse(json) {
  var match = Js_json.decodeString(json);
  if (match !== undefined) {
    switch (match) {
      case "blue" :
          return /* Blue */2;
      case "green" :
          return /* Green */1;
      case "red" :
          return /* Red */0;
      default:
        return /* Red */0;
    }
  } else {
    return /* Red */0;
  }
}

function serialize(color) {
  switch (color) {
    case /* Red */0 :
        return "red";
    case /* Green */1 :
        return "green";
    case /* Blue */2 :
        return "blue";
    
  }
}

var Color = {
  parse: parse,
  serialize: serialize
};

function parse$1(json) {
  var match = Js_json.decodeString(json);
  if (match !== undefined) {
    return new Date(match);
  } else {
    return new Date();
  }
}

function serialize$1(date) {
  return date.toISOString();
}

var DateTime = {
  parse: parse$1,
  serialize: serialize$1
};

var Raw = { };

var query = "query   {\ncustomFields  {\ncurrentTime  \nfavoriteColor  \nfutureTime  \nnullableColor  \n}\n\n}\n";

function parse$2(value) {
  var value$1 = value.customFields;
  var value$2 = value$1.futureTime;
  var value$3 = value$1.nullableColor;
  return {
          customFields: {
            currentTime: parse$1(value$1.currentTime),
            favoriteColor: parse(value$1.favoriteColor),
            futureTime: (value$2 == null) ? undefined : Caml_option.some(parse$1(value$2)),
            nullableColor: (value$3 == null) ? undefined : parse(value$3)
          }
        };
}

function serialize$2(value) {
  var value$1 = value.customFields;
  var value$2 = value$1.nullableColor;
  var nullableColor = value$2 !== undefined ? serialize(value$2) : null;
  var value$3 = value$1.futureTime;
  var futureTime = value$3 !== undefined ? Caml_option.valFromOption(value$3).toISOString() : null;
  var value$4 = value$1.favoriteColor;
  var favoriteColor = serialize(value$4);
  var value$5 = value$1.currentTime;
  var currentTime = value$5.toISOString();
  var customFields = {
    currentTime: currentTime,
    favoriteColor: favoriteColor,
    futureTime: futureTime,
    nullableColor: nullableColor
  };
  return {
          customFields: customFields
        };
}

var Z__INTERNAL = {
  graphql_module: 0
};

var MyQuery$prime = {
  Raw: Raw,
  query: query,
  parse: parse$2,
  serialize: serialize$2,
  Z__INTERNAL: Z__INTERNAL
};

var MyQuery = {
  Raw: Raw,
  query: query,
  parse: parse$2,
  serialize: serialize$2,
  Z__INTERNAL: Z__INTERNAL,
  self: MyQuery$prime
};

exports.Color = Color;
exports.DateTime = DateTime;
exports.MyQuery$prime = MyQuery$prime;
exports.MyQuery = MyQuery;
/* No side effect */
