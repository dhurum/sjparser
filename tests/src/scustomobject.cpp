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

TEST(SCustomObject, Empty) {
  std::string buf(R"({})");

  struct ObjectStruct {};

  using ObjectParser =
      SObject<ObjectStruct, Value<std::string>, Value<int64_t>>;

  auto objectCb = [&](ObjectParser &, ObjectStruct &) { return true; };

  Parser<ObjectParser> parser({{"string", "integer"}, objectCb});

  ASSERT_TRUE(parser.parse(buf));
  ASSERT_TRUE(parser.finish());

  ASSERT_TRUE(parser.parser().isSet());
}

TEST(SCustomObject, Null) {
  std::string buf(R"(null)");

  struct ObjectStruct {};

  using ObjectParser =
      SObject<ObjectStruct, Value<std::string>, Value<int64_t>>;

  auto objectCb = [&](ObjectParser &, ObjectStruct &) { return true; };

  Parser<ObjectParser> parser({{"string", "integer"}, objectCb});

  ASSERT_TRUE(parser.parse(buf));
  ASSERT_TRUE(parser.finish());

  ASSERT_FALSE(parser.parser().isSet());
}

TEST(SCustomObject, AllValuesFields) {
  std::string buf(
      R"({"bool": true, "integer": 10, "double": 11.5, "string": "value"})");

  struct ObjectStruct {
    bool bool_value;
    int64_t int_value;
    double double_value;
    std::string str_value;
  };

  // clang-format off
  using ObjectParser = SObject<
    ObjectStruct,
    Value<bool>,
    Value<int64_t>,
    Value<double>,
    Value<std::string>>;
  // clang-format on

  auto objectCb = [&](ObjectParser &parser, ObjectStruct &value) {
    value.bool_value = parser.get<0>();
    value.int_value = parser.get<1>();
    value.double_value = parser.get<2>();
    value.str_value = parser.get<3>();
    return true;
  };

  Parser<ObjectParser> parser(
      {{"bool", "integer", "double", "string"}, objectCb});

  ASSERT_TRUE(parser.parse(buf));
  ASSERT_TRUE(parser.finish());

  ASSERT_EQ(true, parser.parser().get().bool_value);
  ASSERT_EQ(10, parser.parser().get().int_value);
  ASSERT_EQ(11.5, parser.parser().get().double_value);
  ASSERT_EQ("value", parser.parser().get().str_value);
}

TEST(SCustomObject, FieldsWithCallbacks) {
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

  struct ObjectStruct {
    bool bool_value;
    std::string str_value;
  };

  // clang-format off
  using ObjectParser = SObject<
    ObjectStruct,
    Value<bool>,
    Value<std::string>>;
  // clang-format on

  auto objectCb = [&](ObjectParser &parser, ObjectStruct &value) {
    value.bool_value = parser.get<0>();
    value.str_value = parser.get<1>();
    return true;
  };

  Parser<ObjectParser> parser(
      {{{"bool", boolCb}, {"string", stringCb}}, objectCb});

  ASSERT_TRUE(parser.parse(buf));
  ASSERT_TRUE(parser.finish());

  ASSERT_EQ(true, parser.parser().get().bool_value);
  ASSERT_EQ("value", parser.parser().get().str_value);

  ASSERT_EQ(true, bool_value);
  ASSERT_EQ("value", str_value);
}

TEST(SCustomObject, FieldsWithCallbackError) {
  std::string buf(
      R"({"bool": true, "string": "value"})");

  auto boolCb = [&](const bool &) { return false; };

  auto stringCb = [&](const std::string &) { return true; };

  struct ObjectStruct {
    bool bool_value;
    std::string str_value;
  };

  // clang-format off
  using ObjectParser = SObject<
    ObjectStruct,
    Value<bool>,
    Value<std::string>>;
  // clang-format on

  auto objectCb = [&](ObjectParser &parser, ObjectStruct &value) {
    value.bool_value = parser.get<0>();
    value.str_value = parser.get<1>();
    return true;
  };

  Parser<ObjectParser> parser(
      {{{"bool", boolCb}, {"string", stringCb}}, objectCb});

  ASSERT_FALSE(parser.parse(buf));

  ASSERT_EQ("Callback returned false", parser.getError());
  ASSERT_EQ(
      R"(parse error: client cancelled parse via callback return value
                           {"bool": true, "string": "value"}
                     (right here) ------^
Callback returned false
)",
      parser.getError(true));
}

TEST(SCustomObject, SCustomObjectWithCallbackError) {
  std::string buf(
      R"({"bool": true, "string": "value"})");

  struct ObjectStruct {
    bool bool_value;
    std::string str_value;
  };

  // clang-format off
  using ObjectParser = SObject<
    ObjectStruct,
    Value<bool>,
    Value<std::string>>;
  // clang-format on

  auto objectCb = [&](ObjectParser &, ObjectStruct &) { return false; };

  Parser<ObjectParser> parser({{"bool", "string"}, objectCb});

  ASSERT_FALSE(parser.parse(buf));

  ASSERT_EQ("Callback returned false", parser.getError());
  ASSERT_EQ(
      R"(parse error: client cancelled parse via callback return value
          ool": true, "string": "value"}
                     (right here) ------^
Callback returned false
)",
      parser.getError(true));
}

TEST(SCustomObject, OneField) {
  std::string buf(R"({"string": "value"})");

  struct ObjectStruct {
    std::string str_value;
  };

  // clang-format off
  using ObjectParser = SObject<
    ObjectStruct,
    Value<std::string>>;
  // clang-format on

  auto objectCb = [&](ObjectParser &parser, ObjectStruct &value) {
    value.str_value = parser.get<0>();
    return true;
  };

  Parser<ObjectParser> parser({"string", objectCb});

  ASSERT_TRUE(parser.parse(buf));
  ASSERT_TRUE(parser.finish());

  ASSERT_EQ("value", parser.parser().get().str_value);
}

TEST(SCustomObject, OneFieldWithFieldCallback) {
  std::string buf(R"({"string": "value"})");
  std::string value;

  auto elementCb = [&](const std::string &str) {
    value = str;
    return true;
  };

  struct ObjectStruct {
    std::string str_value;
  };

  // clang-format off
  using ObjectParser = SObject<
    ObjectStruct,
    Value<std::string>>;
  // clang-format on

  auto objectCb = [&](ObjectParser &parser, ObjectStruct &value) {
    value.str_value = parser.get<0>();
    return true;
  };

  Parser<ObjectParser> parser({{{"string", elementCb}}, objectCb});

  ASSERT_TRUE(parser.parse(buf));
  ASSERT_TRUE(parser.finish());

  ASSERT_EQ("value", parser.parser().get().str_value);
  ASSERT_EQ("value", value);
}

TEST(SCustomObject, ObjectWithArgsStruct) {
  std::string buf(
      R"({"string": "value", "integer": 10})");

  struct ObjectStruct {
    std::string str_value;
    int64_t int_value;
  };

  // clang-format off
  using ObjectParser = SObject<
    ObjectStruct,
    Value<std::string>,
    Value<int64_t>>;
  // clang-format on

  auto objectCb = [&](ObjectParser &parser, ObjectStruct &value) {
    value.str_value = parser.get<0>();
    value.int_value = parser.get<1>();
    return true;
  };

  ObjectParser::Args object_args({{"string", "integer"}, objectCb});

  Parser<ObjectParser> parser(object_args);

  ASSERT_TRUE(parser.parse(buf));
  ASSERT_TRUE(parser.finish());

  ASSERT_EQ("value", parser.parser().get().str_value);
  ASSERT_EQ(10, parser.parser().get().int_value);
}

TEST(SCustomObject, StdStringiFieldNames) {
  std::string buf(
      R"({"string": "value", "integer": 10})");

  struct ObjectStruct {
    std::string str_value;
    int64_t int_value;
  };

  // clang-format off
  using ObjectParser = SObject<
    ObjectStruct,
    Value<std::string>,
    Value<int64_t>>;
  // clang-format on

  auto objectCb = [&](ObjectParser &parser, ObjectStruct &value) {
    value.str_value = parser.get<0>();
    value.int_value = parser.get<1>();
    return true;
  };

  std::string string_name = "string";
  std::string integer_name = "integer";

  Parser<ObjectParser> parser({{string_name, integer_name}, objectCb});

  ASSERT_TRUE(parser.parse(buf));
  ASSERT_TRUE(parser.finish());

  ASSERT_EQ("value", parser.parser().get().str_value);
  ASSERT_EQ(10, parser.parser().get().int_value);
}

TEST(SCustomObject, PopValue) {
  std::string buf(
      R"({"string": "value", "integer": 10})");

  struct ObjectStruct {
    std::string str_value;
    int64_t int_value;
  };

  // clang-format off
  using ObjectParser = SObject<
    ObjectStruct,
    Value<std::string>,
    Value<int64_t>>;
  // clang-format on

  auto objectCb = [&](ObjectParser &parser, ObjectStruct &value) {
    value.str_value = parser.get<0>();
    value.int_value = parser.get<1>();
    return true;
  };

  Parser<ObjectParser> parser({{"string", "integer"}, objectCb});

  ASSERT_TRUE(parser.parse(buf));
  ASSERT_TRUE(parser.finish());

  ASSERT_TRUE(parser.parser().isSet());
  auto value = parser.parser().pop();
  ASSERT_FALSE(parser.parser().isSet());

  ASSERT_EQ("value", value.str_value);
  ASSERT_EQ(10, value.int_value);
}

TEST(SCustomObject, SCustomObjectWithObject) {
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
    std::string str_value;
    int64_t int_value;
    int64_t inner_int_value;
    std::string inner_str_value;
    bool bool_value;
  };

  // clang-format off
  using ObjectParser = SObject<
    ObjectStruct,
    Value<std::string>,
    Value<int64_t>,
    Object<
      Value<int64_t>,
      Value<std::string>
    >,
    Value<bool>>;
  // clang-format on

  auto objectCb = [&](ObjectParser &parser, ObjectStruct &value) {
    value.str_value = parser.get<0>();
    value.int_value = parser.get<1>();
    value.inner_int_value = parser.get<2>().get<0>();
    value.inner_str_value = parser.get<2>().get<1>();
    value.bool_value = parser.get<3>();
    return true;
  };

  // clang-format off
  Parser<ObjectParser> parser({{
      "string",
      "integer",
      {
        "object", {
          "integer",
          "string"
        }
      },
      "boolean"}, objectCb});
  // clang-format on

  ASSERT_TRUE(parser.parse(buf));
  ASSERT_TRUE(parser.finish());

  ASSERT_EQ("value", parser.parser().get().str_value);
  ASSERT_EQ(10, parser.parser().get().int_value);
  ASSERT_EQ(1, parser.parser().get().inner_int_value);
  ASSERT_EQ("in_value", parser.parser().get().inner_str_value);
  ASSERT_EQ(true, parser.parser().get().bool_value);
}

TEST(SCustomObject, SCustomObjectWithUnexpectedObject) {
  std::string buf(
      R"(
{
  "string": "value",
  "object": {
    "error": 1
  }
})");

  struct ObjectStruct {};

  // clang-format off
  using ObjectParser = SObject<
    ObjectStruct,
    Value<std::string>,
    Object<
      Value<int64_t>
    >>;
  // clang-format on

  auto objectCb = [&](ObjectParser &, ObjectStruct &) { return true; };

  // clang-format off
  Parser<ObjectParser> parser({{
      "string",
      {
        "object", {
          "integer"
        }
      }}, objectCb});
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

TEST(SCustomObject, SCustomObjectWithObjectWithCallback) {
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

  auto innerCb = [&](InnerObjectParser &parser) {
    int_value = parser.get<0>();
    str_value = parser.get<1>();
    return true;
  };

  struct ObjectStruct {
    std::string str_value;
    int64_t int_value;
    bool bool_value;
  };

  // clang-format off
  using ObjectParser = SObject<
    ObjectStruct,
    Value<std::string>,
    Value<int64_t>,
    InnerObjectParser,
    Value<bool>>;
  // clang-format on

  auto objectCb = [&](ObjectParser &parser, ObjectStruct &value) {
    value.str_value = parser.get<0>();
    value.int_value = parser.get<1>();
    value.bool_value = parser.get<3>();
    return true;
  };

  // clang-format off
  Parser<ObjectParser> parser({{
      "string",
      "integer",
      {
        "object", {
          {
            "integer",
            "string"
          }, innerCb
        }
      },
      "boolean"}, objectCb});
  // clang-format on

  ASSERT_TRUE(parser.parse(buf));
  ASSERT_TRUE(parser.finish());

  ASSERT_EQ("value", parser.parser().get().str_value);
  ASSERT_EQ(10, parser.parser().get().int_value);
  ASSERT_EQ(true, parser.parser().get().bool_value);

  ASSERT_EQ(1, int_value);
  ASSERT_EQ("in_value", str_value);
}

TEST(SCustomObject, SCustomObjectOfObjects) {
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

  struct ObjectStruct {
    std::string first_str_value;
    int64_t first_int_value;
    int64_t second_int_value;
    std::string second_str_value;
    bool bool_value;
  };

  // clang-format off
  using ObjectParser = SObject<
    ObjectStruct,
    Object<
      Value<std::string>,
      Value<int64_t>>,
    Object<
      Value<int64_t>,
      Value<std::string>>,
    Object<Value<bool>>>;
  // clang-format on

  auto objectCb = [&](ObjectParser &parser, ObjectStruct &value) {
    value.first_str_value = parser.get<0>().get<0>();
    value.first_int_value = parser.get<0>().get<1>();
    value.second_int_value = parser.get<1>().get<0>();
    value.second_str_value = parser.get<1>().get<1>();
    value.bool_value = parser.get<2>().get<0>();
    return true;
  };

  // clang-format off
  Parser<ObjectParser> parser({{
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
    }, objectCb});
  // clang-format on

  ASSERT_TRUE(parser.parse(buf));
  ASSERT_TRUE(parser.finish());

  ASSERT_EQ("value", parser.parser().get().first_str_value);
  ASSERT_EQ(10, parser.parser().get().first_int_value);
  ASSERT_EQ(1, parser.parser().get().second_int_value);
  ASSERT_EQ("value2", parser.parser().get().second_str_value);
  ASSERT_EQ(true, parser.parser().get().bool_value);
}

TEST(SCustomObject, SCustomObjectWithSCustomObject) {
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

  struct InnerObjectStruct {
    int64_t int_value;
    std::string str_value;
  };

  using InnerObjectParser =
      SObject<InnerObjectStruct, Value<int64_t>, Value<std::string>>;

  auto innerCb = [&](InnerObjectParser &parser, InnerObjectStruct &value) {
    value.int_value = parser.get<0>();
    value.str_value = parser.get<1>();
    return true;
  };

  struct ObjectStruct {
    std::string str_value;
    int64_t int_value;
    InnerObjectStruct inner_value;
    bool bool_value;
  };

  // clang-format off
  using ObjectParser = SObject<
    ObjectStruct,
    Value<std::string>,
    Value<int64_t>,
    InnerObjectParser,
    Value<bool>>;
  // clang-format on

  auto objectCb = [&](ObjectParser &parser, ObjectStruct &value) {
    value.str_value = parser.get<0>();
    value.int_value = parser.get<1>();
    value.inner_value = parser.get<2>();
    value.bool_value = parser.get<3>();
    return true;
  };

  // clang-format off
  Parser<ObjectParser> parser({{
      "string",
      "integer",
      {
        "object", {
          {
            "integer",
            "string"
          }, innerCb
        }
      },
      "boolean"}, objectCb});
  // clang-format on

  ASSERT_TRUE(parser.parse(buf));
  ASSERT_TRUE(parser.finish());

  ASSERT_EQ("value", parser.parser().get().str_value);
  ASSERT_EQ(10, parser.parser().get().int_value);
  ASSERT_EQ(1, parser.parser().get().inner_value.int_value);
  ASSERT_EQ("in_value", parser.parser().get().inner_value.str_value);
  ASSERT_EQ(true, parser.parser().get().bool_value);
}

TEST(SCustomObject, SCustomObjectWithSAutoObject) {
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
    std::string str_value;
    int64_t int_value;
    std::tuple<int64_t, std::string> inner_value;
    bool bool_value;
  };

  // clang-format off
  using ObjectParser = SObject<
    ObjectStruct,
    Value<std::string>,
    Value<int64_t>,
    SObject<
      Value<int64_t>,
      Value<std::string>
    >,
    Value<bool>>;
  // clang-format on

  auto objectCb = [&](ObjectParser &parser, ObjectStruct &value) {
    value.str_value = parser.get<0>();
    value.int_value = parser.get<1>();
    value.inner_value = parser.get<2>();
    value.bool_value = parser.get<3>();
    return true;
  };

  // clang-format off
  Parser<ObjectParser> parser({{
      "string",
      "integer",
      {
        "object", {
          "integer",
          "string"
        }
      },
      "boolean"}, objectCb});
  // clang-format on

  ASSERT_TRUE(parser.parse(buf));
  ASSERT_TRUE(parser.finish());

  ASSERT_EQ("value", parser.parser().get().str_value);
  ASSERT_EQ(10, parser.parser().get().int_value);
  ASSERT_EQ(1, std::get<0>(parser.parser().get().inner_value));
  ASSERT_EQ("in_value", std::get<1>(parser.parser().get().inner_value));
  ASSERT_EQ(true, parser.parser().get().bool_value);
}

TEST(SCustomObject, SCustomObjectWithStandaloneUnion) {
  std::string buf(
      R"(
{
  "id": 10,
  "data": {
    "type": "1",
    "bool": true
  }
})");

  struct ObjectStruct {
    int64_t int_value;
    bool inner_bool;
    union {
      bool bool_value;
      int64_t int_value;
    } inner_value;
  };

  // clang-format off
  using ObjectParser = SObject<
    ObjectStruct,
    Value<int64_t>,
    Union<
      std::string,
      Object<Value<bool>>,
      Object<Value<int64_t>>
    >>;
  // clang-format on

  auto objectCb = [&](ObjectParser &parser, ObjectStruct &value) {
    value.int_value = parser.get<0>();
    value.inner_bool = (parser.get<1>().currentMemberId() == 0) ? true : false;
    if (value.inner_bool) {
      value.inner_value.bool_value = parser.get<1>().get<0>().get<0>();
    } else {
      value.inner_value.int_value = parser.get<1>().get<1>().get<0>();
    }
    return true;
  };

  Parser<ObjectParser> parser(
      {{"id", {"data", {"type", {{"1", "bool"}, {"2", "int"}}}}}, objectCb});

  ASSERT_TRUE(parser.parse(buf));
  ASSERT_TRUE(parser.finish());

  ASSERT_TRUE(parser.parser().get().inner_bool);

  ASSERT_EQ(10, parser.parser().get().int_value);
  ASSERT_EQ(true, parser.parser().get().inner_value.bool_value);

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

  ASSERT_FALSE(parser.parser().get().inner_bool);

  ASSERT_EQ(10, parser.parser().get().int_value);
  ASSERT_EQ(100, parser.parser().get().inner_value.int_value);
}

TEST(SCustomObject, SCustomObjectWithObjectUnion) {
  std::string buf(
      R"(
{
  "id": 10,
  "type": "1",
  "bool": true
})");

  struct ObjectStruct {
    int64_t int_value;
    bool inner_bool;
    union {
      bool bool_value;
      int64_t int_value;
    } inner_value;
  };

  // clang-format off
  using ObjectParser = SObject<
    ObjectStruct,
    Value<int64_t>,
    Union<
      std::string,
      Object<Value<bool>>,
      Object<Value<int64_t>>
    >>;
  // clang-format on

  auto objectCb = [&](ObjectParser &parser, ObjectStruct &value) {
    value.int_value = parser.get<0>();
    value.inner_bool = (parser.get<1>().currentMemberId() == 0) ? true : false;
    if (value.inner_bool) {
      value.inner_value.bool_value = parser.get<1>().get<0>().get<0>();
    } else {
      value.inner_value.int_value = parser.get<1>().get<1>().get<0>();
    }
    return true;
  };

  Parser<ObjectParser> parser(
      {{"id", {"type", {{"1", "bool"}, {"2", "int"}}}}, objectCb});

  ASSERT_TRUE(parser.parse(buf));
  ASSERT_TRUE(parser.finish());

  ASSERT_TRUE(parser.parser().get().inner_bool);

  ASSERT_EQ(10, parser.parser().get().int_value);
  ASSERT_EQ(true, parser.parser().get().inner_value.bool_value);

  std::string buf2(
      R"(
{
  "id": 10,
  "type": "2",
  "int": 100
})");

  ASSERT_TRUE(parser.parse(buf2));
  ASSERT_TRUE(parser.finish());

  ASSERT_FALSE(parser.parser().get().inner_bool);

  ASSERT_EQ(10, parser.parser().get().int_value);
  ASSERT_EQ(100, parser.parser().get().inner_value.int_value);
}

TEST(SCustomObject, SCustomObjectWithArray) {
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

  std::vector<std::string> tmp_array;

  auto arrayCb = [&](const std::string &value) {
    tmp_array.push_back(value);
    return true;
  };

  struct ObjectStruct {
    std::string str_value;
    int64_t int_value;
    std::vector<std::string> array_value;
  };

  // clang-format off
  using ObjectParser = SObject<
    ObjectStruct,
    Value<std::string>,
    Value<int64_t>,
    Array<Value<std::string>>>;
  // clang-format on

  auto objectCb = [&](ObjectParser &parser, ObjectStruct &value) {
    value.str_value = parser.get<0>();
    value.int_value = parser.get<1>();
    value.array_value = tmp_array;
    return true;
  };

  Parser<ObjectParser> parser(
      {{"string", "integer", {"array", arrayCb}}, objectCb});

  ASSERT_TRUE(parser.parse(buf));
  ASSERT_TRUE(parser.finish());

  ASSERT_EQ("value", parser.parser().get().str_value);
  ASSERT_EQ(10, parser.parser().get().int_value);
  ASSERT_EQ(3, parser.parser().get().array_value.size());
  ASSERT_EQ("elt1", parser.parser().get().array_value[0]);
  ASSERT_EQ("elt2", parser.parser().get().array_value[1]);
  ASSERT_EQ("elt3", parser.parser().get().array_value[2]);
}

TEST(SCustomObject, SCustomObjectWithSArray) {
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

  struct ObjectStruct {
    std::string str_value;
    int64_t int_value;
    std::vector<std::string> array_value;
  };

  // clang-format off
  using ObjectParser = SObject<
    ObjectStruct,
    Value<std::string>,
    Value<int64_t>,
    SArray<Value<std::string>>>;
  // clang-format on

  auto objectCb = [&](ObjectParser &parser, ObjectStruct &value) {
    value.str_value = parser.get<0>();
    value.int_value = parser.get<1>();
    value.array_value = parser.get<2>();
    return true;
  };

  Parser<ObjectParser> parser({{"string", "integer", "array"}, objectCb});

  ASSERT_TRUE(parser.parse(buf));
  ASSERT_TRUE(parser.finish());

  ASSERT_EQ("value", parser.parser().get().str_value);
  ASSERT_EQ(10, parser.parser().get().int_value);
  ASSERT_EQ(3, parser.parser().get().array_value.size());
  ASSERT_EQ("elt1", parser.parser().get().array_value[0]);
  ASSERT_EQ("elt2", parser.parser().get().array_value[1]);
  ASSERT_EQ("elt3", parser.parser().get().array_value[2]);
}
