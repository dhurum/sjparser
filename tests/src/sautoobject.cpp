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

TEST(SAutoObject, Empty) {
  std::string buf(R"({})");

  Parser<SObject<Value<std::string>, Value<int64_t>>> parser(
      {"string", "integer"});

  try {
    parser.parse(buf);
    FAIL() << "No exception thrown";
  } catch (ParsingError &e) {
    ASSERT_FALSE(parser.parser().isSet());
    ASSERT_EQ(
        "Can not set value: Not all fields are set in an storage object "
        "without "
        "a default value",
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

TEST(SAutoObject, EmptyDefault) {
  std::string buf(R"({})");

  Parser<SObject<Value<std::string>, Value<int64_t>>> parser(
      {{"string", "integer"}, {"test", 1}});

  ASSERT_NO_THROW(parser.parse(buf));
  ASSERT_NO_THROW(parser.finish());

  ASSERT_TRUE(parser.parser().isSet());

  ASSERT_EQ("test", std::get<0>(parser.parser().get()));
  ASSERT_EQ(1, std::get<1>(parser.parser().get()));
}

TEST(SAutoObject, EmptyDefaultWithCallback) {
  std::string buf(R"({})");
  bool callback_called = false;

  auto objectCb = [&](const std::tuple<bool, std::string> &) {
    callback_called = true;
    return true;
  };

  Parser<SObject<Value<bool>, Value<std::string>>> parser(
      {{"bool", "string"}, {true, "test"}, objectCb});

  ASSERT_NO_THROW(parser.parse(buf));
  ASSERT_NO_THROW(parser.finish());

  ASSERT_TRUE(parser.parser().isSet());

  ASSERT_TRUE(callback_called);
}

TEST(SAutoObject, Null) {
  std::string buf(R"(null)");

  Parser<SObject<Value<bool>, Value<std::string>>> parser({"bool", "string"});

  ASSERT_NO_THROW(parser.parse(buf));
  ASSERT_NO_THROW(parser.finish());

  ASSERT_FALSE(parser.parser().isSet());
}

TEST(SAutoObject, Reset) {
  std::string buf(
      R"({"bool": true, "string": "value"})");

  // clang-format off
  Parser<SObject<
    Value<bool>,
    Value<std::string>
  >> parser({"bool", "string"});
  // clang-format on

  ASSERT_NO_THROW(parser.parse(buf));
  ASSERT_NO_THROW(parser.finish());

  ASSERT_EQ(true, std::get<0>(parser.parser().get()));
  ASSERT_EQ("value", std::get<1>(parser.parser().get()));

  buf = R"(null)";

  ASSERT_NO_THROW(parser.parse(buf));
  ASSERT_NO_THROW(parser.finish());

  ASSERT_FALSE(parser.parser().isSet());
}

TEST(SAutoObject, AllValuesFields) {
  std::string buf(
      R"({"bool": true, "integer": 10, "double": 11.5, "string": "value"})");

  // clang-format off
  Parser<SObject<
    Value<bool>,
    Value<int64_t>,
    Value<double>,
    Value<std::string>
  >> parser({"bool", "integer", "double", "string"});
  // clang-format on

  ASSERT_NO_THROW(parser.parse(buf));
  ASSERT_NO_THROW(parser.finish());

  ASSERT_EQ(true, std::get<0>(parser.parser().get()));
  ASSERT_EQ(10, std::get<1>(parser.parser().get()));
  ASSERT_EQ(11.5, std::get<2>(parser.parser().get()));
  ASSERT_EQ("value", std::get<3>(parser.parser().get()));
}

TEST(SAutoObject, Default) {
  std::string buf(
      R"({"bool": true, "double": 11.5})");

  // clang-format off
  Parser<SObject<
    Value<bool>,
    Value<int64_t>,
    Value<double>,
    Value<std::string>
  >> parser({{"bool", "integer", "double", "string"},
      {false, 10, 10.0, "value"}});
  // clang-format on

  ASSERT_NO_THROW(parser.parse(buf));
  ASSERT_NO_THROW(parser.finish());

  ASSERT_EQ(true, std::get<0>(parser.parser().get()));
  ASSERT_EQ(10, std::get<1>(parser.parser().get()));
  ASSERT_EQ(11.5, std::get<2>(parser.parser().get()));
  ASSERT_EQ("value", std::get<3>(parser.parser().get()));
}

TEST(SAutoObject, FieldsWithCallbacks) {
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

  Parser<SObject<Value<bool>, Value<std::string>>> parser(
      {{"bool", boolCb}, {"string", stringCb}});

  ASSERT_NO_THROW(parser.parse(buf));
  ASSERT_NO_THROW(parser.finish());

  ASSERT_EQ(true, std::get<0>(parser.parser().get()));
  ASSERT_EQ("value", std::get<1>(parser.parser().get()));

  ASSERT_EQ(true, bool_value);
  ASSERT_EQ("value", str_value);
}

TEST(SAutoObject, FieldsWithCallbackError) {
  std::string buf(
      R"({"bool": true, "string": "value"})");

  auto boolCb = [&](const bool &) { return false; };

  auto stringCb = [&](const std::string &) { return true; };

  Parser<SObject<Value<bool>, Value<std::string>>> parser(
      {{"bool", boolCb}, {"string", stringCb}});

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

TEST(SAutoObject, SAutoObjectWithCallback) {
  std::string buf(
      R"({"bool": true, "string": "value"})");

  using ValueType = std::tuple<bool, std::string>;
  ValueType value;

  auto objectCb = [&](const ValueType &_value) {
    value = _value;
    return true;
  };

  Parser<SObject<Value<bool>, Value<std::string>>> parser(
      {{"bool", "string"}, objectCb});

  ASSERT_NO_THROW(parser.parse(buf));
  ASSERT_NO_THROW(parser.finish());

  ASSERT_EQ(true, std::get<0>(value));
  ASSERT_EQ("value", std::get<1>(value));
}

TEST(SAutoObject, SAutoObjectWithCallbackError) {
  std::string buf(
      R"({"bool": true, "string": "value"})");

  auto objectCb = [&](const std::tuple<bool, std::string> &) { return false; };

  Parser<SObject<Value<bool>, Value<std::string>>> parser(
      {{"bool", "string"}, objectCb});

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

TEST(SAutoObject, OneField) {
  std::string buf(R"({"string": "value"})");

  Parser<SObject<Value<std::string>>> parser("string");

  ASSERT_NO_THROW(parser.parse(buf));
  ASSERT_NO_THROW(parser.finish());

  ASSERT_EQ("value", std::get<0>(parser.parser().get()));
}

TEST(SAutoObject, OneFieldEmpty) {
  std::string buf(R"({})");

  Parser<SObject<Value<std::string>>> parser("string");

  try {
    parser.parse(buf);
    FAIL() << "No exception thrown";
  } catch (ParsingError &e) {
    ASSERT_FALSE(parser.parser().isSet());
    ASSERT_EQ(
        "Can not set value: Not all fields are set in an storage object "
        "without "
        "a default value",
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

TEST(SAutoObject, OneFieldEmptyDefault) {
  std::string buf(R"({})");

  Parser<SObject<Value<std::string>>> parser({"string", "value"});

  ASSERT_NO_THROW(parser.parse(buf));
  ASSERT_NO_THROW(parser.finish());

  ASSERT_EQ("value", std::get<0>(parser.parser().get()));
}

TEST(SAutoObject, OneFieldWithFieldCallback) {
  std::string buf(R"({"string": "value"})");
  std::string value;

  auto elementCb = [&](const std::string &str) {
    value = str;
    return true;
  };

  Parser<SObject<Value<std::string>>> parser({{"string", elementCb}});

  ASSERT_NO_THROW(parser.parse(buf));
  ASSERT_NO_THROW(parser.finish());

  ASSERT_EQ("value", std::get<0>(parser.parser().get()));
  ASSERT_EQ("value", value);
}

TEST(SAutoObject, OneFieldWithObjectCallback) {
  std::string buf(R"({"string": "value"})");

  using ValueType = std::tuple<std::string>;
  ValueType value;

  auto objectCb = [&](const ValueType &_value) {
    value = _value;
    return true;
  };

  // {} around "string" are optional, but they make it a bit more clear
  Parser<SObject<Value<std::string>>> parser({{"string"}, objectCb});

  ASSERT_NO_THROW(parser.parse(buf));
  ASSERT_NO_THROW(parser.finish());

  ASSERT_EQ("value", std::get<0>(parser.parser().get()));
  ASSERT_EQ("value", std::get<0>(value));
}

TEST(SAutoObject, OneFieldWithElementAndObjectCallbacks) {
  std::string buf(R"({"string": "value"})");
  std::string value;

  using ValueType = std::tuple<std::string>;
  ValueType object_value;

  auto elementCb = [&](const std::string &str) {
    value = str;
    return true;
  };

  auto objectCb = [&](const ValueType &_value) {
    object_value = _value;
    return true;
  };

  Parser<SObject<Value<std::string>>> parser(
      {{{"string", elementCb}}, objectCb});

  ASSERT_NO_THROW(parser.parse(buf));
  ASSERT_NO_THROW(parser.finish());

  ASSERT_EQ("value", std::get<0>(parser.parser().get()));
  ASSERT_EQ("value", value);
  ASSERT_EQ("value", std::get<0>(object_value));
}

TEST(SAutoObject, SAutoObjectWithArgsStruct) {
  std::string buf(
      R"({"string": "value", "integer": 10})");

  using ObjectParser = SObject<Value<std::string>, Value<int64_t>>;

  ObjectParser::Args object_args({"string", "integer"});

  Parser<ObjectParser> parser(object_args);

  ASSERT_NO_THROW(parser.parse(buf));
  ASSERT_NO_THROW(parser.finish());

  ASSERT_EQ("value", std::get<0>(parser.parser().get()));
  ASSERT_EQ(10, std::get<1>(parser.parser().get()));
}

TEST(SAutoObject, SAutoObjectWithSAutoObject) {
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
  Parser<SObject<
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

  ASSERT_NO_THROW(parser.parse(buf));
  ASSERT_NO_THROW(parser.finish());

  ASSERT_EQ("value", std::get<0>(parser.parser().get()));
  ASSERT_EQ(10, std::get<1>(parser.parser().get()));
  ASSERT_EQ(1, std::get<0>(std::get<2>(parser.parser().get())));
  ASSERT_EQ("in_value", std::get<1>(std::get<2>(parser.parser().get())));
  ASSERT_EQ(true, std::get<3>(parser.parser().get()));
}

TEST(SAutoObject, SAutoObjectWithUnexpectedSAutoObject) {
  std::string buf(
      R"(
{
  "string": "value",
  "object": {
    "error": 1
  }
})");

  // clang-format off
  Parser<SObject<
    Value<std::string>,
    SObject<
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

  try {
    parser.parse(buf);
    FAIL() << "No exception thrown";
  } catch (ParsingError &e) {
    ASSERT_FALSE(parser.parser().isSet());
    ASSERT_EQ("Unexpected field error", e.sjparserError());

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

TEST(SAutoObject, SAutoObjectWithSAutoObjectWithCallback) {
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

  using ValueType = std::tuple<int64_t, std::string>;
  ValueType value;

  using InnerObjectParser = SObject<Value<int64_t>, Value<std::string>>;

  auto callback = [&](const ValueType &_value) {
    value = _value;
    return true;
  };

  // clang-format off
  Parser<SObject<
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

  ASSERT_NO_THROW(parser.parse(buf));
  ASSERT_NO_THROW(parser.finish());

  ASSERT_EQ("value", std::get<0>(parser.parser().get()));
  ASSERT_EQ(10, std::get<1>(parser.parser().get()));
  ASSERT_EQ(1, std::get<0>(std::get<2>(parser.parser().get())));
  ASSERT_EQ("in_value", std::get<1>(std::get<2>(parser.parser().get())));
  ASSERT_EQ(true, std::get<3>(parser.parser().get()));

  ASSERT_EQ(1, std::get<0>(value));
  ASSERT_EQ("in_value", std::get<1>(value));
}

TEST(SAutoObject, SAutoObjectOfSAutoObjects) {
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
  Parser<SObject<
    SObject<
      Value<std::string>,
      Value<int64_t>>,
    SObject<
      Value<int64_t>,
      Value<std::string>>,
    SObject<Value<bool>>
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

  ASSERT_NO_THROW(parser.parse(buf));
  ASSERT_NO_THROW(parser.finish());

  ASSERT_EQ("value", std::get<0>(std::get<0>(parser.parser().get())));
  ASSERT_EQ(10, std::get<1>(std::get<0>(parser.parser().get())));
  ASSERT_EQ(1, std::get<0>(std::get<1>(parser.parser().get())));
  ASSERT_EQ("value2", std::get<1>(std::get<1>(parser.parser().get())));
  ASSERT_EQ(true, std::get<0>(std::get<2>(parser.parser().get())));
}

TEST(SAutoObject, SAutoObjectWithSCustomObject) {
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
    value = {parser.pop<0>(), parser.pop<1>()};
    return true;
  };

  // clang-format off
  Parser<SObject<
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

  ASSERT_NO_THROW(parser.parse(buf));
  ASSERT_NO_THROW(parser.finish());

  ASSERT_EQ("value", std::get<0>(parser.parser().get()));
  ASSERT_EQ(10, std::get<1>(parser.parser().get()));
  ASSERT_EQ(1, std::get<2>(parser.parser().get()).int_field);
  ASSERT_EQ("in_value", std::get<2>(parser.parser().get()).str_field);
  ASSERT_EQ(true, std::get<3>(parser.parser().get()));
}

TEST(SAutoObject, SAutoObjectWithSArray) {
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
      SObject<Value<std::string>, Value<int64_t>, SArray<Value<std::string>>>;

  Parser<ParserType> parser({"string", "integer", "array"});

  ASSERT_NO_THROW(parser.parse(buf));
  ASSERT_NO_THROW(parser.finish());

  ASSERT_EQ("value", std::get<0>(parser.parser().get()));
  ASSERT_EQ(10, std::get<1>(parser.parser().get()));

  auto array = std::get<2>(parser.parser().get());
  ASSERT_EQ(3, array.size());
  ASSERT_EQ("elt1", array[0]);
  ASSERT_EQ("elt2", array[1]);
  ASSERT_EQ("elt3", array[2]);
}

TEST(SAutoObject, SAutoObjectWithDefaultSArray) {
  std::string buf(
      R"(
{
  "string": "value",
  "integer": 10
})");

  using ParserType =
      SObject<Value<std::string>, Value<int64_t>, SArray<Value<std::string>>>;

  Parser<ParserType> parser(
      {{"string", "integer", "array"}, {"default", 0, {"elt1", "elt2"}}});

  ASSERT_NO_THROW(parser.parse(buf));
  ASSERT_NO_THROW(parser.finish());

  ASSERT_EQ("value", std::get<0>(parser.parser().get()));
  ASSERT_EQ(10, std::get<1>(parser.parser().get()));

  auto array = std::get<2>(parser.parser().get());
  ASSERT_EQ(2, array.size());
  ASSERT_EQ("elt1", array[0]);
  ASSERT_EQ("elt2", array[1]);
}

TEST(SAutoObject, SAutoObjectWithStandaloneSUnion) {
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
  using ObjectParser = SObject<
    Value<int64_t>,
    SUnion<
      std::string,
      SObject<Value<bool>>,
      SObject<Value<int64_t>>
    >>;
  // clang-format on

  Parser<ObjectParser> parser(
      {{"id", {"data", {"type", {{"1", "bool"}, {"2", "int"}}}}}});

  ASSERT_NO_THROW(parser.parse(buf));
  ASSERT_NO_THROW(parser.finish());

  ASSERT_EQ(10, std::get<0>(parser.parser().get()));
  ASSERT_EQ(true, std::get<0>(std::get<0>(std::get<1>(parser.parser().get()))));

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

  ASSERT_EQ(10, std::get<0>(parser.parser().get()));
  ASSERT_EQ(100, std::get<0>(std::get<1>(std::get<1>(parser.parser().get()))));
}

TEST(SAutoObject, SAutoObjectWithEmbeddedSUnion) {
  std::string buf(
      R"(
{
  "id": 10,
  "type": "1",
  "bool": true
})");

  // clang-format off
  using ObjectParser = SObject<
    Value<int64_t>,
    SUnion<
      std::string,
      SObject<Value<bool>>,
      SObject<Value<int64_t>>
    >>;
  // clang-format on

  Parser<ObjectParser> parser(
      {{"id", {"type", {{"1", "bool"}, {"2", "int"}}}}});

  ASSERT_NO_THROW(parser.parse(buf));
  ASSERT_NO_THROW(parser.finish());

  ASSERT_EQ(10, std::get<0>(parser.parser().get()));
  ASSERT_EQ(true, std::get<0>(std::get<0>(std::get<1>(parser.parser().get()))));

  std::string buf2(
      R"(
{
  "id": 10,
  "type": "2",
  "int": 100
})");

  ASSERT_NO_THROW(parser.parse(buf2));
  ASSERT_NO_THROW(parser.finish());

  ASSERT_EQ(10, std::get<0>(parser.parser().get()));
  ASSERT_EQ(100, std::get<0>(std::get<1>(std::get<1>(parser.parser().get()))));
}

TEST(SAutoObject, Move) {
  std::string buf(
      R"(
{
  "object": {
    "integer": 1,
    "string": "in_value"
  }
})");
  static bool copy_used = false;

  struct ObjectStruct {
    int64_t int_field;
    std::string str_field;

    ObjectStruct() { int_field = 0; }

    ObjectStruct(const ObjectStruct &other) {
      int_field = other.int_field;
      str_field = other.str_field;
      copy_used = true;
    }

    ObjectStruct &operator=(const ObjectStruct &other) {
      int_field = other.int_field;
      str_field = other.str_field;
      copy_used = true;
      return *this;
    }

    ObjectStruct(ObjectStruct &&other) {
      int_field = std::move(other.int_field);
      str_field = std::move(other.str_field);
    }
  };

  using InnerObjectParser =
      SObject<ObjectStruct, Value<int64_t>, Value<std::string>>;

  auto innerObjectCb = [&](InnerObjectParser &parser, ObjectStruct &value) {
    value.int_field = parser.get<0>();
    value.str_field = parser.get<1>();
    return true;
  };

  // clang-format off
  Parser<SObject<InnerObjectParser>> parser({
      {
        "object", {
          {
            "integer",
            "string"
          }, innerObjectCb
        }
      }});
  // clang-format on

  ASSERT_NO_THROW(parser.parse(buf));
  ASSERT_NO_THROW(parser.finish());

  copy_used = false;

  auto value = parser.parser().pop();
  ASSERT_FALSE(parser.parser().isSet());

  ASSERT_FALSE(copy_used);
  ASSERT_EQ(1, std::get<0>(value).int_field);
  ASSERT_EQ("in_value", std::get<0>(value).str_field);

  buf = R"(
{
  "object": {
    "integer": 10,
    "string": "in_value2"
  }
})";

  ASSERT_NO_THROW(parser.parse(buf));
  ASSERT_NO_THROW(parser.finish());

  copy_used = false;

  auto value2 = parser.parser().pop();
  ASSERT_FALSE(parser.parser().isSet());

  ASSERT_FALSE(copy_used);
  ASSERT_EQ(10, std::get<0>(value2).int_field);
  ASSERT_EQ("in_value2", std::get<0>(value2).str_field);
}

TEST(SAutoObject, UnknownExceptionInValueSetter) {
  std::string buf(
      R"(
{
  "object": {
    "integer": 1,
    "string": "in_value"
  }
})");

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

  using InnerObjectParser =
      SObject<ObjectStruct, Value<int64_t>, Value<std::string>>;

  auto innerObjectCb = [&](InnerObjectParser &, ObjectStruct &object) {
    object.throw_on_assign = true;
    return true;
  };

  // clang-format off
  Parser<SObject<InnerObjectParser>> parser({
      {
        "object", {
          {
            "integer",
            "string"
          }, innerObjectCb
        }
      }});
  // clang-format on

  try {
    parser.parse(buf);
    FAIL() << "No exception thrown";
  } catch (ParsingError &e) {
    ASSERT_FALSE(parser.parser().isSet());
    ASSERT_EQ("Can not set value: unknown exception", e.sjparserError());

    ASSERT_EQ(
        R"(parse error: client cancelled parse via callback return value
              "string": "in_value"   } }
                     (right here) ------^
)",
        e.parserError());
  } catch (...) {
    FAIL() << "Invalid exception thrown";
  }
}
