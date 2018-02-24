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

TEST(SCustomObject, Empty) {
  std::string buf(R"({})");

  struct ObjectStruct {};

  auto objectCb = [&](auto &, ObjectStruct &) { return true; };

  Parser parser{SCustomObject{TypeHolder<ObjectStruct>{},
                              std::tuple{Member{"string", Value<std::string>{}},
                                         Member{"integer", Value<int64_t>{}}},
                              objectCb}};

  ASSERT_NO_THROW(parser.parse(buf));
  ASSERT_NO_THROW(parser.finish());

  ASSERT_FALSE(parser.parser().isSet());
  ASSERT_TRUE(parser.parser().isEmpty());
}

TEST(SCustomObject, Null) {
  std::string buf(R"(null)");

  struct ObjectStruct {};

  auto objectCb = [&](auto &, ObjectStruct &) { return true; };

  Parser parser{SCustomObject{TypeHolder<ObjectStruct>{},
                              std::tuple{Member{"string", Value<std::string>{}},
                                         Member{"integer", Value<int64_t>{}}},
                              objectCb}};

  ASSERT_NO_THROW(parser.parse(buf));
  ASSERT_NO_THROW(parser.finish());

  ASSERT_FALSE(parser.parser().isSet());
  ASSERT_TRUE(parser.parser().isEmpty());
}

TEST(SCustomObject, Reset) {
  std::string buf(
      R"({"bool": true, "string": "value"})");

  struct ObjectStruct {
    bool bool_value;
    std::string str_value;
  };

  auto objectCb = [&](auto &parser, ObjectStruct &value) {
    value.bool_value = parser.template get<0>();
    value.str_value = parser.template get<1>();
    return true;
  };

  Parser parser{
      SCustomObject{TypeHolder<ObjectStruct>{},
                    std::tuple{Member{"bool", Value<bool>{}},
                               Member{"string", Value<std::string>{}}},
                    objectCb}};

  ASSERT_NO_THROW(parser.parse(buf));
  ASSERT_NO_THROW(parser.finish());

  ASSERT_EQ(true, parser.parser().get().bool_value);
  ASSERT_EQ("value", parser.parser().get().str_value);

  buf = R"(null)";

  ASSERT_NO_THROW(parser.parse(buf));
  ASSERT_NO_THROW(parser.finish());

  ASSERT_FALSE(parser.parser().isSet());
}

TEST(SCustomObject, UnexpectedMember) {
  std::string buf(R"({"error": true, "bool": true, "string": "value"})");

  struct ObjectStruct {
    bool bool_value;
    std::string str_value;
  };

  Parser parser{
      SCustomObject{TypeHolder<ObjectStruct>{},
                    std::tuple{Member{"bool", Value<bool>{}},
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

TEST(SCustomObject, IgnoredUnexpectedMember) {
  std::string buf(R"({"error": true, "bool": true, "string": "value"})");

  struct ObjectStruct {
    bool bool_value;
    std::string str_value;
  };

  Parser parser{
      SCustomObject{TypeHolder<ObjectStruct>{},
                    std::tuple{Member{"bool", Value<bool>{}},
                               Member{"string", Value<std::string>{}}},
                    {Reaction::Ignore}}};

  auto objectCb = [&](decltype(parser)::ParserType &parser,
                      ObjectStruct &value) {
    value.bool_value = parser.get<0>();
    value.str_value = parser.get<1>();
    return true;
  };

  parser.parser().setFinishCallback(objectCb);

  ASSERT_NO_THROW(parser.parse(buf));
  ASSERT_NO_THROW(parser.finish());

  ASSERT_EQ(true, parser.parser().get().bool_value);
  ASSERT_EQ("value", parser.parser().get().str_value);
}

TEST(SCustomObject, MembersWithCallbackError) {
  std::string buf(
      R"({"bool": true, "string": "value"})");

  auto boolCb = [&](const bool &) { return false; };

  auto stringCb = [&](const std::string &) { return true; };

  struct ObjectStruct {
    bool bool_value;
    std::string str_value;
  };

  Parser parser{SCustomObject{
      TypeHolder<ObjectStruct>{},
      std::tuple{Member{"bool", Value<bool>{boolCb}},
                 Member{"string", Value<std::string>{stringCb}}}}};

  try {
    parser.parse(buf);
    FAIL() << "No exception thrown";
  } catch (ParsingError &e) {
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

TEST(SCustomObject, SCustomObjectWithCallbackError) {
  std::string buf(
      R"({"bool": true, "string": "value"})");

  struct ObjectStruct {
    bool bool_value;
    std::string str_value;
  };

  Parser parser{
      SCustomObject{TypeHolder<ObjectStruct>{},
                    std::tuple{Member{"bool", Value<bool>{}},
                               Member{"string", Value<std::string>{}}}}};

  auto objectCb = [&](decltype(parser)::ParserType &, ObjectStruct &) {
    return false;
  };

  parser.parser().setFinishCallback(objectCb);

  try {
    parser.parse(buf);
    FAIL() << "No exception thrown";
  } catch (ParsingError &e) {
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

TEST(SCustomObject, PopValue) {
  std::string buf(
      R"({"string": "value", "integer": 10})");

  struct ObjectStruct {
    std::string str_value;
    int64_t int_value;
  };

  Parser parser{SCustomObject{TypeHolder<ObjectStruct>{},
                              std::tuple{Member{"string", Value<std::string>{}},
                                         Member{"integer", Value<int64_t>{}}}}};

  auto objectCb = [&](decltype(parser)::ParserType &parser,
                      ObjectStruct &value) {
    value.str_value = parser.get<0>();
    value.int_value = parser.get<1>();
    return true;
  };

  parser.parser().setFinishCallback(objectCb);

  ASSERT_NO_THROW(parser.parse(buf));
  ASSERT_NO_THROW(parser.finish());

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

  Parser parser{SCustomObject{
      TypeHolder<ObjectStruct>{},
      std::tuple{Member{"string", Value<std::string>{}},
                 Member{"integer", Value<int64_t>{}},
                 Member{"object", Object{std::tuple{
                                      Member{"integer", Value<int64_t>{}},
                                      Member{"string", Value<std::string>{}}}}},
                 Member{"boolean", Value<bool>{}}}}};

  auto objectCb = [&](decltype(parser)::ParserType &parser,
                      ObjectStruct &value) {
    value.str_value = parser.get<0>();
    value.int_value = parser.get<1>();
    value.inner_int_value = parser.get<2>().get<0>();
    value.inner_str_value = parser.get<2>().get<1>();
    value.bool_value = parser.get<3>();
    return true;
  };

  parser.parser().setFinishCallback(objectCb);

  ASSERT_NO_THROW(parser.parse(buf));
  ASSERT_NO_THROW(parser.finish());

  ASSERT_EQ("value", parser.parser().get().str_value);
  ASSERT_EQ(10, parser.parser().get().int_value);
  ASSERT_EQ(1, parser.parser().get().inner_int_value);
  ASSERT_EQ("in_value", parser.parser().get().inner_str_value);
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

  struct ObjectStruct {
    std::string str_value;
    int64_t int_value;
    InnerObjectStruct inner_value;
    bool bool_value;
  };

  Parser parser{SCustomObject{
      TypeHolder<ObjectStruct>{},
      std::tuple{
          Member{"string", Value<std::string>{}},
          Member{"integer", Value<int64_t>{}},
          Member{"object",
                 SCustomObject{
                     TypeHolder<InnerObjectStruct>{},
                     std::tuple{Member{"integer", Value<int64_t>{}},
                                Member{"string", Value<std::string>{}}}}},
          Member{"boolean", Value<bool>{}}}}};

  auto innerCb = [&](decltype(parser)::ParserType::ParserType<2> &parser,
                     InnerObjectStruct &value) {
    value.int_value = parser.get<0>();
    value.str_value = parser.get<1>();
    return true;
  };

  parser.parser().parser<2>().setFinishCallback(innerCb);

  auto objectCb = [&](decltype(parser)::ParserType &parser,
                      ObjectStruct &value) {
    value.str_value = parser.get<0>();
    value.int_value = parser.get<1>();
    value.inner_value = parser.get<2>();
    value.bool_value = parser.get<3>();
    return true;
  };

  parser.parser().setFinishCallback(objectCb);

  ASSERT_NO_THROW(parser.parse(buf));
  ASSERT_NO_THROW(parser.finish());

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

  Parser parser{SCustomObject{
      TypeHolder<ObjectStruct>{},
      std::tuple{Member{"string", Value<std::string>{}},
                 Member{"integer", Value<int64_t>{}},
                 Member{"object", SAutoObject{std::tuple{
                                      Member{"integer", Value<int64_t>{}},
                                      Member{"string", Value<std::string>{}}}}},
                 Member{"boolean", Value<bool>{}}}}};

  auto objectCb = [&](decltype(parser)::ParserType &parser,
                      ObjectStruct &value) {
    value.str_value = parser.get<0>();
    value.int_value = parser.get<1>();
    value.inner_value = parser.get<2>();
    value.bool_value = parser.get<3>();
    return true;
  };

  parser.parser().setFinishCallback(objectCb);

  ASSERT_NO_THROW(parser.parse(buf));
  ASSERT_NO_THROW(parser.finish());

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

  Parser parser{SCustomObject{
      TypeHolder<ObjectStruct>{},
      std::tuple{
          Member{"id", Value<int64_t>{}},
          Member{"data",
                 Union{TypeHolder<std::string>{}, "type",
                       std::tuple{
                           Member{"1", Object{std::tuple{
                                           Member{"bool", Value<bool>{}}}}},
                           Member{"2", Object{std::tuple{Member{
                                           "int", Value<int64_t>{}}}}}}}}}}};

  auto objectCb = [&](decltype(parser)::ParserType &parser,
                      ObjectStruct &value) {
    value.int_value = parser.get<0>();
    value.inner_bool = (parser.get<1>().currentMemberId() == 0) ? true : false;
    if (value.inner_bool) {
      value.inner_value.bool_value = parser.get<1>().get<0>().get<0>();
    } else {
      value.inner_value.int_value = parser.get<1>().get<1>().get<0>();
    }
    return true;
  };

  parser.parser().setFinishCallback(objectCb);

  ASSERT_NO_THROW(parser.parse(buf));
  ASSERT_NO_THROW(parser.finish());

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

  ASSERT_NO_THROW(parser.parse(buf2));
  ASSERT_NO_THROW(parser.finish());

  ASSERT_FALSE(parser.parser().get().inner_bool);

  ASSERT_EQ(10, parser.parser().get().int_value);
  ASSERT_EQ(100, parser.parser().get().inner_value.int_value);
}

TEST(SCustomObject, SCustomObjectWithStandaloneSUnion) {
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
    std::variant<bool, int64_t> inner_value;
  };

  Parser parser{SCustomObject{
      TypeHolder<ObjectStruct>{},
      std::tuple{
          Member{"id", Value<int64_t>{}},
          Member{"data",
                 SUnion{TypeHolder<std::string>{}, "type",
                        std::tuple{
                            Member{"1", SAutoObject{std::tuple{
                                            Member{"bool", Value<bool>{}}}}},
                            Member{"2", SAutoObject{std::tuple{Member{
                                            "int", Value<int64_t>{}}}}}}}}}}};

  auto objectCb = [&](decltype(parser)::ParserType &parser,
                      ObjectStruct &value) {
    value.int_value = parser.get<0>();

    auto variant = parser.get<1>();

    if (variant.index() == 0) {
      value.inner_value = std::get<0>(std::get<0>(variant));
    } else {
      value.inner_value = std::get<0>(std::get<1>(variant));
    }
    return true;
  };

  parser.parser().setFinishCallback(objectCb);

  ASSERT_NO_THROW(parser.parse(buf));
  ASSERT_NO_THROW(parser.finish());

  ASSERT_EQ(10, parser.parser().get().int_value);
  ASSERT_EQ(true, std::get<0>(parser.parser().get().inner_value));

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

  ASSERT_EQ(10, parser.parser().get().int_value);
  ASSERT_EQ(100, std::get<1>(parser.parser().get().inner_value));
}

TEST(SCustomObject, SCustomObjectWithEmbeddedUnion) {
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

  Parser parser{SCustomObject{
      TypeHolder<ObjectStruct>{},
      std::tuple{
          Member{"id", Value<int64_t>{}},
          Member{"type",
                 Union{TypeHolder<std::string>{},
                       std::tuple{
                           Member{"1", Object{std::tuple{
                                           Member{"bool", Value<bool>{}}}}},
                           Member{"2", Object{std::tuple{Member{
                                           "int", Value<int64_t>{}}}}}}}}}}};

  auto objectCb = [&](decltype(parser)::ParserType &parser,
                      ObjectStruct &value) {
    value.int_value = parser.get<0>();
    value.inner_bool = (parser.get<1>().currentMemberId() == 0) ? true : false;
    if (value.inner_bool) {
      value.inner_value.bool_value = parser.get<1>().get<0>().get<0>();
    } else {
      value.inner_value.int_value = parser.get<1>().get<1>().get<0>();
    }
    return true;
  };

  parser.parser().setFinishCallback(objectCb);

  ASSERT_NO_THROW(parser.parse(buf));
  ASSERT_NO_THROW(parser.finish());

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

  ASSERT_NO_THROW(parser.parse(buf2));
  ASSERT_NO_THROW(parser.finish());

  ASSERT_FALSE(parser.parser().get().inner_bool);

  ASSERT_EQ(10, parser.parser().get().int_value);
  ASSERT_EQ(100, parser.parser().get().inner_value.int_value);
}

TEST(SCustomObject, SCustomObjectWithEmbeddedSUnion) {
  std::string buf(
      R"(
{
  "id": 10,
  "type": "1",
  "bool": true
})");

  struct ObjectStruct {
    int64_t int_value;
    std::variant<bool, int64_t> inner_value;
  };

  Parser parser{SCustomObject{
      TypeHolder<ObjectStruct>{},
      std::tuple{
          Member{"id", Value<int64_t>{}},
          Member{"type",
                 SUnion{TypeHolder<std::string>{},
                        std::tuple{
                            Member{"1", SAutoObject{std::tuple{
                                            Member{"bool", Value<bool>{}}}}},
                            Member{"2", SAutoObject{std::tuple{Member{
                                            "int", Value<int64_t>{}}}}}}}}}}};

  auto objectCb = [&](decltype(parser)::ParserType &parser,
                      ObjectStruct &value) {
    value.int_value = parser.get<0>();

    auto variant = parser.get<1>();

    if (variant.index() == 0) {
      value.inner_value = std::get<0>(std::get<0>(variant));
    } else {
      value.inner_value = std::get<0>(std::get<1>(variant));
    }
    return true;
  };

  parser.parser().setFinishCallback(objectCb);

  ASSERT_NO_THROW(parser.parse(buf));
  ASSERT_NO_THROW(parser.finish());

  ASSERT_EQ(10, parser.parser().get().int_value);
  ASSERT_EQ(true, std::get<0>(parser.parser().get().inner_value));

  std::string buf2(
      R"(
{
  "id": 10,
  "type": "2",
  "int": 100
})");

  ASSERT_NO_THROW(parser.parse(buf2));
  ASSERT_NO_THROW(parser.finish());

  ASSERT_EQ(10, parser.parser().get().int_value);
  ASSERT_EQ(100, std::get<1>(parser.parser().get().inner_value));
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

  auto arrayElementCb = [&](const std::string &value) {
    tmp_array.push_back(value);
    return true;
  };

  struct ObjectStruct {
    std::string str_value;
    int64_t int_value;
    std::vector<std::string> array_value;
  };

  Parser parser{SCustomObject{
      TypeHolder<ObjectStruct>{},
      std::tuple{Member{"string", Value<std::string>{}},
                 Member{"integer", Value<int64_t>{}},
                 Member{"array", Array{Value<std::string>{arrayElementCb}}}}}};

  auto objectCb = [&](decltype(parser)::ParserType &parser,
                      ObjectStruct &value) {
    value.str_value = parser.get<0>();
    value.int_value = parser.get<1>();
    value.array_value = tmp_array;
    return true;
  };

  parser.parser().setFinishCallback(objectCb);

  ASSERT_NO_THROW(parser.parse(buf));
  ASSERT_NO_THROW(parser.finish());

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

  Parser parser{
      SCustomObject{TypeHolder<ObjectStruct>{},
                    std::tuple{Member{"string", Value<std::string>{}},
                               Member{"integer", Value<int64_t>{}},
                               Member{"array", SArray{Value<std::string>{}}}}}};

  auto objectCb = [&](decltype(parser)::ParserType &parser,
                      ObjectStruct &value) {
    value.str_value = parser.get<0>();
    value.int_value = parser.get<1>();
    value.array_value = parser.get<2>();
    return true;
  };

  parser.parser().setFinishCallback(objectCb);

  ASSERT_NO_THROW(parser.parse(buf));
  ASSERT_NO_THROW(parser.finish());

  ASSERT_EQ("value", parser.parser().get().str_value);
  ASSERT_EQ(10, parser.parser().get().int_value);
  ASSERT_EQ(3, parser.parser().get().array_value.size());
  ASSERT_EQ("elt1", parser.parser().get().array_value[0]);
  ASSERT_EQ("elt2", parser.parser().get().array_value[1]);
  ASSERT_EQ("elt3", parser.parser().get().array_value[2]);
}

TEST(SCustomObject, Move) {
  std::string buf(
      R"(
{
  "integer": 1,
  "string": "in_value"
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

  Parser parser{
      SCustomObject{TypeHolder<ObjectStruct>{},
                    std::tuple{Member{"integer", Value<int64_t>{}},
                               Member{"string", Value<std::string>{}}}}};

  auto objectCb = [&](decltype(parser)::ParserType &parser,
                      ObjectStruct &value) {
    value.int_member = parser.get<0>();
    value.str_member = parser.get<1>();
    return true;
  };

  parser.parser().setFinishCallback(objectCb);

  ASSERT_NO_THROW(parser.parse(buf));
  ASSERT_NO_THROW(parser.finish());

  auto value = parser.parser().pop();
  ASSERT_FALSE(parser.parser().isSet());

  ASSERT_EQ(1, value.int_member);
  ASSERT_EQ("in_value", value.str_member);

  buf = R"(
{
  "integer": 10,
  "string": "in_value2"
})";

  ASSERT_NO_THROW(parser.parse(buf));
  ASSERT_NO_THROW(parser.finish());

  auto value2 = parser.parser().pop();
  ASSERT_FALSE(parser.parser().isSet());

  ASSERT_EQ(10, value2.int_member);
  ASSERT_EQ("in_value2", value2.str_member);
}

TEST(SCustomObject, SCustomObjectWithParserReference) {
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

  SArray sarray{Value<std::string>{}};

  Parser parser{SCustomObject{TypeHolder<ObjectStruct>{},
                              std::tuple{Member{"string", Value<std::string>{}},
                                         Member{"integer", Value<int64_t>{}},
                                         Member{"array", sarray}}}};

  auto objectCb = [&](decltype(parser)::ParserType &parser,
                      ObjectStruct &value) {
    value.str_value = parser.get<0>();
    value.int_value = parser.get<1>();
    value.array_value = parser.get<2>();
    return true;
  };

  parser.parser().setFinishCallback(objectCb);

  ASSERT_NO_THROW(parser.parse(buf));
  ASSERT_NO_THROW(parser.finish());

  ASSERT_EQ("value", parser.parser().get().str_value);
  ASSERT_EQ(10, parser.parser().get().int_value);
  ASSERT_EQ(3, parser.parser().get().array_value.size());
  ASSERT_EQ("elt1", parser.parser().get().array_value[0]);
  ASSERT_EQ("elt2", parser.parser().get().array_value[1]);
  ASSERT_EQ("elt3", parser.parser().get().array_value[2]);

  ASSERT_EQ(&(parser.parser().parser<2>()), &sarray);
}

TEST(SCustomObject, MissingMember) {
  std::string buf(R"({"bool": true})");

  struct ObjectStruct {
    bool bool_value;
    std::string str_value;
  };

  Parser parser{
      SCustomObject{TypeHolder<ObjectStruct>{},
                    std::tuple{Member{"bool", Value<bool>{}},
                               Member{"string", Value<std::string>{}}}}};

  try {
    parser.parse(buf);
    FAIL() << "No exception thrown";
  } catch (ParsingError &e) {
    ASSERT_FALSE(parser.parser().isSet());
    ASSERT_EQ("Mandatory member string is not present", e.sjparserError());

    ASSERT_EQ(
        R"(parse error: client cancelled parse via callback return value
                          {"bool": true}
                     (right here) ------^
)",
        e.parserError());
  } catch (...) {
    FAIL() << "Invalid exception thrown";
  }
}

TEST(SCustomObject, OptionalMember) {
  std::string buf(R"({"bool": true})");

  struct ObjectStruct {
    bool bool_value;
    std::string str_value;
  };

  Parser parser{SCustomObject{
      TypeHolder<ObjectStruct>{},
      std::tuple{Member{"bool", Value<bool>{}},
                 Member{"string", Value<std::string>{}, Presence::Optional}}}};

  parser.parse(buf);
  ASSERT_NO_THROW(parser.finish());

  ASSERT_EQ(true, parser.parser().get<0>());
  ASSERT_FALSE(parser.parser().parser<1>().isSet());
}

TEST(SCustomObject, OptionalMemberWithDefaultValue) {
  std::string buf(R"({"bool": true})");

  struct ObjectStruct {
    bool bool_value;
    std::string str_value;
  };

  Parser parser{SCustomObject{TypeHolder<ObjectStruct>{},
                              std::tuple{Member{"bool", Value<bool>{}},
                                         Member{"string", Value<std::string>{},
                                                Presence::Optional, "value"}}}};

  ASSERT_NO_THROW(parser.parse(buf));
  ASSERT_NO_THROW(parser.finish());

  ASSERT_EQ(true, parser.parser().get<0>());
  ASSERT_FALSE(parser.parser().parser<1>().isSet());
  ASSERT_EQ("value", parser.parser().get<1>());
}
