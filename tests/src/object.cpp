/*******************************************************************************

Copyright (c) 2016-2017 Denis Tikhomirov <dvtikhomirov@gmail.com>

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal in
the Software without restriction, including without limitation the rights to
use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of
the Software, and to permit persons to whom the Software is furnished to do so,
subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS
FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR
COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER
IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

*******************************************************************************/

#include <gtest/gtest.h>
#include "sjparser.h"

using namespace SJParser;

TEST(Object, Empty) {
  std::string buf(R"({})");

  Parser<Object<Value<bool>, Value<std::string>>> parser({"bool", "string"});

  ASSERT_TRUE(parser.parse(buf));
  ASSERT_TRUE(parser.finish());

  ASSERT_TRUE(parser.parser().isSet());
  ASSERT_FALSE(parser.parser().get<0>().isSet());
  ASSERT_FALSE(parser.parser().get<1>().isSet());
}

TEST(Object, EmptyWithCallback) {
  std::string buf(R"({})");
  bool callback_called = false;

  using ObjectParser = Object<Value<bool>, Value<std::string>>;

  auto objectCb = [&](ObjectParser &) {
    callback_called = true;
    return true;
  };

  Parser<ObjectParser> parser({{"bool", "string"}, objectCb});

  ASSERT_TRUE(parser.parse(buf));
  ASSERT_TRUE(parser.finish());

  ASSERT_TRUE(parser.parser().isSet());
  ASSERT_FALSE(parser.parser().get<0>().isSet());
  ASSERT_FALSE(parser.parser().get<1>().isSet());

  ASSERT_TRUE(callback_called);
}

TEST(Object, Null) {
  std::string buf(R"(null)");

  Parser<Object<Value<bool>, Value<std::string>>> parser({"bool", "string"});

  ASSERT_TRUE(parser.parse(buf));
  ASSERT_TRUE(parser.finish());

  ASSERT_FALSE(parser.parser().isSet());
}

TEST(Object, AllValuesFields) {
  std::string buf(
      R"({"bool": true, "integer": 10, "double": 11.5, "string": "value"})");

  // clang-format off
  Parser<Object<
    Value<bool>,
    Value<int64_t>,
    Value<double>,
    Value<std::string>
  >> parser({"bool", "integer", "double", "string"});
  // clang-format on

  ASSERT_TRUE(parser.parse(buf));
  ASSERT_TRUE(parser.finish());

  ASSERT_EQ(true, parser.parser().get<0>().get());
  ASSERT_EQ(10, parser.parser().get<1>().get());
  ASSERT_EQ(11.5, parser.parser().get<2>().get());
  ASSERT_EQ("value", parser.parser().get<3>().get());
}

TEST(Object, FieldsWithCallbacks) {
  std::string buf(
      R"({"bool": true, "string": "value"})");
  bool bool_value = false;
  std::string str_value;

  auto boolCb = [&](const bool &value) {
    bool_value = value;
    return true;
  };

  auto stringCb = [&](const std::string &value) {
    str_value = value;
    return true;
  };

  Parser<Object<Value<bool>, Value<std::string>>> parser(
      {{"bool", boolCb}, {"string", stringCb}});

  ASSERT_TRUE(parser.parse(buf));
  ASSERT_TRUE(parser.finish());

  ASSERT_EQ(true, parser.parser().get<0>().get());
  ASSERT_EQ("value", parser.parser().get<1>().get());

  ASSERT_EQ(true, bool_value);
  ASSERT_EQ("value", str_value);
}

TEST(Object, FieldsWithCallbackError) {
  std::string buf(
      R"({"bool": true, "string": "value"})");

  auto boolCb = [&](const bool &) { return false; };

  auto stringCb = [&](const std::string &) { return true; };

  Parser<Object<Value<bool>, Value<std::string>>> parser(
      {{"bool", boolCb}, {"string", stringCb}});

  ASSERT_FALSE(parser.parse(buf));
  ASSERT_FALSE(parser.parser().isSet());

  ASSERT_EQ("Callback returned false", parser.getError());
  ASSERT_EQ(
      R"(parse error: client cancelled parse via callback return value
                           {"bool": true, "string": "value"}
                     (right here) ------^
Callback returned false
)",
      parser.getError(true));
}

TEST(Object, ObjectWithCallback) {
  std::string buf(
      R"({"bool": true, "string": "value"})");
  bool bool_value = false;
  std::string str_value;

  using ObjectParser = Object<Value<bool>, Value<std::string>>;

  auto objectCb = [&](ObjectParser &parser) {
    bool_value = parser.get<0>().get();
    str_value = parser.get<1>().get();
    return true;
  };

  Parser<ObjectParser> parser({{"bool", "string"}, objectCb});

  ASSERT_TRUE(parser.parse(buf));
  ASSERT_TRUE(parser.finish());

  ASSERT_EQ(true, parser.parser().get<0>().get());
  ASSERT_EQ("value", parser.parser().get<1>().get());
  ASSERT_EQ(true, bool_value);
  ASSERT_EQ("value", str_value);
}

TEST(Object, ObjectWithCallbackError) {
  std::string buf(
      R"({"bool": true, "string": "value"})");

  using ObjectParser = Object<Value<bool>, Value<std::string>>;

  auto objectCb = [&](ObjectParser &) { return false; };

  Parser<ObjectParser> parser({{"bool", "string"}, objectCb});

  ASSERT_FALSE(parser.parse(buf));
  ASSERT_TRUE(parser.parser().isSet());

  ASSERT_EQ("Callback returned false", parser.getError());
  ASSERT_EQ(
      R"(parse error: client cancelled parse via callback return value
          ool": true, "string": "value"}
                     (right here) ------^
Callback returned false
)",
      parser.getError(true));
}

TEST(Object, OneField) {
  std::string buf(R"({"string": "value"})");

  // Notice - no {} around the argument!
  Parser<Object<Value<std::string>>> parser("string");

  ASSERT_TRUE(parser.parse(buf));
  ASSERT_TRUE(parser.finish());

  ASSERT_EQ("value", parser.parser().get<0>().get());
}

TEST(Object, OneFieldWithFieldCallback) {
  std::string buf(R"({"string": "value"})");
  std::string value;

  auto elementCb = [&](const std::string &str) {
    value = str;
    return true;
  };

  Parser<Object<Value<std::string>>> parser({{"string", elementCb}});

  ASSERT_TRUE(parser.parse(buf));
  ASSERT_TRUE(parser.finish());

  ASSERT_EQ("value", parser.parser().get<0>().get());
  ASSERT_EQ("value", value);
}

TEST(Object, OneFieldWithObjectCallback) {
  std::string buf(R"({"string": "value"})");
  std::string value;

  using ObjectParser = Object<Value<std::string>>;

  auto objectCb = [&](ObjectParser &parser) {
    value = parser.get<0>().get();
    return true;
  };

  // {} around "string" are optional, but they make it a bit more clear
  Parser<ObjectParser> parser({{"string"}, objectCb});

  ASSERT_TRUE(parser.parse(buf));
  ASSERT_TRUE(parser.finish());

  ASSERT_EQ("value", parser.parser().get<0>().get());
  ASSERT_EQ("value", value);
}

TEST(Object, OneFieldWithElementAndObjectCallbacks) {
  std::string buf(R"({"string": "value"})");
  std::string value;
  std::string object_value;

  auto elementCb = [&](const std::string &str) {
    value = str;
    return true;
  };

  using ObjectParser = Object<Value<std::string>>;

  auto objectCb = [&](ObjectParser &parser) {
    object_value = parser.get<0>().get();
    return true;
  };

  Parser<ObjectParser> parser({{{"string", elementCb}}, objectCb});

  ASSERT_TRUE(parser.parse(buf));
  ASSERT_TRUE(parser.finish());

  ASSERT_EQ("value", parser.parser().get<0>().get());
  ASSERT_EQ("value", value);
  ASSERT_EQ("value", object_value);
}

TEST(Object, ObjectWithArgsStruct) {
  std::string buf(
      R"({"string": "value", "integer": 10})");

  using ObjectParser = Object<Value<std::string>, Value<int64_t>>;

  ObjectParser::Args object_args({"string", "integer"});

  Parser<ObjectParser> parser(object_args);

  ASSERT_TRUE(parser.parse(buf));
  ASSERT_TRUE(parser.finish());

  ASSERT_EQ("value", parser.parser().get<0>().get());
  ASSERT_EQ(10, parser.parser().get<1>().get());
}

TEST(Object, StdStringiFieldNames) {
  std::string buf(R"({"string": "value", "integer": 10})");

  std::string string_name = "string";
  std::string integer_name = "integer";
  Parser<Object<Value<std::string>, Value<int64_t>>> parser(
      {string_name, integer_name});

  ASSERT_TRUE(parser.parse(buf));
  ASSERT_TRUE(parser.finish());

  ASSERT_EQ("value", parser.parser().get<0>().get());
  ASSERT_EQ(10, parser.parser().get<1>().get());
}

TEST(Object, ObjectWithObject) {
  std::string buf(
      R"(
{
  "string": "value",
  "integer": 10,
  "object": {
    "integer": 1,
    "string": "in_value"
  },
  "boolean": true
})");

  // clang-format off
  Parser<Object<
    Value<std::string>,
    Value<int64_t>,
    Object<
      Value<int64_t>,
      Value<std::string>
    >,
    Value<bool>
  >> parser({
      "string",
      "integer",
      {
        "object", {
          "integer",
          "string"
        }
      },
      "boolean"});
  // clang-format on

  ASSERT_TRUE(parser.parse(buf));
  ASSERT_TRUE(parser.finish());

  ASSERT_EQ("value", parser.parser().get<0>().get());
  ASSERT_EQ(10, parser.parser().get<1>().get());
  ASSERT_EQ(1, parser.parser().get<2>().get<0>().get());
  ASSERT_EQ("in_value", parser.parser().get<2>().get<1>().get());
  ASSERT_EQ(true, parser.parser().get<3>().get());
}

TEST(Object, ObjectWithUnexpectedObject) {
  std::string buf(
      R"(
{
  "string": "value",
  "object": {
    "error": 1
  }
})");

  // clang-format off
  Parser<Object<
    Value<std::string>,
    Object<
      Value<int64_t>
    >
  >> parser({
      "string",
      {
        "object", {
          "integer"
        }
      }});
  // clang-format on

  ASSERT_FALSE(parser.parse(buf));

  ASSERT_FALSE(parser.parser().isSet());
  ASSERT_EQ("Unexpected field error", parser.getError());

  ASSERT_EQ(
      R"(parse error: client cancelled parse via callback return value
          ue",   "object": {     "error": 1   } }
                     (right here) ------^
Unexpected field error
)",
      parser.getError(true));
}

TEST(Object, ObjectWithObjectWithCallback) {
  std::string buf(
      R"(
{
  "string": "value",
  "integer": 10,
  "object": {
    "integer": 1,
    "string": "in_value"
  },
  "boolean": true
})");

  int64_t int_value;
  std::string str_value;

  using InnerObjectParser = Object<Value<int64_t>, Value<std::string>>;

  auto callback = [&](InnerObjectParser &parser) {
    int_value = parser.get<0>().get();
    str_value = parser.get<1>().get();
    return true;
  };

  // clang-format off
  Parser<Object<
    Value<std::string>,
    Value<int64_t>,
    InnerObjectParser,
    Value<bool>
  >> parser({
      "string",
      "integer",
      {
        "object", {
          {
            "integer",
            "string"
          }, callback
        }
      },
      "boolean"});
  // clang-format on

  ASSERT_TRUE(parser.parse(buf));
  ASSERT_TRUE(parser.finish());

  ASSERT_EQ("value", parser.parser().get<0>().get());
  ASSERT_EQ(10, parser.parser().get<1>().get());
  ASSERT_EQ(1, parser.parser().get<2>().get<0>().get());
  ASSERT_EQ("in_value", parser.parser().get<2>().get<1>().get());
  ASSERT_EQ(true, parser.parser().get<3>().get());

  ASSERT_EQ(1, int_value);
  ASSERT_EQ("in_value", str_value);
}

TEST(Object, ObjectOfObjects) {
  std::string buf(
      R"(
{
  "object1": {
    "string": "value",
    "integer": 10
  },
  "object2": {
    "integer": 1,
    "string": "value2"
  },
  "object3": {
    "boolean": true
  }
})");

  // clang-format off
  Parser<Object<
    Object<
      Value<std::string>,
      Value<int64_t>>,
    Object<
      Value<int64_t>,
      Value<std::string>>,
    Object<Value<bool>>
  >> parser({
      {
        "object1", {
          "string",
          "integer"
        }
      }, {
        "object2", {
          "integer",
          "string"
        }
      }, {
        "object3", {
          "boolean"
        }
      }
    });
  // clang-format on

  ASSERT_TRUE(parser.parse(buf));
  ASSERT_TRUE(parser.finish());

  ASSERT_EQ("value", parser.parser().get<0>().get<0>().get());
  ASSERT_EQ(10, parser.parser().get<0>().get<1>().get());
  ASSERT_EQ(1, parser.parser().get<1>().get<0>().get());
  ASSERT_EQ("value2", parser.parser().get<1>().get<1>().get());
  ASSERT_EQ(true, parser.parser().get<2>().get<0>().get());
}

TEST(Object, ObjectWithSCustomObject) {
  std::string buf(
      R"(
{
  "string": "value",
  "integer": 10,
  "object": {
    "integer": 1,
    "string": "in_value"
  },
  "boolean": true
})");

  struct ObjectStruct {
    int64_t int_field;
    std::string str_field;
  };

  using InnerObjectParser =
      SObject<ObjectStruct, Value<int64_t>, Value<std::string>>;

  auto innerObjectCb = [&](InnerObjectParser &parser, ObjectStruct &value) {
    value = {parser.get<0>().pop(), parser.get<1>().pop()};
    return true;
  };

  // clang-format off
  Parser<Object<
    Value<std::string>,
    Value<int64_t>,
    InnerObjectParser,
    Value<bool>
  >> parser({
      "string",
      "integer",
      {
        "object", {
          {
            "integer",
            "string"
          }, innerObjectCb
        }
      },
      "boolean"});
  // clang-format on

  ASSERT_TRUE(parser.parse(buf));
  ASSERT_TRUE(parser.finish());

  ASSERT_EQ("value", parser.parser().get<0>().get());
  ASSERT_EQ(10, parser.parser().get<1>().get());
  ASSERT_EQ(1, parser.parser().get<2>().get().int_field);
  ASSERT_EQ("in_value", parser.parser().get<2>().get().str_field);
  ASSERT_EQ(true, parser.parser().get<3>().get());
}

TEST(Object, ObjectWithSAutoObject) {
  std::string buf(
      R"(
{
  "string": "value",
  "integer": 10,
  "object": {
    "integer": 1,
    "string": "in_value"
  },
  "boolean": true
})");

  // clang-format off
  Parser<Object<
    Value<std::string>,
    Value<int64_t>,
    SObject<
      Value<int64_t>,
      Value<std::string>
    >,
    Value<bool>
  >> parser({
      "string",
      "integer",
      {
        "object", {
          "integer",
          "string"
        }
      },
      "boolean"});
  // clang-format on

  ASSERT_TRUE(parser.parse(buf));
  ASSERT_TRUE(parser.finish());

  ASSERT_EQ("value", parser.parser().get<0>().get());
  ASSERT_EQ(10, parser.parser().get<1>().get());

  auto inner_value = parser.parser().get<2>().get();
  ASSERT_EQ(1, std::get<0>(inner_value));
  ASSERT_EQ("in_value", std::get<1>(inner_value));

  ASSERT_EQ(true, parser.parser().get<3>().get());
}

TEST(Object, ObjectWithStandaloneUnion) {
  std::string buf(
      R"(
{
  "id": 10,
  "data": {
    "type": "1",
    "bool": true
  }
})");

  // clang-format off
  Parser<Object<
    Value<int64_t>,
    Union<
      std::string,
      Object<Value<bool>>,
      Object<Value<int64_t>>
  >>> parser({"id", {"data", {"type", {{"1", "bool"}, {"2", "int"}}}}});
  // clang-format on

  ASSERT_TRUE(parser.parse(buf));
  ASSERT_TRUE(parser.finish());

  ASSERT_TRUE(parser.parser().get<1>().get<0>().isSet());
  ASSERT_FALSE(parser.parser().get<1>().get<1>().isSet());
  ASSERT_EQ(0, parser.parser().get<1>().currentMemberId());

  ASSERT_EQ(10, parser.parser().get<0>().get());
  ASSERT_EQ(true, parser.parser().get<1>().get<0>().get<0>().get());

  std::string buf2(
      R"(
{
  "id": 10,
  "data": {
    "type": "2",
    "int": 100
  }
})");

  ASSERT_TRUE(parser.parse(buf2));
  ASSERT_TRUE(parser.finish());

  ASSERT_FALSE(parser.parser().get<1>().get<0>().isSet());
  ASSERT_TRUE(parser.parser().get<1>().get<1>().isSet());
  ASSERT_EQ(1, parser.parser().get<1>().currentMemberId());

  ASSERT_EQ(10, parser.parser().get<0>().get());
  ASSERT_EQ(100, parser.parser().get<1>().get<1>().get<0>().get());
}

TEST(Object, ObjectWithObjectUnion) {
  std::string buf(
      R"(
{
  "id": 10,
  "type": "1",
  "bool": true
})");

  // clang-format off
  Parser<Object<
    Value<int64_t>,
    Union<
      std::string,
      Object<Value<bool>>,
      Object<Value<int64_t>>
  >>> parser({"id", {"type", {{"1", "bool"}, {"2", "int"}}}});
  // clang-format on

  ASSERT_TRUE(parser.parse(buf));
  ASSERT_TRUE(parser.finish());

  ASSERT_TRUE(parser.parser().get<1>().get<0>().isSet());
  ASSERT_FALSE(parser.parser().get<1>().get<1>().isSet());
  ASSERT_EQ(0, parser.parser().get<1>().currentMemberId());

  ASSERT_EQ(10, parser.parser().get<0>().get());
  ASSERT_EQ(true, parser.parser().get<1>().get<0>().get<0>().get());

  std::string buf2(
      R"(
{
  "id": 10,
  "type": "2",
  "int": 100
})");

  ASSERT_TRUE(parser.parse(buf2));
  ASSERT_TRUE(parser.finish());

  ASSERT_FALSE(parser.parser().get<1>().get<0>().isSet());
  ASSERT_TRUE(parser.parser().get<1>().get<1>().isSet());
  ASSERT_EQ(1, parser.parser().get<1>().currentMemberId());

  ASSERT_EQ(10, parser.parser().get<0>().get());
  ASSERT_EQ(100, parser.parser().get<1>().get<1>().get<0>().get());
}

TEST(Object, ObjectWithArray) {
  std::string buf(
      R"(
{
  "string": "value",
  "integer": 10,
  "array": [
    "elt1",
    "elt2",
    "elt3"
  ]
})");

  std::vector<std::string> array_value;

  using ParserType =
      Object<Value<std::string>, Value<int64_t>, Array<Value<std::string>>>;

  auto arrayCb = [&](const std::string &value) {
    array_value.push_back(value);
    return true;
  };

  Parser<ParserType> parser({"string", "integer", {"array", arrayCb}});

  ASSERT_TRUE(parser.parse(buf));
  ASSERT_TRUE(parser.finish());

  ASSERT_EQ("value", parser.parser().get<0>().get());
  ASSERT_EQ(10, parser.parser().get<1>().get());
  ASSERT_EQ(3, array_value.size());
  ASSERT_EQ("elt1", array_value[0]);
  ASSERT_EQ("elt2", array_value[1]);
  ASSERT_EQ("elt3", array_value[2]);
}

TEST(Object, ObjectWithSArray) {
  std::string buf(
      R"(
{
  "string": "value",
  "integer": 10,
  "array": [
    "elt1",
    "elt2",
    "elt3"
  ]
})");

  using ParserType =
      Object<Value<std::string>, Value<int64_t>, SArray<Value<std::string>>>;

  Parser<ParserType> parser({"string", "integer", "array"});

  ASSERT_TRUE(parser.parse(buf));
  ASSERT_TRUE(parser.finish());

  ASSERT_EQ("value", parser.parser().get<0>().get());
  ASSERT_EQ(10, parser.parser().get<1>().get());
  ASSERT_EQ(3, parser.parser().get<2>().get().size());
  ASSERT_EQ("elt1", parser.parser().get<2>().get()[0]);
  ASSERT_EQ("elt2", parser.parser().get<2>().get()[1]);
  ASSERT_EQ("elt3", parser.parser().get<2>().get()[2]);
}
