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
#include <map>
#include "sjparser/sjparser.h"

using namespace SJParser;

TEST(Object, Empty) {
  std::string buf(R"({})");

  Parser parser{Object{std::tuple{Member{"bool", Value<bool>{}},
                                  Member{"string", Value<std::string>{}}}}};

  ASSERT_NO_THROW(parser.parse(buf));
  ASSERT_NO_THROW(parser.finish());

  ASSERT_TRUE(parser.parser().isSet());
  ASSERT_FALSE(parser.parser().parser<0>().isSet());
  ASSERT_FALSE(parser.parser().parser<1>().isSet());
}

TEST(Object, EmptyWithCallback) {
  std::string buf(R"({})");
  bool callback_called = false;

  using ObjectParser = Object<Value<bool>, Value<std::string>>;

  auto objectCb = [&](ObjectParser &) {
    callback_called = true;
    return true;
  };

  Parser parser{Object{std::tuple{Member{"bool", Value<bool>{}},
                                  Member{"string", Value<std::string>{}}},
                       objectCb}};

  ASSERT_NO_THROW(parser.parse(buf));
  ASSERT_NO_THROW(parser.finish());

  ASSERT_TRUE(parser.parser().isSet());
  ASSERT_FALSE(parser.parser().parser<0>().isSet());
  ASSERT_FALSE(parser.parser().parser<1>().isSet());

  ASSERT_TRUE(callback_called);
}

TEST(Object, Null) {
  std::string buf(R"(null)");

  Parser parser{Object{std::tuple{Member{"bool", Value<bool>{}},
                                  Member{"string", Value<std::string>{}}}}};

  ASSERT_NO_THROW(parser.parse(buf));
  ASSERT_NO_THROW(parser.finish());

  ASSERT_FALSE(parser.parser().isSet());
}

TEST(Object, Reset) {
  std::string buf(
      R"({"bool": true, "string": "value"})");

  Parser parser{Object{std::tuple{Member{"bool", Value<bool>{}},
                                  Member{"string", Value<std::string>{}}}}};

  ASSERT_NO_THROW(parser.parse(buf));
  ASSERT_NO_THROW(parser.finish());

  ASSERT_EQ(true, parser.parser().get<0>());
  ASSERT_EQ("value", parser.parser().get<1>());

  buf = R"(null)";

  ASSERT_NO_THROW(parser.parse(buf));
  ASSERT_NO_THROW(parser.finish());

  ASSERT_FALSE(parser.parser().isSet());
}

TEST(Object, UnexpectedMember) {
  std::string buf(R"({"error": true, "bool": true, "string": "value"})");

  Parser parser{Object{std::tuple{Member{"bool", Value<bool>{}},
                                  Member{"string", Value<std::string>{}}}}};

  try {
    parser.parse(buf);
    FAIL() << "No exception thrown";
  } catch (ParsingError &e) {
    ASSERT_FALSE(parser.parser().isSet());
    ASSERT_EQ("Unexpected member error", e.sjparserError());

    ASSERT_EQ(
        R"(parse error: client cancelled parse via callback return value
                                {"error": true, "bool": true, "string"
                     (right here) ------^
)",
        e.parserError());
  } catch (...) {
    FAIL() << "Invalid exception thrown";
  }
}

TEST(Object, IgnoredUnexpectedMember) {
  std::string buf(R"({"error": true, "bool": true, "string": "value"})");

  Parser parser{Object{std::tuple{Member{"bool", Value<bool>{}},
                                  Member{"string", Value<std::string>{}}},
                       ObjectOptions{Reaction::Ignore}}};

  parser.parse(buf);
  ASSERT_NO_THROW(parser.finish());

  ASSERT_EQ(true, parser.parser().get<0>());
  ASSERT_EQ("value", parser.parser().get<1>());
}

TEST(Object, MembersWithCallbackError) {
  std::string buf(
      R"({"bool": true, "string": "value"})");

  auto boolCb = [&](const bool &) { return false; };

  auto stringCb = [&](const std::string &) { return true; };

  Parser parser{
      Object{std::tuple{Member{"bool", Value<bool>{boolCb}},
                        Member{"string", Value<std::string>{stringCb}}}}};

  try {
    parser.parse(buf);
    FAIL() << "No exception thrown";
  } catch (ParsingError &e) {
    ASSERT_FALSE(parser.parser().isSet());

    ASSERT_EQ("Callback returned false", e.sjparserError());
    ASSERT_EQ(
        R"(parse error: client cancelled parse via callback return value
                           {"bool": true, "string": "value"}
                     (right here) ------^
)",
        e.parserError());
  } catch (...) {
    FAIL() << "Invalid exception thrown";
  }
}

TEST(Object, ObjectWithCallback) {
  std::string buf(
      R"({"bool": true, "string": "value"})");
  bool bool_value = false;
  std::string str_value;

  using ObjectParser = Object<Value<bool>, Value<std::string>>;

  auto objectCb = [&](ObjectParser &parser) {
    bool_value = parser.get<0>();
    str_value = parser.get<1>();
    return true;
  };

  Parser parser{Object{std::tuple{Member{"bool", Value<bool>{}},
                                  Member{"string", Value<std::string>{}}},
                       objectCb}};

  ASSERT_NO_THROW(parser.parse(buf));
  ASSERT_NO_THROW(parser.finish());

  ASSERT_EQ(true, parser.parser().get<0>());
  ASSERT_EQ("value", parser.parser().get<1>());
  ASSERT_EQ(true, bool_value);
  ASSERT_EQ("value", str_value);
}

TEST(Object, ObjectWithOptionsAndCallback) {
  std::string buf(
      R"({"error": true, "bool": true, "string": "value"})");
  bool bool_value = false;
  std::string str_value;

  using ObjectParser = Object<Value<bool>, Value<std::string>>;

  auto objectCb = [&](ObjectParser &parser) {
    bool_value = parser.get<0>();
    str_value = parser.get<1>();
    return true;
  };

  Parser parser{Object{std::tuple{Member{"bool", Value<bool>{}},
                                  Member{"string", Value<std::string>{}}},
                       ObjectOptions{Reaction::Ignore}, objectCb}};

  ASSERT_NO_THROW(parser.parse(buf));
  ASSERT_NO_THROW(parser.finish());

  ASSERT_EQ(true, parser.parser().get<0>());
  ASSERT_EQ("value", parser.parser().get<1>());
  ASSERT_EQ(true, bool_value);
  ASSERT_EQ("value", str_value);
}

TEST(Object, ObjectWithCallbackError) {
  std::string buf(
      R"({"bool": true, "string": "value"})");

  using ObjectParser = Object<Value<bool>, Value<std::string>>;

  auto objectCb = [&](ObjectParser &) { return false; };

  Parser parser{Object{std::tuple{Member{"bool", Value<bool>{}},
                                  Member{"string", Value<std::string>{}}},
                       objectCb}};

  try {
    parser.parse(buf);
    FAIL() << "No exception thrown";
  } catch (ParsingError &e) {
    ASSERT_TRUE(parser.parser().isSet());

    ASSERT_EQ("Callback returned false", e.sjparserError());
    ASSERT_EQ(
        R"(parse error: client cancelled parse via callback return value
          ool": true, "string": "value"}
                     (right here) ------^
)",
        e.parserError());
  } catch (...) {
    FAIL() << "Invalid exception thrown";
  }
}

TEST(Object, StdStringiMemberNames) {
  std::string buf(R"({"string": "value", "integer": 10})");

  std::string string_name = "string";
  std::string integer_name = "integer";

  Parser parser{Object{std::tuple{Member{string_name, Value<std::string>{}},
                                  Member{integer_name, Value<int64_t>{}}}}};

  ASSERT_NO_THROW(parser.parse(buf));
  ASSERT_NO_THROW(parser.finish());

  ASSERT_EQ("value", parser.parser().get<0>());
  ASSERT_EQ(10, parser.parser().get<1>());
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

  Parser parser{Object{std::tuple{
      Member{"string", Value<std::string>{}},
      Member{"object",
             Object{std::tuple{Member{"integer", Value<int64_t>{}}}}}}}};

  try {
    parser.parse(buf);
    FAIL() << "No exception thrown";
  } catch (ParsingError &e) {
    ASSERT_FALSE(parser.parser().isSet());
    ASSERT_EQ("Unexpected member error", e.sjparserError());

    ASSERT_EQ(
        R"(parse error: client cancelled parse via callback return value
          ue",   "object": {     "error": 1   } }
                     (right here) ------^
)",
        e.parserError());
  } catch (...) {
    FAIL() << "Invalid exception thrown";
  }
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

  Parser parser{Object{std::tuple{
      Member{"string", Value<std::string>{}},
      Member{"integer", Value<int64_t>{}},
      Member{"object",
             Object{std::tuple{Member{"integer", Value<int64_t>{}},
                               Member{"string", Value<std::string>{}}}}},
      Member{"boolean", Value<bool>{}}}}};

  ASSERT_NO_THROW(parser.parse(buf));
  ASSERT_NO_THROW(parser.finish());

  ASSERT_EQ("value", parser.parser().get<0>());
  ASSERT_EQ(10, parser.parser().get<1>());
  ASSERT_EQ(1, parser.parser().get<2>().get<0>());
  ASSERT_EQ("in_value", parser.parser().get<2>().get<1>());
  ASSERT_EQ(true, parser.parser().get<3>());
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
    int64_t int_member;
    std::string str_member;
  };

  Parser parser{Object{std::tuple{
      Member{"string", Value<std::string>{}},
      Member{"integer", Value<int64_t>{}},
      Member{"object",
             SCustomObject{TypeHolder<ObjectStruct>{},
                           std::tuple{Member{"integer", Value<int64_t>{}},
                                      Member{"string", Value<std::string>{}}}}},
      Member{"boolean", Value<bool>{}}}}};

  auto innerObjectCb = [&](decltype(parser)::ParserType::ParserType<2> &parser,
                           ObjectStruct &value) {
    value = {parser.pop<0>(), parser.pop<1>()};
    return true;
  };

  parser.parser().parser<2>().setFinishCallback(innerObjectCb);

  ASSERT_NO_THROW(parser.parse(buf));
  ASSERT_NO_THROW(parser.finish());

  ASSERT_EQ("value", parser.parser().get<0>());
  ASSERT_EQ(10, parser.parser().get<1>());
  ASSERT_EQ(1, parser.parser().get<2>().int_member);
  ASSERT_EQ("in_value", parser.parser().get<2>().str_member);
  ASSERT_EQ(true, parser.parser().get<3>());
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

  Parser parser{Object{std::tuple{
      Member{"string", Value<std::string>{}},
      Member{"integer", Value<int64_t>{}},
      Member{"object",
             SAutoObject{std::tuple{Member{"integer", Value<int64_t>{}},
                                    Member{"string", Value<std::string>{}}}}},
      Member{"boolean", Value<bool>{}}}}};

  ASSERT_NO_THROW(parser.parse(buf));
  ASSERT_NO_THROW(parser.finish());

  ASSERT_EQ("value", parser.parser().get<0>());
  ASSERT_EQ(10, parser.parser().get<1>());

  auto inner_value = parser.parser().get<2>();
  ASSERT_EQ(1, std::get<0>(inner_value));
  ASSERT_EQ("in_value", std::get<1>(inner_value));

  ASSERT_EQ(true, parser.parser().get<3>());
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

  Parser parser{Object{std::tuple{
      Member{"id", Value<int64_t>{}},
      Member{"data",
             Union{TypeHolder<std::string>{}, "type",
                   std::tuple{Member{"1", Object{std::tuple{
                                              Member{"bool", Value<bool>{}}}}},
                              Member{"2", Object{std::tuple{Member{
                                              "int", Value<int64_t>{}}}}}}}}}}};

  ASSERT_NO_THROW(parser.parse(buf));
  ASSERT_NO_THROW(parser.finish());

  ASSERT_TRUE(parser.parser().get<1>().parser<0>().isSet());
  ASSERT_FALSE(parser.parser().get<1>().parser<1>().isSet());
  ASSERT_EQ(0, parser.parser().get<1>().currentMemberId());

  ASSERT_EQ(10, parser.parser().get<0>());
  ASSERT_EQ(true, parser.parser().get<1>().get<0>().get<0>());

  std::string buf2(
      R"(
{
  "id": 10,
  "data": {
    "type": "2",
    "int": 100
  }
})");

  ASSERT_NO_THROW(parser.parse(buf2));
  ASSERT_NO_THROW(parser.finish());

  ASSERT_FALSE(parser.parser().get<1>().parser<0>().isSet());
  ASSERT_TRUE(parser.parser().get<1>().parser<1>().isSet());
  ASSERT_EQ(1, parser.parser().get<1>().currentMemberId());

  ASSERT_EQ(10, parser.parser().get<0>());
  ASSERT_EQ(100, parser.parser().get<1>().get<1>().get<0>());
}

TEST(Object, ObjectWithStandaloneSUnion) {
  std::string buf(
      R"(
{
  "id": 10,
  "data": {
    "type": "1",
    "bool": true
  }
})");

  Parser parser{Object{std::tuple{
      Member{"id", Value<int64_t>{}},
      Member{
          "data",
          SUnion{TypeHolder<std::string>{}, "type",
                 std::tuple{Member{"1", SAutoObject{std::tuple{
                                            Member{"bool", Value<bool>{}}}}},
                            Member{"2", SAutoObject{std::tuple{Member{
                                            "int", Value<int64_t>{}}}}}}}}}}};

  ASSERT_NO_THROW(parser.parse(buf));
  ASSERT_NO_THROW(parser.finish());

  ASSERT_EQ(10, parser.parser().get<0>());

  {
    auto variant = parser.parser().get<1>();

    ASSERT_EQ(0, variant.index());

    ASSERT_EQ(true, std::get<0>(std::get<0>(variant)));
  }

  std::string buf2(
      R"(
{
  "id": 10,
  "data": {
    "type": "2",
    "int": 100
  }
})");

  ASSERT_NO_THROW(parser.parse(buf2));
  ASSERT_NO_THROW(parser.finish());

  ASSERT_EQ(10, parser.parser().get<0>());

  {
    auto variant = parser.parser().get<1>();

    ASSERT_EQ(1, variant.index());

    ASSERT_EQ(100, std::get<0>(std::get<1>(variant)));
  }
}

TEST(Object, ObjectWithEmbeddedUnion) {
  std::string buf(
      R"(
{
  "id": 10,
  "type": "1",
  "bool": true
})");

  Parser parser{Object{std::tuple{
      Member{"id", Value<int64_t>{}},
      Member{"type",
             Union{TypeHolder<std::string>{},
                   std::tuple{Member{"1", Object{std::tuple{
                                              Member{"bool", Value<bool>{}}}}},
                              Member{"2", Object{std::tuple{Member{
                                              "int", Value<int64_t>{}}}}}}}}}}};

  ASSERT_NO_THROW(parser.parse(buf));
  ASSERT_NO_THROW(parser.finish());

  ASSERT_TRUE(parser.parser().get<1>().parser<0>().isSet());
  ASSERT_FALSE(parser.parser().get<1>().parser<1>().isSet());
  ASSERT_EQ(0, parser.parser().get<1>().currentMemberId());

  ASSERT_EQ(10, parser.parser().get<0>());
  ASSERT_EQ(true, parser.parser().get<1>().get<0>().get<0>());

  std::string buf2(
      R"(
{
  "id": 10,
  "type": "2",
  "int": 100
})");

  ASSERT_NO_THROW(parser.parse(buf2));
  ASSERT_NO_THROW(parser.finish());

  ASSERT_FALSE(parser.parser().get<1>().parser<0>().isSet());
  ASSERT_TRUE(parser.parser().get<1>().parser<1>().isSet());
  ASSERT_EQ(1, parser.parser().get<1>().currentMemberId());

  ASSERT_EQ(10, parser.parser().get<0>());
  ASSERT_EQ(100, parser.parser().get<1>().get<1>().get<0>());
}

TEST(Object, ObjectWithEmbeddedSUnion) {
  std::string buf(
      R"(
{
  "id": 10,
  "type": "1",
  "bool": true
})");

  Parser parser{Object{std::tuple{
      Member{"id", Value<int64_t>{}},
      Member{
          "type",
          SUnion{TypeHolder<std::string>{},
                 std::tuple{Member{"1", SAutoObject{std::tuple{
                                            Member{"bool", Value<bool>{}}}}},
                            Member{"2", SAutoObject{std::tuple{Member{
                                            "int", Value<int64_t>{}}}}}}}}}}};

  ASSERT_NO_THROW(parser.parse(buf));
  ASSERT_NO_THROW(parser.finish());

  ASSERT_EQ(10, parser.parser().get<0>());

  {
    auto variant = parser.parser().get<1>();

    ASSERT_EQ(0, variant.index());

    ASSERT_EQ(true, std::get<0>(std::get<0>(variant)));
  }

  std::string buf2(
      R"(
{
  "id": 10,
  "type": "2",
  "int": 100
})");

  ASSERT_NO_THROW(parser.parse(buf2));
  ASSERT_NO_THROW(parser.finish());

  ASSERT_EQ(10, parser.parser().get<0>());

  {
    auto variant = parser.parser().get<1>();

    ASSERT_EQ(1, variant.index());

    ASSERT_EQ(100, std::get<0>(std::get<1>(variant)));
  }
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

  auto arrayElementCb = [&](const std::string &value) {
    array_value.push_back(value);
    return true;
  };

  Parser parser{Object{
      std::tuple{Member{"string", Value<std::string>{}},
                 Member{"integer", Value<int64_t>{}},
                 Member{"array", Array{Value<std::string>{arrayElementCb}}}}}};

  ASSERT_NO_THROW(parser.parse(buf));
  ASSERT_NO_THROW(parser.finish());

  ASSERT_EQ("value", parser.parser().get<0>());
  ASSERT_EQ(10, parser.parser().get<1>());
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

  Parser parser{
      Object{std::tuple{Member{"string", Value<std::string>{}},
                        Member{"integer", Value<int64_t>{}},
                        Member{"array", SArray{Value<std::string>{}}}}}};

  ASSERT_NO_THROW(parser.parse(buf));
  ASSERT_NO_THROW(parser.finish());

  ASSERT_EQ("value", parser.parser().get<0>());
  ASSERT_EQ(10, parser.parser().get<1>());
  ASSERT_EQ(3, parser.parser().get<2>().size());
  ASSERT_EQ("elt1", parser.parser().get<2>()[0]);
  ASSERT_EQ("elt2", parser.parser().get<2>()[1]);
  ASSERT_EQ("elt3", parser.parser().get<2>()[2]);
}

TEST(Object, ObjectWithMap) {
  std::string buf(
      R"(
{
  "string": "value",
  "integer": 10,
  "map": {
    "1": 10,
    "2": 20
  }
})");

  std::map<std::string, int64_t> values;

  auto mapElementCb = [&](const std::string &key, Value<int64_t> &parser) {
    values[key] = parser.get();
    return true;
  };

  Parser parser{
      Object{std::tuple{Member{"string", Value<std::string>{}},
                        Member{"integer", Value<int64_t>{}},
                        Member{"map", Map{Value<int64_t>{}, mapElementCb}}}}};

  ASSERT_NO_THROW(parser.parse(buf));
  ASSERT_NO_THROW(parser.finish());

  ASSERT_EQ("value", parser.parser().get<0>());
  ASSERT_EQ(10, parser.parser().get<1>());
  ASSERT_EQ(2, values.size());
  ASSERT_EQ(10, values["1"]);
  ASSERT_EQ(20, values["2"]);
}

TEST(Object, Move) {
  std::string buf(
      R"(
{
  "object": {
    "integer": 1,
    "string": "in_value"
  }
})");

  struct ObjectStruct {
    int64_t int_member;
    std::string str_member;

    ObjectStruct() { int_member = 0; }

    ObjectStruct(ObjectStruct &&other) {
      int_member = std::move(other.int_member);
      str_member = std::move(other.str_member);
    }

    // Needed for parser internals
    ObjectStruct &operator=(ObjectStruct &&other) {
      int_member = std::move(other.int_member);
      str_member = std::move(other.str_member);
      return *this;
    }
  };

  Parser parser{Object{std::tuple{Member{
      "object",
      SCustomObject{TypeHolder<ObjectStruct>{},
                    std::tuple{Member{"integer", Value<int64_t>{}},
                               Member{"string", Value<std::string>{}}}}}}}};

  auto innerObjectCb = [&](decltype(parser)::ParserType::ParserType<0> &parser,
                           ObjectStruct &value) {
    value.int_member = parser.get<0>();
    value.str_member = parser.get<1>();
    return true;
  };

  parser.parser().parser<0>().setFinishCallback(innerObjectCb);

  ASSERT_NO_THROW(parser.parse(buf));
  ASSERT_NO_THROW(parser.finish());

  auto value = parser.parser().pop<0>();
  ASSERT_FALSE(parser.parser().parser<0>().isSet());

  ASSERT_EQ(1, value.int_member);
  ASSERT_EQ("in_value", value.str_member);

  buf = R"(
{
  "object": {
    "integer": 10,
    "string": "in_value2"
  }
})";

  ASSERT_NO_THROW(parser.parse(buf));
  ASSERT_NO_THROW(parser.finish());

  auto value2 = parser.parser().pop<0>();
  ASSERT_FALSE(parser.parser().parser<0>().isSet());

  ASSERT_EQ(10, value2.int_member);
  ASSERT_EQ("in_value2", value2.str_member);
}

TEST(Object, RepeatingMembers) {
  try {
    Parser parser{Object{std::tuple{Member{"member", Value<bool>{}},
                                    Member{"member", Value<std::string>{}}}}};

    FAIL() << "No exception thrown";
  } catch (std::runtime_error &e) {
    ASSERT_STREQ("Member member appears more, than once", e.what());
  } catch (...) {
    FAIL() << "Invalid exception thrown";
  }
}

TEST(Object, ObjectWithParserReference) {
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

  SArray sarray{Value<std::string>{}};

  Parser parser{Object{std::tuple{Member{"string", Value<std::string>{}},
                                  Member{"integer", Value<int64_t>{}},
                                  Member{"array", sarray}}}};

  ASSERT_NO_THROW(parser.parse(buf));
  ASSERT_NO_THROW(parser.finish());

  ASSERT_EQ("value", parser.parser().get<0>());
  ASSERT_EQ(10, parser.parser().get<1>());
  ASSERT_EQ(3, parser.parser().get<2>().size());
  ASSERT_EQ("elt1", parser.parser().get<2>()[0]);
  ASSERT_EQ("elt2", parser.parser().get<2>()[1]);
  ASSERT_EQ("elt3", parser.parser().get<2>()[2]);

  ASSERT_EQ(&(parser.parser().parser<2>()), &sarray);
}
