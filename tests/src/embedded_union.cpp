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

TEST(EmbeddedUnion, Empty) {
  std::string buf(R"({})");

  // clang-format off
  Parser<Object<
    Union<
      int64_t,
      Object<Value<bool>>,
      Object<Value<int64_t>>
  >>> parser({{"type", {{1, "bool"}, {2, "int"}}}});
  // clang-format on

  ASSERT_NO_THROW(parser.parse(buf));
  ASSERT_NO_THROW(parser.finish());

  ASSERT_TRUE(parser.parser().isSet());
}

TEST(EmbeddedUnion, Null) {
  std::string buf(R"(null)");

  // clang-format off
  Parser<Object<
    Union<
      int64_t,
      Object<Value<bool>>,
      Object<Value<int64_t>>
  >>> parser({{"type", {{1, "bool"}, {2, "int"}}}});
  // clang-format on

  ASSERT_NO_THROW(parser.parse(buf));
  ASSERT_NO_THROW(parser.finish());

  ASSERT_FALSE(parser.parser().isSet());
}

TEST(EmbeddedUnion, AllValuesFields) {
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

  ASSERT_NO_THROW(parser.parse(buf));
  ASSERT_NO_THROW(parser.finish());

  ASSERT_TRUE(parser.parser().get<0>().parser<0>().isSet());
  ASSERT_FALSE(parser.parser().get<0>().parser<1>().isSet());
  ASSERT_EQ(0, parser.parser().get<0>().currentMemberId());

  ASSERT_EQ(true, parser.parser().get<0>().get<0>().get<0>());
  ASSERT_EQ(10, parser.parser().get<0>().get<0>().get<1>());

  buf = R"({"type": 2, "double": 11.5, "string": "value"})";

  ASSERT_NO_THROW(parser.parse(buf));
  ASSERT_NO_THROW(parser.finish());

  ASSERT_FALSE(parser.parser().get<0>().parser<0>().isSet());
  ASSERT_TRUE(parser.parser().get<0>().parser<1>().isSet());
  ASSERT_EQ(1, parser.parser().get<0>().currentMemberId());

  ASSERT_EQ(11.5, parser.parser().get<0>().get<1>().get<0>());
  ASSERT_EQ("value", parser.parser().get<0>().get<1>().get<1>());
}

TEST(EmbeddedUnion, StringType) {
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

  ASSERT_NO_THROW(parser.parse(buf));
  ASSERT_NO_THROW(parser.finish());

  ASSERT_TRUE(parser.parser().get<0>().parser<0>().isSet());
  ASSERT_FALSE(parser.parser().get<0>().parser<1>().isSet());
  ASSERT_EQ(0, parser.parser().get<0>().currentMemberId());

  ASSERT_EQ(true, parser.parser().get<0>().get<0>().get<0>());

  buf = R"(
{
  "type": "2",
  "int": 100
})";

  ASSERT_NO_THROW(parser.parse(buf));
  ASSERT_NO_THROW(parser.finish());

  ASSERT_FALSE(parser.parser().get<0>().parser<0>().isSet());
  ASSERT_TRUE(parser.parser().get<0>().parser<1>().isSet());
  ASSERT_EQ(1, parser.parser().get<0>().currentMemberId());

  ASSERT_EQ(100, parser.parser().get<0>().get<1>().get<0>());
}

TEST(EmbeddedUnion, StdStringType) {
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

  ASSERT_NO_THROW(parser.parse(buf));
  ASSERT_NO_THROW(parser.finish());

  ASSERT_TRUE(parser.parser().get<0>().parser<0>().isSet());
  ASSERT_FALSE(parser.parser().get<0>().parser<1>().isSet());
  ASSERT_EQ(0, parser.parser().get<0>().currentMemberId());

  ASSERT_EQ(true, parser.parser().get<0>().get<0>().get<0>());

  buf = R"(
{
  "type": "2",
  "int": 100
})";

  ASSERT_NO_THROW(parser.parse(buf));
  ASSERT_NO_THROW(parser.finish());

  ASSERT_FALSE(parser.parser().get<0>().parser<0>().isSet());
  ASSERT_TRUE(parser.parser().get<0>().parser<1>().isSet());
  ASSERT_EQ(1, parser.parser().get<0>().currentMemberId());

  ASSERT_EQ(100, parser.parser().get<0>().get<1>().get<0>());
}

TEST(EmbeddedUnion, IncorrectTypeType) {
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

  try {
    parser.parse(buf);
    FAIL() << "No exception thrown";
  } catch (ParsingError &e) {
    ASSERT_FALSE(parser.parser().parser<0>().isSet());

    ASSERT_EQ("Unexpected token string", e.sjparserError());
    ASSERT_EQ(
        R"(parse error: client cancelled parse via callback return value
                         {   "type": "1",   "bool": true }
                     (right here) ------^
)",
        e.parserError());
  } catch (...) {
    FAIL() << "Invalid exception thrown";
  }
}

TEST(EmbeddedUnion, IncorrectTypeValue) {
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

  try {
    parser.parse(buf);
    FAIL() << "No exception thrown";
  } catch (ParsingError &e) {
    ASSERT_FALSE(parser.parser().parser<0>().isSet());

    ASSERT_EQ("Unexpected field 3", e.sjparserError());
    ASSERT_EQ(
        R"(parse error: client cancelled parse via callback return value
                           {   "type": 3,   "bool": true }
                     (right here) ------^
)",
        e.parserError());
  } catch (...) {
    FAIL() << "Invalid exception thrown";
  }
}

TEST(EmbeddedUnion, FieldsWithCallbacks) {
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

  ASSERT_NO_THROW(parser.parse(buf));
  ASSERT_NO_THROW(parser.finish());

  ASSERT_EQ(true, parser.parser().get<0>().get<0>().get<0>());
  ASSERT_EQ(true, bool_value);

  buf = R"(
{
  "type": 2,
  "int": 100
})";

  ASSERT_NO_THROW(parser.parse(buf));
  ASSERT_NO_THROW(parser.finish());

  ASSERT_EQ(100, parser.parser().get<0>().get<1>().get<0>());
  ASSERT_EQ(100, int_value);
}

TEST(EmbeddedUnion, FieldsWithCallbackError) {
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

  try {
    parser.parse(buf);
    FAIL() << "No exception thrown";
  } catch (ParsingError &e) {
    ASSERT_FALSE(parser.parser().parser<0>().isSet());

    ASSERT_EQ("Callback returned false", e.sjparserError());
    ASSERT_EQ(
        R"(parse error: client cancelled parse via callback return value
             "type": 1,   "bool": true }
                     (right here) ------^
)",
        e.parserError());
  } catch (...) {
    FAIL() << "Invalid exception thrown";
  }

  buf = R"(
{
  "type": 2,
  "int": 100
})";

  try {
    parser.parse(buf);
    FAIL() << "No exception thrown";
  } catch (ParsingError &e) {
    ASSERT_FALSE(parser.parser().parser<0>().isSet());

    ASSERT_EQ("Callback returned false", e.sjparserError());
    ASSERT_EQ(
        R"(parse error: client cancelled parse via callback return value
           {   "type": 2,   "int": 100 }
                     (right here) ------^
)",
        e.parserError());
  } catch (...) {
    FAIL() << "Invalid exception thrown";
  }
}

TEST(EmbeddedUnion, UnionWithCallback) {
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

  ASSERT_NO_THROW(parser.parse(buf));
  ASSERT_NO_THROW(parser.finish());

  ASSERT_EQ(true, parser.parser().get<0>().get<0>().get<0>());
  ASSERT_EQ(true, bool_value);

  buf = R"(
{
  "type": 2,
  "int": 100
})";

  ASSERT_NO_THROW(parser.parse(buf));
  ASSERT_NO_THROW(parser.finish());

  ASSERT_EQ(100, parser.parser().get<0>().get<1>().get<0>());
  ASSERT_EQ(100, int_value);
}

TEST(EmbeddedUnion, UnionWithCallbackError) {
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

  try {
    parser.parse(buf);
    FAIL() << "No exception thrown";
  } catch (ParsingError &e) {
    ASSERT_TRUE(parser.parser().parser<0>().isSet());

    ASSERT_EQ("Callback returned false", e.sjparserError());
    ASSERT_EQ(
        R"(parse error: client cancelled parse via callback return value
             "type": 1,   "bool": true }
                     (right here) ------^
)",
        e.parserError());
  } catch (...) {
    FAIL() << "Invalid exception thrown";
  }
}

TEST(EmbeddedUnion, UnionWithArgsStruct) {
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

  ASSERT_NO_THROW(parser.parse(buf));
  ASSERT_NO_THROW(parser.finish());

  ASSERT_EQ(true, parser.parser().get<0>().get<0>().get<0>());

  buf = R"(
{
  "type": 2,
  "int": 100
})";

  ASSERT_NO_THROW(parser.parse(buf));
  ASSERT_NO_THROW(parser.finish());

  ASSERT_EQ(100, parser.parser().get<0>().get<1>().get<0>());
}

TEST(EmbeddedUnion, UnionWithUnexpectedObject) {
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

  try {
    parser.parse(buf);
    FAIL() << "No exception thrown";
  } catch (ParsingError &e) {
    ASSERT_FALSE(parser.parser().parser<0>().isSet());

    ASSERT_EQ("Unexpected field error", e.sjparserError());
    ASSERT_EQ(
        R"(parse error: client cancelled parse via callback return value
                {   "type": 1,   "error": true }
                     (right here) ------^
)",
        e.parserError());
  } catch (...) {
    FAIL() << "Invalid exception thrown";
  }
}

TEST(EmbeddedUnion, UnionWithSCustomObject) {
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
    value = {parser.pop<0>(), parser.pop<1>()};
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

  ASSERT_NO_THROW(parser.parse(buf));
  ASSERT_NO_THROW(parser.finish());

  ASSERT_EQ(true, parser.parser().get<0>().get<0>().bool_field);
  ASSERT_EQ("value", parser.parser().get<0>().get<0>().str_field);

  buf = R"(
{
  "type": 2,
  "int": 100
})";

  ASSERT_NO_THROW(parser.parse(buf));
  ASSERT_NO_THROW(parser.finish());

  ASSERT_EQ(100, parser.parser().get<0>().get<1>().get<0>());
}

TEST(EmbeddedUnion, UnionWithSAutoObject) {
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

  ASSERT_NO_THROW(parser.parse(buf));
  ASSERT_NO_THROW(parser.finish());

  ASSERT_EQ(true, std::get<0>(parser.parser().get<0>().get<0>()));
  ASSERT_EQ("value", std::get<1>(parser.parser().get<0>().get<0>()));

  buf = R"(
{
  "type": 2,
  "int": 100
})";

  ASSERT_NO_THROW(parser.parse(buf));
  ASSERT_NO_THROW(parser.finish());

  ASSERT_EQ(100, parser.parser().get<0>().get<1>().get<0>());
}

TEST(EmbeddedUnion, UnionWithEmbeddedUnion) {
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

  ASSERT_NO_THROW(parser.parse(buf));
  ASSERT_NO_THROW(parser.finish());

  ASSERT_EQ(true, parser.parser().get<0>().get<0>().get<0>().get<0>());

  buf = R"(
{
  "type": 1,
  "subtype": 2,
  "int": 100
})";

  ASSERT_NO_THROW(parser.parse(buf));
  ASSERT_NO_THROW(parser.finish());

  ASSERT_EQ(100, parser.parser().get<0>().get<0>().get<1>().get<0>());

  buf = R"(
{
  "type": 2,
  "string": "value"
})";

  ASSERT_NO_THROW(parser.parse(buf));
  ASSERT_NO_THROW(parser.finish());

  ASSERT_EQ("value", parser.parser().get<0>().get<1>().get<0>());
}

TEST(EmbeddedUnion, UnionWithUnexpectedMapStart) {
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

  try {
    parser.parse(buf);
    FAIL() << "No exception thrown";
  } catch (ParsingError &e) {
    ASSERT_FALSE(parser.parser().parser<0>().isSet());

    ASSERT_EQ("Union with an empty type field can't parse this",
              e.sjparserError());
    ASSERT_EQ(
        R"(parse error: client cancelled parse via callback return value
                                       {   "type": 1,   "bool": true }
                     (right here) ------^
)",
        e.parserError());
  } catch (...) {
    FAIL() << "Invalid exception thrown";
  }
}
