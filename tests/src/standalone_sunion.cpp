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

using namespace SJParser;

TEST(StandaloneSUnion, Empty) {
  std::string buf(R"({})");

  // clang-format off
  Parser<SUnion<
    int64_t,
    SObject<Value<bool>>,
    SObject<Value<int64_t>>
  >> parser({"type", {{1, "bool"}, {2, "int"}}});
  // clang-format on

  try {
    parser.parse(buf);
    FAIL() << "No exception thrown";
  } catch (ParsingError &e) {
    ASSERT_FALSE(parser.parser().isSet());
    ASSERT_EQ("Can not set value: Empty storage union without a default value",
              e.sjparserError());

    ASSERT_EQ(
        R"(parse error: client cancelled parse via callback return value
                                      {}
                     (right here) ------^
)",
        e.parserError());
  } catch (...) {
    FAIL() << "Invalid exception thrown";
  }
}

TEST(StandaloneSUnion, EmptyDefault) {
  std::string buf(R"({})");

  // clang-format off
  using ObjectParser = SObject<Value<int64_t>>;
  using UnionParser = SUnion<
                        int64_t,
                        SObject<Value<bool>>,
                        ObjectParser>;
  // clang-format on

  UnionParser::Type default_value = ObjectParser::Type(100);

  // clang-format off
  Parser<UnionParser> parser(
      {"type", {{1, "bool"}, {2, "int"}}, default_value});
  // clang-format on

  ASSERT_NO_THROW(parser.parse(buf));
  ASSERT_NO_THROW(parser.finish());

  ASSERT_TRUE(parser.parser().isSet());

  ASSERT_EQ(1, parser.parser().get().index());
  ASSERT_EQ(100, std::get<0>(std::get<1>(parser.parser().get())));
}

TEST(StandaloneSUnion, EmptyDefaultWithCallback) {
  std::string buf(R"({})");

  // clang-format off
  using ObjectParser = SObject<Value<int64_t>>;
  using UnionParser = SUnion<
                        int64_t,
                        SObject<Value<bool>>,
                        ObjectParser>;
  // clang-format on
  bool callback_called = false;

  auto unionCb = [&](const UnionParser::Type &) {
    callback_called = true;
    return true;
  };

  UnionParser::Type default_value = ObjectParser::Type(100);

  // clang-format off
  Parser<UnionParser> parser(
      {"type", {{1, "bool"}, {2, "int"}}, default_value, unionCb});
  // clang-format on

  ASSERT_NO_THROW(parser.parse(buf));
  ASSERT_NO_THROW(parser.finish());

  ASSERT_TRUE(parser.parser().isSet());

  ASSERT_EQ(1, parser.parser().get().index());
  ASSERT_EQ(100, std::get<0>(std::get<1>(parser.parser().get())));
}

TEST(StandaloneSUnion, Null) {
  std::string buf(R"(null)");

  // clang-format off
  Parser<SUnion<
    int64_t,
    SObject<Value<bool>>,
    SObject<Value<int64_t>>
  >> parser({"type", {{1, "bool"}, {2, "int"}}});
  // clang-format on

  ASSERT_NO_THROW(parser.parse(buf));
  ASSERT_NO_THROW(parser.finish());

  ASSERT_FALSE(parser.parser().isSet());
}

TEST(StandaloneSUnion, Reset) {
  std::string buf(
      R"({"type": 1, "bool": true, "integer": 10})");

  // clang-format off
  Parser<SUnion<
    int64_t,
    SObject<
      Value<bool>,
      Value<int64_t>>,
    SObject<
      Value<bool>>
  >> parser({"type", {{1, {"bool", "integer"}}, {2, {"bool"}}}});
  // clang-format on

  ASSERT_NO_THROW(parser.parse(buf));
  ASSERT_NO_THROW(parser.finish());

  auto variant = parser.parser().get();

  ASSERT_EQ(0, variant.index());

  auto object = std::get<0>(variant);

  ASSERT_EQ(true, std::get<0>(object));
  ASSERT_EQ(10, std::get<1>(object));

  buf = R"(null)";

  ASSERT_NO_THROW(parser.parse(buf));
  ASSERT_NO_THROW(parser.finish());

  ASSERT_FALSE(parser.parser().isSet());
}

TEST(StandaloneSUnion, AllValuesFields) {
  std::string buf(
      R"({"type": 1, "bool": true, "integer": 10})");

  // clang-format off
  Parser<SUnion<
    int64_t,
    SObject<
      Value<bool>,
      Value<int64_t>>,
    SObject<
      Value<double>,
      Value<std::string>>
  >> parser({"type", {{1, {"bool", "integer"}}, {2, {"double", "string"}}}});
  // clang-format on

  ASSERT_NO_THROW(parser.parse(buf));
  ASSERT_NO_THROW(parser.finish());

  {
    auto variant = parser.parser().get();

    ASSERT_EQ(0, variant.index());

    auto object = std::get<0>(variant);

    ASSERT_EQ(true, std::get<0>(object));
    ASSERT_EQ(10, std::get<1>(object));
  }

  buf = R"({"type": 2, "double": 11.5, "string": "value"})";

  ASSERT_NO_THROW(parser.parse(buf));
  ASSERT_NO_THROW(parser.finish());

  {
    auto variant = parser.parser().get();

    ASSERT_EQ(1, variant.index());

    auto object = std::get<1>(variant);

    ASSERT_EQ(11.5, std::get<0>(object));
    ASSERT_EQ("value", std::get<1>(object));
  }
}

TEST(StandaloneSUnion, StringType) {
  std::string buf(
      R"(
{
  "type": "1",
  "bool": true
})");

  // clang-format off
  Parser<SUnion<
    std::string,
    SObject<Value<bool>>,
    SObject<Value<int64_t>>
  >> parser({"type", {{"1", "bool"}, {"2", "int"}}});
  // clang-format on

  ASSERT_NO_THROW(parser.parse(buf));
  ASSERT_NO_THROW(parser.finish());

  {
    auto variant = parser.parser().get();

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
    auto variant = parser.parser().get();

    ASSERT_EQ(1, variant.index());

    ASSERT_EQ(100, std::get<0>(std::get<1>(variant)));
  }
}

TEST(StandaloneSUnion, StdStringType) {
  std::string buf(
      R"(
{
  "type": "1",
  "bool": true
})");

  std::string types[2] = {"1", "2"};

  // clang-format off
  Parser<SUnion<
    std::string,
    SObject<Value<bool>>,
    SObject<Value<int64_t>>
  >> parser({"type", {{types[0], "bool"}, {types[1], "int"}}});
  // clang-format on

  ASSERT_NO_THROW(parser.parse(buf));
  ASSERT_NO_THROW(parser.finish());

  {
    auto variant = parser.parser().get();

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
    auto variant = parser.parser().get();

    ASSERT_EQ(1, variant.index());

    ASSERT_EQ(100, std::get<0>(std::get<1>(variant)));
  }
}

TEST(StandaloneSUnion, IncorrectTypeType) {
  std::string buf(
      R"(
{
  "type": "1",
  "bool": true
})");

  // clang-format off
  Parser<SUnion<
    int64_t,
    SObject<Value<bool>>,
    SObject<Value<int64_t>>
  >> parser({"type", {{1, "bool"}, {2, "int"}}});
  // clang-format on

  try {
    parser.parse(buf);
    FAIL() << "No exception thrown";
  } catch (ParsingError &e) {
    ASSERT_FALSE(parser.parser().isSet());

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

TEST(StandaloneSUnion, IncorrectTypeValue) {
  std::string buf(
      R"(
{
  "type": 3,
  "bool": true
})");

  // clang-format off
  Parser<SUnion<
    int64_t,
    SObject<Value<bool>>,
    SObject<Value<int64_t>>
  >> parser({"type", {{1, "bool"}, {2, "int"}}});
  // clang-format on

  try {
    parser.parse(buf);
    FAIL() << "No exception thrown";
  } catch (ParsingError &e) {
    ASSERT_FALSE(parser.parser().isSet());

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

TEST(StandaloneSUnion, IncorrectTypeField) {
  std::string buf(
      R"(
{
  "error": 1,
  "bool": true
})");

  // clang-format off
  Parser<SUnion<
    int64_t,
    SObject<Value<bool>>,
    SObject<Value<int64_t>>
  >> parser({"type", {{1, "bool"}, {2, "int"}}});
  // clang-format on

  try {
    parser.parse(buf);
    FAIL() << "No exception thrown";
  } catch (ParsingError &e) {
    ASSERT_FALSE(parser.parser().isSet());

    ASSERT_EQ("Unexpected field error", e.sjparserError());
    ASSERT_EQ(
        R"(parse error: client cancelled parse via callback return value
                             {   "error": 1,   "bool": true }
                     (right here) ------^
)",
        e.parserError());
  } catch (...) {
    FAIL() << "Invalid exception thrown";
  }
}

TEST(StandaloneSUnion, FieldsWithCallbacks) {
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
  Parser<SUnion<
    int64_t,
    SObject<Value<bool>>,
    SObject<Value<int64_t>>
  >> parser({"type", {{1, {"bool", boolCb}}, {2, {"int", intCb}}}});
  // clang-format on

  ASSERT_NO_THROW(parser.parse(buf));
  ASSERT_NO_THROW(parser.finish());

  ASSERT_EQ(true, std::get<0>(std::get<0>(parser.parser().get())));
  ASSERT_EQ(true, bool_value);

  buf = R"(
{
  "type": 2,
  "int": 100
})";

  ASSERT_NO_THROW(parser.parse(buf));
  ASSERT_NO_THROW(parser.finish());

  ASSERT_EQ(100, std::get<0>(std::get<1>(parser.parser().get())));
  ASSERT_EQ(100, int_value);
}

TEST(StandaloneSUnion, FieldsWithCallbackError) {
  std::string buf(
      R"(
{
  "type": 1,
  "bool": true
})");

  auto boolCb = [&](const std::tuple<bool> &) { return false; };

  auto intCb = [&](const std::tuple<int64_t> &) { return false; };

  // clang-format off
  Parser<SUnion<
    int64_t,
    SObject<Value<bool>>,
    SObject<Value<int64_t>>
  >> parser({"type", {{1, {"bool", boolCb}}, {2, {"int", intCb}}}});
  // clang-format on

  try {
    parser.parse(buf);
    FAIL() << "No exception thrown";
  } catch (ParsingError &e) {
    ASSERT_FALSE(parser.parser().isSet());

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
    ASSERT_FALSE(parser.parser().isSet());

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

TEST(StandaloneSUnion, SUnionWithCallback) {
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

  Parser<UnionParser> parser({"type", {{1, "bool"}, {2, "int"}}, unionCb});

  ASSERT_NO_THROW(parser.parse(buf));
  ASSERT_NO_THROW(parser.finish());

  ASSERT_EQ(true, std::get<0>(std::get<0>(parser.parser().get())));
  ASSERT_EQ(true, bool_value);

  buf = R"(
{
  "type": 2,
  "int": 100
})";

  ASSERT_NO_THROW(parser.parse(buf));
  ASSERT_NO_THROW(parser.finish());

  ASSERT_EQ(100, std::get<0>(std::get<1>(parser.parser().get())));
  ASSERT_EQ(100, int_value);
}

TEST(StandaloneSUnion, SUnionWithCallbackError) {
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

  Parser<UnionParser> parser({"type", {{1, "bool"}, {2, "int"}}, unionCb});

  try {
    parser.parse(buf);
    FAIL() << "No exception thrown";
  } catch (ParsingError &e) {
    ASSERT_TRUE(parser.parser().isSet());

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

TEST(StandaloneSUnion, SUnionWithArgsStruct) {
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

  UnionParser::Args union_args({"type", {{1, "bool"}, {2, "int"}}});

  Parser<UnionParser> parser(union_args);

  ASSERT_NO_THROW(parser.parse(buf));
  ASSERT_NO_THROW(parser.finish());

  ASSERT_EQ(true, std::get<0>(std::get<0>(parser.parser().get())));

  buf = R"(
{
  "type": 2,
  "int": 100
})";

  ASSERT_NO_THROW(parser.parse(buf));
  ASSERT_NO_THROW(parser.finish());

  ASSERT_EQ(100, std::get<0>(std::get<1>(parser.parser().get())));
}

TEST(StandaloneSUnion, SUnionWithUnexpectedObject) {
  std::string buf(
      R"(
{
  "type": 1,
  "error": true
})");

  // clang-format off
  Parser<SUnion<
    int64_t,
    SObject<Value<bool>>,
    SObject<Value<int64_t>>
  >> parser({"type", {{1, "bool"}, {2, "int"}}});
  // clang-format on

  try {
    parser.parse(buf);
    FAIL() << "No exception thrown";
  } catch (ParsingError &e) {
    ASSERT_FALSE(parser.parser().isSet());

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

TEST(StandaloneSUnion, SUnionWithSCustomObject) {
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
  Parser<SUnion<
    int64_t,
    InnerObjectParser,
    SObject<Value<int64_t>>
  >> parser({
      "type", {
      {
        1, {{"bool", "string"}, innerObjectCb}
      }, {
        2, "int"
      }}});
  // clang-format on

  ASSERT_NO_THROW(parser.parse(buf));
  ASSERT_NO_THROW(parser.finish());

  {
    auto value = std::get<0>(parser.parser().get());

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

  ASSERT_EQ(100, std::get<0>(std::get<1>(parser.parser().get())));
}

TEST(StandaloneSUnion, SUnionWithEmbeddedSUnion) {
  std::string buf(
      R"(
{
  "type": 1,
  "subtype": 1,
  "bool": true
})");

  // clang-format off
  Parser<SUnion<
    int64_t,
    SUnion<
      int64_t,
      SObject<Value<bool>>,
      SObject<Value<int64_t>>
    >,
    SObject<Value<std::string>>
  >> parser({
      "type", {
        {
          1, {"subtype", {{1, "bool"}, {2, "int"}}}
        }, {
          2, "string"
        }}});
  // clang-format on

  ASSERT_NO_THROW(parser.parse(buf));
  ASSERT_NO_THROW(parser.finish());

  ASSERT_EQ(true, std::get<0>(std::get<0>(std::get<0>(parser.parser().get()))));

  buf = R"(
{
  "type": 1,
  "subtype": 2,
  "int": 100
})";

  ASSERT_NO_THROW(parser.parse(buf));
  ASSERT_NO_THROW(parser.finish());

  ASSERT_EQ(100, std::get<0>(std::get<1>(std::get<0>(parser.parser().get()))));

  buf = R"(
{
  "type": 2,
  "string": "value"
})";

  ASSERT_NO_THROW(parser.parse(buf));
  ASSERT_NO_THROW(parser.finish());

  ASSERT_EQ("value", std::get<0>(std::get<1>(parser.parser().get())));
}

template <typename Field1, typename Field2> struct MoveStruct {
  Field1 field1;
  Field2 field2;
  static bool copy_used;

  MoveStruct() {}

  MoveStruct(const MoveStruct<Field1, Field2> &other) {
    field1 = std::move(other.field1);
    field2 = std::move(other.field2);
    copy_used = true;
  }

  MoveStruct &operator=(const MoveStruct<Field1, Field2> &other) {
    field1 = std::move(other.field1);
    field2 = std::move(other.field2);
    copy_used = true;
    return *this;
  }

  MoveStruct(MoveStruct<Field1, Field2> &&other) {
    field1 = std::move(other.field1);
    field2 = std::move(other.field2);
  }
};

template <typename Field1, typename Field2>
bool MoveStruct<Field1, Field2>::copy_used = false;

TEST(StandaloneSUnion, Move) {
  std::string buf(
      R"({"type": 1, "bool": true, "integer": 10})");

  using ObjectParser1 =
      SObject<MoveStruct<bool, int64_t>, Value<bool>, Value<int64_t>>;

  using ObjectParser2 = SObject<MoveStruct<double, std::string>, Value<double>,
                                Value<std::string>>;

  auto objectCb1 = [&](ObjectParser1 &parser, auto &value) {
    value.field1 = parser.get<0>();
    value.field2 = parser.get<1>();
    return true;
  };

  auto objectCb2 = [&](ObjectParser2 &parser, auto &value) {
    value.field1 = parser.get<0>();
    value.field2 = parser.get<1>();
    return true;
  };

  // clang-format off
  Parser<SUnion<
    int64_t,
    ObjectParser1,
    ObjectParser2
  >> parser({{
      "type", {
        {1, {{"bool", "integer"}, objectCb1}},
        {2, {{"double", "string"}, objectCb2}}
      }}});
  // clang-format on

  ASSERT_NO_THROW(parser.parse(buf));
  ASSERT_NO_THROW(parser.finish());

  MoveStruct<bool, int64_t>::copy_used = false;

  {
    auto variant = parser.parser().pop();
    ASSERT_FALSE(parser.parser().isSet());

    ASSERT_FALSE((MoveStruct<bool, int64_t>::copy_used));

    ASSERT_EQ(0, variant.index());

    auto &object = std::get<0>(variant);

    ASSERT_EQ(true, object.field1);
    ASSERT_EQ(10, object.field2);
  }

  buf = R"({"type": 2, "double": 11.5, "string": "value"})";

  ASSERT_NO_THROW(parser.parse(buf));
  ASSERT_NO_THROW(parser.finish());

  MoveStruct<double, std::string>::copy_used = false;

  {
    auto variant = parser.parser().pop();
    ASSERT_FALSE(parser.parser().isSet());

    ASSERT_FALSE((MoveStruct<double, std::string>::copy_used));

    ASSERT_EQ(1, variant.index());

    auto &object = std::get<1>(variant);

    ASSERT_EQ(11.5, object.field1);
    ASSERT_EQ("value", object.field2);
  }
}

TEST(StandaloneSUnion, UnknownExceptionInValueSetter) {
  std::string buf(
      R"({"type": 1, "bool": true})");

  struct ObjectStruct {
    bool throw_on_assign = false;

    ObjectStruct() {}

    ObjectStruct(const ObjectStruct &) {}

    ObjectStruct &operator=(const ObjectStruct &other) {
      throw_on_assign = other.throw_on_assign;
      if (throw_on_assign) {
        throw 10;
      }
      return *this;
    }
  };

  using InnerObjectParser = SObject<ObjectStruct, Value<bool>>;

  auto innerObjectCb = [&](InnerObjectParser &, ObjectStruct &object) {
    object.throw_on_assign = true;
    return true;
  };

  // clang-format off
  Parser<SUnion<
    int64_t,
    InnerObjectParser,
    SObject<Value<int64_t>>
  >> parser({{
      "type", {
        {1, {{"bool"}, innerObjectCb}},
        {2, "int"}
      }}});
  // clang-format on

  try {
    parser.parse(buf);
    FAIL() << "No exception thrown";
  } catch (ParsingError &e) {
    ASSERT_FALSE(parser.parser().isSet());
    ASSERT_EQ("Can not set value: unknown exception", e.sjparserError());

    ASSERT_EQ(
        R"(parse error: client cancelled parse via callback return value
               {"type": 1, "bool": true}
                     (right here) ------^
)",
        e.parserError());
  } catch (...) {
    FAIL() << "Invalid exception thrown";
  }
}
