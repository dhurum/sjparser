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

TEST(ObjectUnion, Empty) {
  std::string buf(R"({})");

  // clang-format off
  Parser<Object<
    Union<
      int64_t,
      Object<Value<bool>>,
      Object<Value<int64_t>>
  >>> parser({{"type", {{1, "bool"}, {2, "int"}}}});
  // clang-format on

  ASSERT_TRUE(parser.parse(buf));
  ASSERT_TRUE(parser.finish());

  ASSERT_TRUE(parser.parser().isSet());
}

TEST(ObjectUnion, Null) {
  std::string buf(R"(null)");

  // clang-format off
  Parser<Object<
    Union<
      int64_t,
      Object<Value<bool>>,
      Object<Value<int64_t>>
  >>> parser({{"type", {{1, "bool"}, {2, "int"}}}});
  // clang-format on

  ASSERT_TRUE(parser.parse(buf));
  ASSERT_TRUE(parser.finish());

  ASSERT_FALSE(parser.parser().isSet());
}

TEST(ObjectUnion, AllValuesFields) {
  std::string buf(
      R"({"type": 1, "bool": true, "integer": 10})");

  // clang-format off
  Parser<Object<
    Union<
      int64_t,
      Object<
        Value<bool>,
        Value<int64_t>>,
      Object<
        Value<double>,
        Value<std::string>>
  >>> parser({{"type", {{1, {"bool", "integer"}}, {2, {"double", "string"}}}}});
  // clang-format on

  ASSERT_TRUE(parser.parse(buf));
  ASSERT_TRUE(parser.finish());

  ASSERT_TRUE(parser.parser().get<0>().parser<0>().isSet());
  ASSERT_FALSE(parser.parser().get<0>().parser<1>().isSet());
  ASSERT_EQ(0, parser.parser().get<0>().currentMemberId());

  ASSERT_EQ(true, parser.parser().get<0>().get<0>().get<0>());
  ASSERT_EQ(10, parser.parser().get<0>().get<0>().get<1>());

  buf = R"({"type": 2, "double": 11.5, "string": "value"})";

  ASSERT_TRUE(parser.parse(buf));
  ASSERT_TRUE(parser.finish());

  ASSERT_FALSE(parser.parser().get<0>().parser<0>().isSet());
  ASSERT_TRUE(parser.parser().get<0>().parser<1>().isSet());
  ASSERT_EQ(1, parser.parser().get<0>().currentMemberId());

  ASSERT_EQ(11.5, parser.parser().get<0>().get<1>().get<0>());
  ASSERT_EQ("value", parser.parser().get<0>().get<1>().get<1>());
}

TEST(ObjectUnion, StringType) {
  std::string buf(
      R"(
{
  "type": "1",
  "bool": true
})");

  // clang-format off
  Parser<Object<
    Union<
      std::string,
      Object<Value<bool>>,
      Object<Value<int64_t>>
  >>> parser({{"type", {{"1", "bool"}, {"2", "int"}}}});
  // clang-format on

  ASSERT_TRUE(parser.parse(buf));
  ASSERT_TRUE(parser.finish());

  ASSERT_TRUE(parser.parser().get<0>().parser<0>().isSet());
  ASSERT_FALSE(parser.parser().get<0>().parser<1>().isSet());
  ASSERT_EQ(0, parser.parser().get<0>().currentMemberId());

  ASSERT_EQ(true, parser.parser().get<0>().get<0>().get<0>());

  buf = R"(
{
  "type": "2",
  "int": 100
})";

  ASSERT_TRUE(parser.parse(buf));
  ASSERT_TRUE(parser.finish());

  ASSERT_FALSE(parser.parser().get<0>().parser<0>().isSet());
  ASSERT_TRUE(parser.parser().get<0>().parser<1>().isSet());
  ASSERT_EQ(1, parser.parser().get<0>().currentMemberId());

  ASSERT_EQ(100, parser.parser().get<0>().get<1>().get<0>());
}

TEST(ObjectUnion, StdStringType) {
  std::string buf(
      R"(
{
  "type": "1",
  "bool": true
})");

  std::string types[2] = {"1", "2"};

  // clang-format off
  Parser<Object<
    Union<
      std::string,
      Object<Value<bool>>,
      Object<Value<int64_t>>
  >>> parser({{"type", {{types[0], "bool"}, {types[1], "int"}}}});
  // clang-format on

  ASSERT_TRUE(parser.parse(buf));
  ASSERT_TRUE(parser.finish());

  ASSERT_TRUE(parser.parser().get<0>().parser<0>().isSet());
  ASSERT_FALSE(parser.parser().get<0>().parser<1>().isSet());
  ASSERT_EQ(0, parser.parser().get<0>().currentMemberId());

  ASSERT_EQ(true, parser.parser().get<0>().get<0>().get<0>());

  buf = R"(
{
  "type": "2",
  "int": 100
})";

  ASSERT_TRUE(parser.parse(buf));
  ASSERT_TRUE(parser.finish());

  ASSERT_FALSE(parser.parser().get<0>().parser<0>().isSet());
  ASSERT_TRUE(parser.parser().get<0>().parser<1>().isSet());
  ASSERT_EQ(1, parser.parser().get<0>().currentMemberId());

  ASSERT_EQ(100, parser.parser().get<0>().get<1>().get<0>());
}

TEST(ObjectUnion, IncorrectTypeType) {
  std::string buf(
      R"(
{
  "type": "1",
  "bool": true
})");

  // clang-format off
  Parser<Object<
    Union<
      int64_t,
      Object<Value<bool>>,
      Object<Value<int64_t>>
  >>> parser({{"type", {{1, "bool"}, {2, "int"}}}});
  // clang-format on

  ASSERT_FALSE(parser.parse(buf));
  ASSERT_FALSE(parser.parser().parser<0>().isSet());

  ASSERT_EQ("Unexpected token string", parser.getError());
  ASSERT_EQ(
      R"(parse error: client cancelled parse via callback return value
                         {   "type": "1",   "bool": true }
                     (right here) ------^
Unexpected token string
)",
      parser.getError(true));
}

TEST(ObjectUnion, IncorrectTypeValue) {
  std::string buf(
      R"(
{
  "type": 3,
  "bool": true
})");

  // clang-format off
  Parser<Object<
    Union<
      int64_t,
      Object<Value<bool>>,
      Object<Value<int64_t>>
  >>> parser({{"type", {{1, "bool"}, {2, "int"}}}});
  // clang-format on

  ASSERT_FALSE(parser.parse(buf));
  ASSERT_FALSE(parser.parser().parser<0>().isSet());

  ASSERT_EQ("Unexpected field 3", parser.getError());
  ASSERT_EQ(
      R"(parse error: client cancelled parse via callback return value
                           {   "type": 3,   "bool": true }
                     (right here) ------^
Unexpected field 3
)",
      parser.getError(true));
}

TEST(ObjectUnion, FieldsWithCallbacks) {
  std::string buf(
      R"(
{
  "type": 1,
  "bool": true
})");

  bool bool_value = false;
  int64_t int_value;

  auto boolCb = [&](Object<Value<bool>> &parser) {
    bool_value = parser.get<0>();
    return true;
  };

  auto intCb = [&](Object<Value<int64_t>> &parser) {
    int_value = parser.get<0>();
    return true;
  };

  // clang-format off
  Parser<Object<
    Union<
      int64_t,
      Object<Value<bool>>,
      Object<Value<int64_t>>
  >>> parser({{"type", {{1, {"bool", boolCb}}, {2, {"int", intCb}}}}});
  // clang-format on

  ASSERT_TRUE(parser.parse(buf));
  ASSERT_TRUE(parser.finish());

  ASSERT_EQ(true, parser.parser().get<0>().get<0>().get<0>());
  ASSERT_EQ(true, bool_value);

  buf = R"(
{
  "type": 2,
  "int": 100
})";

  ASSERT_TRUE(parser.parse(buf));
  ASSERT_TRUE(parser.finish());

  ASSERT_EQ(100, parser.parser().get<0>().get<1>().get<0>());
  ASSERT_EQ(100, int_value);
}

TEST(ObjectUnion, FieldsWithCallbackError) {
  std::string buf(
      R"(
{
  "type": 1,
  "bool": true
})");

  auto boolCb = [&](Object<Value<bool>> &) { return false; };

  auto intCb = [&](Object<Value<int64_t>> &) { return false; };

  // clang-format off
  Parser<Object<
    Union<
      int64_t,
      Object<Value<bool>>,
      Object<Value<int64_t>>
  >>> parser({{"type", {{1, {"bool", boolCb}}, {2, {"int", intCb}}}}});
  // clang-format on

  ASSERT_FALSE(parser.parse(buf));
  ASSERT_FALSE(parser.parser().parser<0>().isSet());

  ASSERT_EQ("Callback returned false", parser.getError());
  ASSERT_EQ(
      R"(parse error: client cancelled parse via callback return value
             "type": 1,   "bool": true }
                     (right here) ------^
Callback returned false
)",
      parser.getError(true));

  buf = R"(
{
  "type": 2,
  "int": 100
})";

  ASSERT_FALSE(parser.parse(buf));
  ASSERT_FALSE(parser.parser().parser<0>().isSet());

  ASSERT_EQ("Callback returned false", parser.getError());
  ASSERT_EQ(
      R"(parse error: client cancelled parse via callback return value
                                         {   "type": 2,   "int": 100 }
                     (right here) ------^
Callback returned false
)",
      parser.getError(true));
}

TEST(ObjectUnion, UnionWithCallback) {
  std::string buf(
      R"(
{
  "type": 1,
  "bool": true
})");

  bool bool_value = false;
  int64_t int_value;

  // clang-format off
  using UnionParser = Union<
                        int64_t,
                        Object<Value<bool>>,
                        Object<Value<int64_t>>>;
  // clang-format on

  auto unionCb = [&](UnionParser &parser) {
    if (parser.currentMemberId() == 0) {
      bool_value = parser.get<0>().get<0>();
    } else {
      int_value = parser.get<1>().get<0>();
    }
    return true;
  };

  Parser<Object<UnionParser>> parser(
      {{"type", {{{1, "bool"}, {2, "int"}}, unionCb}}});

  ASSERT_TRUE(parser.parse(buf));
  ASSERT_TRUE(parser.finish());

  ASSERT_EQ(true, parser.parser().get<0>().get<0>().get<0>());
  ASSERT_EQ(true, bool_value);

  buf = R"(
{
  "type": 2,
  "int": 100
})";

  ASSERT_TRUE(parser.parse(buf));
  ASSERT_TRUE(parser.finish());

  ASSERT_EQ(100, parser.parser().get<0>().get<1>().get<0>());
  ASSERT_EQ(100, int_value);
}

TEST(ObjectUnion, UnionWithCallbackError) {
  std::string buf(R"(
{
  "type": 1,
  "bool": true
})");

  // clang-format off
  using UnionParser = Union<
                        int64_t,
                        Object<Value<bool>>,
                        Object<Value<int64_t>>>;
  // clang-format on

  auto unionCb = [&](UnionParser &) { return false; };

  Parser<Object<UnionParser>> parser(
      {{"type", {{{1, "bool"}, {2, "int"}}, unionCb}}});

  ASSERT_FALSE(parser.parse(buf));
  ASSERT_TRUE(parser.parser().parser<0>().isSet());

  ASSERT_EQ("Callback returned false", parser.getError());
  ASSERT_EQ(
      R"(parse error: client cancelled parse via callback return value
             "type": 1,   "bool": true }
                     (right here) ------^
Callback returned false
)",
      parser.getError(true));
}

TEST(ObjectUnion, UnionWithArgsStruct) {
  std::string buf(
      R"(
{
  "type": 1,
  "bool": true
})");

  // clang-format off
  using UnionParser = Union<
                        int64_t,
                        Object<Value<bool>>,
                        Object<Value<int64_t>>>;
  // clang-format on

  UnionParser::Args union_args({{{1, "bool"}, {2, "int"}}});

  Parser<Object<UnionParser>> parser({{"type", union_args}});

  ASSERT_TRUE(parser.parse(buf));
  ASSERT_TRUE(parser.finish());

  ASSERT_EQ(true, parser.parser().get<0>().get<0>().get<0>());

  buf = R"(
{
  "type": 2,
  "int": 100
})";

  ASSERT_TRUE(parser.parse(buf));
  ASSERT_TRUE(parser.finish());

  ASSERT_EQ(100, parser.parser().get<0>().get<1>().get<0>());
}

TEST(ObjectUnion, UnionWithUnexpectedObject) {
  std::string buf(
      R"(
{
  "type": 1,
  "error": true
})");

  // clang-format off
  Parser<Object<
    Union<
      int64_t,
      Object<Value<bool>>,
      Object<Value<int64_t>>
  >>> parser({{"type", {{1, "bool"}, {2, "int"}}}});
  // clang-format on

  ASSERT_FALSE(parser.parse(buf));
  ASSERT_FALSE(parser.parser().parser<0>().isSet());

  ASSERT_EQ("Unexpected field error", parser.getError());
  ASSERT_EQ(
      R"(parse error: client cancelled parse via callback return value
                {   "type": 1,   "error": true }
                     (right here) ------^
Unexpected field error
)",
      parser.getError(true));
}

TEST(ObjectUnion, UnionWithSCustomObject) {
  std::string buf(
      R"(
{
  "type": 1,
  "bool": true,
  "string": "value"
})");

  struct ObjectStruct {
    bool bool_field;
    std::string str_field;
  };

  using InnerObjectParser =
      SObject<ObjectStruct, Value<bool>, Value<std::string>>;

  auto innerObjectCb = [&](InnerObjectParser &parser, ObjectStruct &value) {
    value = {parser.parser<0>().pop(), parser.parser<1>().pop()};
    return true;
  };

  // clang-format off
  Parser<Object<
    Union<
      int64_t,
      InnerObjectParser,
      Object<Value<int64_t>>
  >>> parser({{
      "type", {
      {
        1, {{"bool", "string"}, innerObjectCb}
      }, {
        2, "int"
      }}}});
  // clang-format on

  ASSERT_TRUE(parser.parse(buf));
  ASSERT_TRUE(parser.finish());

  ASSERT_EQ(true, parser.parser().get<0>().get<0>().bool_field);
  ASSERT_EQ("value", parser.parser().get<0>().get<0>().str_field);

  buf = R"(
{
  "type": 2,
  "int": 100
})";

  ASSERT_TRUE(parser.parse(buf));
  ASSERT_TRUE(parser.finish());

  ASSERT_EQ(100, parser.parser().get<0>().get<1>().get<0>());
}

TEST(ObjectUnion, UnionWithSAutoObject) {
  std::string buf(
      R"(
{
  "type": 1,
  "bool": true,
  "string": "value"
})");

  // clang-format off
  Parser<Object<
    Union<
      int64_t,
      SObject<Value<bool>, Value<std::string>>,
      Object<Value<int64_t>>
  >>> parser({{
      "type", {
      {
        1, {"bool", "string"}
      }, {
        2, "int"
      }}}});
  // clang-format on

  ASSERT_TRUE(parser.parse(buf));
  ASSERT_TRUE(parser.finish());

  ASSERT_EQ(true, std::get<0>(parser.parser().get<0>().get<0>()));
  ASSERT_EQ("value", std::get<1>(parser.parser().get<0>().get<0>()));

  buf = R"(
{
  "type": 2,
  "int": 100
})";

  ASSERT_TRUE(parser.parse(buf));
  ASSERT_TRUE(parser.finish());

  ASSERT_EQ(100, parser.parser().get<0>().get<1>().get<0>());
}

TEST(ObjectUnion, UnionWithObjectUnion) {
  std::string buf(
      R"(
{
  "type": 1,
  "subtype": 1,
  "bool": true
})");

  // clang-format off
  Parser<Object<
    Union<
      int64_t,
      Union<
        int64_t,
        Object<Value<bool>>,
        Object<Value<int64_t>>
      >,
      Object<Value<std::string>>
  >>> parser({{
      "type", {
        {
          1, {"subtype", {{1, "bool"}, {2, "int"}}}
        }, {
          2, "string"
        }}}});
  // clang-format on

  ASSERT_TRUE(parser.parse(buf));
  ASSERT_TRUE(parser.finish());

  ASSERT_EQ(true, parser.parser().get<0>().get<0>().get<0>().get<0>());

  buf = R"(
{
  "type": 1,
  "subtype": 2,
  "int": 100
})";

  ASSERT_TRUE(parser.parse(buf));
  ASSERT_TRUE(parser.finish());

  ASSERT_EQ(100, parser.parser().get<0>().get<0>().get<1>().get<0>());

  buf = R"(
{
  "type": 2,
  "string": "value"
})";

  ASSERT_TRUE(parser.parse(buf));
  ASSERT_TRUE(parser.finish());

  ASSERT_EQ("value", parser.parser().get<0>().get<1>().get<0>());
}

TEST(ObjectUnion, UnionWithUnexpectedMapStart) {
  std::string buf(
      R"(
{
  "type": 1,
  "bool": true
})");

  // clang-format off
  Parser<Union<
    int64_t,
    Object<Value<bool>>,
    Object<Value<int64_t>>
  >> parser({{1, "bool"}, {2, "int"}});
  // clang-format on

  ASSERT_FALSE(parser.parse(buf));
  ASSERT_FALSE(parser.parser().parser<0>().isSet());

  ASSERT_EQ("Union with an empty type field can't parse this",
            parser.getError());
  ASSERT_EQ(
      R"(parse error: client cancelled parse via callback return value
                                       {   "type": 1,   "bool": true }
                     (right here) ------^
Union with an empty type field can't parse this
)",
      parser.getError(true));
}
