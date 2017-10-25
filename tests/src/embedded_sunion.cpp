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
#include "sjparser/sjparser.h"
#include "test_parser.h"

using namespace SJParser;

TEST(EmbeddedSUnion, Empty) {
  std::string buf(R"({})");

  // clang-format off
  Parser<Object<
    SUnion<
      int64_t,
      SObject<Value<bool>>,
      SObject<Value<int64_t>>
  >>> parser({{"type", {{1, "bool"}, {2, "int"}}}});
  // clang-format on

  ASSERT_NO_THROW(parser.parse(buf));
  ASSERT_NO_THROW(parser.finish());

  ASSERT_TRUE(parser.parser().isSet());
  ASSERT_FALSE(parser.parser().parser<0>().isSet());
}

TEST(EmbeddedSUnion, Null) {
  std::string buf(R"(null)");

  // clang-format off
  Parser<Object<
    SUnion<
      int64_t,
      SObject<Value<bool>>,
      SObject<Value<int64_t>>
  >>> parser({{"type", {{1, "bool"}, {2, "int"}}}});
  // clang-format on

  ASSERT_NO_THROW(parser.parse(buf));
  ASSERT_NO_THROW(parser.finish());

  ASSERT_FALSE(parser.parser().isSet());
  ASSERT_FALSE(parser.parser().parser<0>().isSet());
}

TEST(EmbeddedSUnion, Reset) {
  std::string buf(
      R"({"type": 1, "bool": true, "integer": 10})");

  // clang-format off
  Parser<Object<
    SUnion<
      int64_t,
      SObject<
        Value<bool>,
        Value<int64_t>>,
      SObject<
        Value<bool>>
  >>> parser({{"type", {{1, {"bool", "integer"}}, {2, {"bool"}}}}});
  // clang-format on

  ASSERT_NO_THROW(parser.parse(buf));
  ASSERT_NO_THROW(parser.finish());

  auto variant = parser.parser().get<0>();

  ASSERT_EQ(0, variant.index());

  auto object = std::get<0>(variant);

  ASSERT_EQ(true, std::get<0>(object));
  ASSERT_EQ(10, std::get<1>(object));

  buf = R"(null)";

  ASSERT_NO_THROW(parser.parse(buf));
  ASSERT_NO_THROW(parser.finish());

  ASSERT_FALSE(parser.parser().isSet());
  ASSERT_FALSE(parser.parser().parser<0>().isSet());
}

TEST(EmbeddedSUnion, AllValuesFields) {
  std::string buf(
      R"({"type": 1, "bool": true, "integer": 10})");

  // clang-format off
  Parser<Object<
    SUnion<
      int64_t,
      SObject<
        Value<bool>,
        Value<int64_t>>,
      SObject<
        Value<double>,
        Value<std::string>>
  >>> parser({{"type", {{1, {"bool", "integer"}}, {2, {"double", "string"}}}}});
  // clang-format on

  ASSERT_NO_THROW(parser.parse(buf));
  ASSERT_NO_THROW(parser.finish());

  {
    auto variant = parser.parser().get<0>();

    ASSERT_EQ(0, variant.index());

    auto object = std::get<0>(variant);

    ASSERT_EQ(true, std::get<0>(object));
    ASSERT_EQ(10, std::get<1>(object));
  }

  buf = R"({"type": 2, "double": 11.5, "string": "value"})";

  ASSERT_NO_THROW(parser.parse(buf));
  ASSERT_NO_THROW(parser.finish());

  {
    auto variant = parser.parser().get<0>();

    ASSERT_EQ(1, variant.index());

    auto object = std::get<1>(variant);

    ASSERT_EQ(11.5, std::get<0>(object));
    ASSERT_EQ("value", std::get<1>(object));
  }
}

TEST(EmbeddedSUnion, StringType) {
  std::string buf(
      R"(
{
  "type": "1",
  "bool": true
})");

  // clang-format off
  Parser<Object<
    SUnion<
      std::string,
      SObject<Value<bool>>,
      SObject<Value<int64_t>>
  >>> parser({{"type", {{"1", "bool"}, {"2", "int"}}}});
  // clang-format on

  ASSERT_NO_THROW(parser.parse(buf));
  ASSERT_NO_THROW(parser.finish());

  {
    auto variant = parser.parser().get<0>();

    ASSERT_EQ(0, variant.index());

    ASSERT_EQ(true, std::get<0>(std::get<0>(variant)));
  }

  buf = R"(
{
  "type": "2",
  "int": 100
})";

  ASSERT_NO_THROW(parser.parse(buf));
  ASSERT_NO_THROW(parser.finish());

  {
    auto variant = parser.parser().get<0>();

    ASSERT_EQ(1, variant.index());

    ASSERT_EQ(100, std::get<0>(std::get<1>(variant)));
  }
}

TEST(EmbeddedSUnion, StdStringType) {
  std::string buf(
      R"(
{
  "type": "1",
  "bool": true
})");

  std::string types[2] = {"1", "2"};

  // clang-format off
  Parser<Object<
    SUnion<
      std::string,
      SObject<Value<bool>>,
      SObject<Value<int64_t>>
  >>> parser({{"type", {{types[0], "bool"}, {types[1], "int"}}}});
  // clang-format on

  ASSERT_NO_THROW(parser.parse(buf));
  ASSERT_NO_THROW(parser.finish());

  {
    auto variant = parser.parser().get<0>();

    ASSERT_EQ(0, variant.index());

    ASSERT_EQ(true, std::get<0>(std::get<0>(variant)));
  }

  buf = R"(
{
  "type": "2",
  "int": 100
})";

  ASSERT_NO_THROW(parser.parse(buf));
  ASSERT_NO_THROW(parser.finish());

  {
    auto variant = parser.parser().get<0>();

    ASSERT_EQ(1, variant.index());

    ASSERT_EQ(100, std::get<0>(std::get<1>(variant)));
  }
}

TEST(EmbeddedSUnion, IncorrectTypeType) {
  std::string buf(
      R"(
{
  "type": "1",
  "bool": true
})");

  // clang-format off
  Parser<Object<
    SUnion<
      int64_t,
      SObject<Value<bool>>,
      SObject<Value<int64_t>>
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

TEST(EmbeddedSUnion, IncorrectTypeValue) {
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
      SObject<Value<bool>>,
      SObject<Value<int64_t>>
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

TEST(EmbeddedSUnion, FieldsWithCallbacks) {
  std::string buf(
      R"(
{
  "type": 1,
  "bool": true
})");

  bool bool_value = false;
  int64_t int_value;

  auto boolCb = [&](const std::tuple<bool> &value) {
    bool_value = std::get<0>(value);
    return true;
  };

  auto intCb = [&](const std::tuple<int64_t> &value) {
    int_value = std::get<0>(value);
    return true;
  };

  // clang-format off
  Parser<Object<
    SUnion<
      int64_t,
      SObject<Value<bool>>,
      SObject<Value<int64_t>>
  >>> parser({{"type", {{1, {"bool", boolCb}}, {2, {"int", intCb}}}}});
  // clang-format on

  ASSERT_NO_THROW(parser.parse(buf));
  ASSERT_NO_THROW(parser.finish());

  ASSERT_EQ(true, std::get<0>(std::get<0>(parser.parser().get<0>())));
  ASSERT_EQ(true, bool_value);

  buf = R"(
{
  "type": 2,
  "int": 100
})";

  ASSERT_NO_THROW(parser.parse(buf));
  ASSERT_NO_THROW(parser.finish());

  ASSERT_EQ(100, std::get<0>(std::get<1>(parser.parser().get<0>())));
  ASSERT_EQ(100, int_value);
}

TEST(EmbeddedSUnion, FieldsWithCallbackError) {
  std::string buf(
      R"(
{
  "type": 1,
  "bool": true
})");

  auto boolCb = [&](const std::tuple<bool> &) { return false; };

  auto intCb = [&](const std::tuple<int64_t> &) { return false; };

  // clang-format off
  Parser<Object<
    SUnion<
      int64_t,
      SObject<Value<bool>>,
      SObject<Value<int64_t>>
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

TEST(EmbeddedSUnion, SUnionWithCallback) {
  std::string buf(
      R"(
{
  "type": 1,
  "bool": true
})");

  bool bool_value = false;
  int64_t int_value;

  // clang-format off
  using UnionParser = SUnion<
                        int64_t,
                        SObject<Value<bool>>,
                        SObject<Value<int64_t>>>;
  // clang-format on

  auto unionCb =
      [&](const std::variant<std::tuple<bool>, std::tuple<int64_t>> &value) {
        if (value.index() == 0) {
          bool_value = std::get<0>(std::get<0>(value));
        } else {
          int_value = std::get<0>(std::get<1>(value));
        }
        return true;
      };

  Parser<Object<UnionParser>> parser(
      {{"type", {{{1, "bool"}, {2, "int"}}, unionCb}}});

  ASSERT_NO_THROW(parser.parse(buf));
  ASSERT_NO_THROW(parser.finish());

  ASSERT_EQ(true, std::get<0>(std::get<0>(parser.parser().get<0>())));
  ASSERT_EQ(true, bool_value);

  buf = R"(
{
  "type": 2,
  "int": 100
})";

  ASSERT_NO_THROW(parser.parse(buf));
  ASSERT_NO_THROW(parser.finish());

  ASSERT_EQ(100, std::get<0>(std::get<1>(parser.parser().get<0>())));
  ASSERT_EQ(100, int_value);
}

TEST(EmbeddedSUnion, SUnionWithCallbackError) {
  std::string buf(R"(
{
  "type": 1,
  "bool": true
})");

  // clang-format off
  using UnionParser = SUnion<
                        int64_t,
                        SObject<Value<bool>>,
                        SObject<Value<int64_t>>>;
  // clang-format on

  auto unionCb =
      [&](const std::variant<std::tuple<bool>, std::tuple<int64_t>> &) {
        return false;
      };

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

TEST(EmbeddedSUnion, SUnionWithArgsStruct) {
  std::string buf(
      R"(
{
  "type": 1,
  "bool": true
})");

  // clang-format off
  using UnionParser = SUnion<
                        int64_t,
                        SObject<Value<bool>>,
                        SObject<Value<int64_t>>>;
  // clang-format on

  UnionParser::Args union_args({{{1, "bool"}, {2, "int"}}});

  Parser<Object<UnionParser>> parser({{"type", union_args}});

  ASSERT_NO_THROW(parser.parse(buf));
  ASSERT_NO_THROW(parser.finish());

  ASSERT_EQ(true, std::get<0>(std::get<0>(parser.parser().get<0>())));

  buf = R"(
{
  "type": 2,
  "int": 100
})";

  ASSERT_NO_THROW(parser.parse(buf));
  ASSERT_NO_THROW(parser.finish());

  ASSERT_EQ(100, std::get<0>(std::get<1>(parser.parser().get<0>())));
}

TEST(EmbeddedSUnion, SUnionWithUnexpectedObject) {
  std::string buf(
      R"(
{
  "type": 1,
  "error": true
})");

  // clang-format off
  Parser<Object<
    SUnion<
      int64_t,
      SObject<Value<bool>>,
      SObject<Value<int64_t>>
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

TEST(EmbeddedSUnion, SUnionWithSCustomObject) {
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
    SUnion<
      int64_t,
      InnerObjectParser,
      SObject<Value<int64_t>>
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

  {
    auto value = std::get<0>(parser.parser().get<0>());

    ASSERT_EQ(true, value.bool_field);
    ASSERT_EQ("value", value.str_field);
  }

  buf = R"(
{
  "type": 2,
  "int": 100
})";

  ASSERT_NO_THROW(parser.parse(buf));
  ASSERT_NO_THROW(parser.finish());

  ASSERT_EQ(100, std::get<0>(std::get<1>(parser.parser().get<0>())));
}

TEST(EmbeddedSUnion, SUnionWithEmbeddedSUnion) {
  std::string buf(
      R"(
{
  "type": 1,
  "subtype": 1,
  "bool": true
})");

  // clang-format off
  Parser<Object<
    SUnion<
      int64_t,
      SUnion<
        int64_t,
        SObject<Value<bool>>,
        SObject<Value<int64_t>>
      >,
      SObject<Value<std::string>>
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

  ASSERT_EQ(true,
            std::get<0>(std::get<0>(std::get<0>(parser.parser().get<0>()))));

  buf = R"(
{
  "type": 1,
  "subtype": 2,
  "int": 100
})";

  ASSERT_NO_THROW(parser.parse(buf));
  ASSERT_NO_THROW(parser.finish());

  ASSERT_EQ(100,
            std::get<0>(std::get<1>(std::get<0>(parser.parser().get<0>()))));

  buf = R"(
{
  "type": 2,
  "string": "value"
})";

  ASSERT_NO_THROW(parser.parse(buf));
  ASSERT_NO_THROW(parser.finish());

  ASSERT_EQ("value", std::get<0>(std::get<1>(parser.parser().get<0>())));
}

TEST(EmbeddedSUnion, SUnionWithUnexpectedMapStart) {
  std::string buf(
      R"(
{
  "type": 1,
  "bool": true
})");

  // clang-format off
  Parser<SUnion<
    int64_t,
    SObject<Value<bool>>,
    SObject<Value<int64_t>>
  >> parser({{1, "bool"}, {2, "int"}});
  // clang-format on

  try {
    parser.parse(buf);
    FAIL() << "No exception thrown";
  } catch (ParsingError &e) {
    ASSERT_FALSE(parser.parser().isSet());

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

TEST(EmbeddedSUnion, SUnionWithUnexpectedMapKey) {
  // clang-format off
  Parser<SUnion<
    int64_t,
    SObject<Value<bool>>,
    SObject<Value<int64_t>>
  >, TestParser> parser({{1, "bool"}, {2, "int"}});
  // clang-format on

  auto test = [](TestParser *parser) {
    parser->dispatcher->on(MapKeyT{"test"});
  };

  try {
    parser.run(test);
    FAIL() << "No exception thrown";
  } catch (std::runtime_error &e) {
    ASSERT_FALSE(parser.parser().isSet());
    ASSERT_STREQ("Union with an empty type field can't parse this", e.what());
  } catch (...) {
    FAIL() << "Invalid exception thrown";
  }
}
