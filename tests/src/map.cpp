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

TEST(Map, Empty) {
  std::string buf(R"({})");

  Parser<Map<Value<bool>>> parser;

  ASSERT_NO_THROW(parser.parse(buf));
  ASSERT_NO_THROW(parser.finish());

  ASSERT_TRUE(parser.parser().isSet());
}

TEST(Map, EmptyWithCallbacks) {
  std::string buf(R"({})");
  bool key_callback_called = false;
  bool finish_callback_called = false;

  using MapParser = Map<Value<bool>>;

  auto keyCb = [&](const std::string &, MapParser::ParserType &) {
    key_callback_called = true;
    return true;
  };

  auto finishCb = [&](MapParser &) {
    finish_callback_called = true;
    return true;
  };

  Parser<MapParser> parser({keyCb, finishCb});

  ASSERT_NO_THROW(parser.parse(buf));
  ASSERT_NO_THROW(parser.finish());

  ASSERT_TRUE(parser.parser().isSet());

  ASSERT_FALSE(key_callback_called);
  ASSERT_TRUE(finish_callback_called);
}

TEST(Map, Null) {
  std::string buf(R"(null)");

  Parser<Map<Value<bool>>> parser;

  ASSERT_NO_THROW(parser.parse(buf));
  ASSERT_NO_THROW(parser.finish());

  ASSERT_FALSE(parser.parser().isSet());
}

TEST(Map, Reset) {
  std::string buf(
      R"({"1": true})");

  bool value = false;

  using MapParser = Map<Value<bool>>;

  auto keyCb = [&](const std::string &, MapParser::ParserType &parser) {
    value = parser.get();
    return true;
  };

  Parser<MapParser> parser({keyCb});

  ASSERT_NO_THROW(parser.parse(buf));
  ASSERT_NO_THROW(parser.finish());

  ASSERT_TRUE(value);

  buf = R"(null)";

  ASSERT_NO_THROW(parser.parse(buf));
  ASSERT_NO_THROW(parser.finish());

  ASSERT_FALSE(parser.parser().isSet());
}

TEST(Map, SeveralKeys) {
  std::string buf(
      R"({"1": 10, "2": 15})");

  std::map<std::string, int64_t> values;

  using MapParser = Map<Value<int64_t>>;

  auto keyCb = [&](const std::string &key, MapParser::ParserType &parser) {
    values[key] = parser.get();
    return true;
  };

  Parser<MapParser> parser({keyCb});

  ASSERT_NO_THROW(parser.parse(buf));
  ASSERT_NO_THROW(parser.finish());

  ASSERT_EQ(10, values["1"]);
  ASSERT_EQ(15, values["2"]);
}

TEST(Map, WithInternalCallback) {
  std::string buf(
      R"({"1": 10})");

  int64_t value;

  using MapParser = Map<Value<int64_t>>;

  auto internalCb = [&](const int64_t &val) {
    value = val;
    return true;
  };

  Parser<MapParser> parser({internalCb});

  ASSERT_NO_THROW(parser.parse(buf));
  ASSERT_NO_THROW(parser.finish());

  ASSERT_EQ(10, value);
}

TEST(Map, WithInternalAndFinishCallback) {
  std::string buf(
      R"({"1": 10})");

  bool internal_callback_called = false;
  bool finish_callback_called = false;

  using MapParser = Map<Value<int64_t>>;

  auto internalCb = [&](const int64_t &) {
    internal_callback_called = true;
    return true;
  };

  auto finishCb = [&](MapParser &) {
    finish_callback_called = true;
    return true;
  };

  Parser<MapParser> parser({internalCb, finishCb});

  ASSERT_NO_THROW(parser.parse(buf));
  ASSERT_NO_THROW(parser.finish());

  ASSERT_TRUE(internal_callback_called);
  ASSERT_TRUE(finish_callback_called);
}

TEST(Map, WithAllCallbacks) {
  std::string buf(
      R"({"1": 10})");

  bool internal_callback_called = false;
  bool key_callback_called = false;
  bool finish_callback_called = false;

  using MapParser = Map<Value<int64_t>>;

  auto internalCb = [&](const int64_t &) {
    internal_callback_called = true;
    return true;
  };

  auto keyCb = [&](const std::string &, MapParser::ParserType &) {
    key_callback_called = true;
    return true;
  };

  auto finishCb = [&](MapParser &) {
    finish_callback_called = true;
    return true;
  };

  Parser<MapParser> parser({internalCb, keyCb, finishCb});

  ASSERT_NO_THROW(parser.parse(buf));
  ASSERT_NO_THROW(parser.finish());

  ASSERT_TRUE(internal_callback_called);
  ASSERT_TRUE(key_callback_called);
  ASSERT_TRUE(finish_callback_called);
}

TEST(Map, WithInternalCallbackError) {
  std::string buf(
      R"({"1": 10})");

  using MapParser = Map<Value<int64_t>>;

  auto internalCb = [&](const int64_t &) { return false; };

  Parser<MapParser> parser({internalCb});

  try {
    parser.parse(buf);
    FAIL() << "No exception thrown";
  } catch (ParsingError &e) {
    ASSERT_FALSE(parser.parser().isSet());

    ASSERT_EQ("Callback returned false", e.sjparserError());
    ASSERT_EQ(
        R"(parse error: client cancelled parse via callback return value
                                {"1": 10}
                     (right here) ------^
)",
        e.parserError());
  } catch (...) {
    FAIL() << "Invalid exception thrown";
  }
}

TEST(Map, WithKeyCallbackError) {
  std::string buf(
      R"({"1": 10})");

  using MapParser = Map<Value<int64_t>>;

  auto keyCb = [&](const std::string &, MapParser::ParserType &) {
    return false;
  };

  Parser<MapParser> parser({keyCb});

  try {
    parser.parse(buf);
    FAIL() << "No exception thrown";
  } catch (ParsingError &e) {
    ASSERT_FALSE(parser.parser().isSet());

    ASSERT_EQ("Key callback returned false", e.sjparserError());
    ASSERT_EQ(
        R"(parse error: client cancelled parse via callback return value
                                {"1": 10}
                     (right here) ------^
)",
        e.parserError());
  } catch (...) {
    FAIL() << "Invalid exception thrown";
  }
}

TEST(Map, WithFinishCallbackError) {
  std::string buf(
      R"({"1": 10})");

  using MapParser = Map<Value<int64_t>>;

  auto finishCb = [&](MapParser &) { return false; };

  Parser<MapParser> parser({finishCb});

  try {
    parser.parse(buf);
    FAIL() << "No exception thrown";
  } catch (ParsingError &e) {
    ASSERT_TRUE(parser.parser().isSet());

    ASSERT_EQ("Callback returned false", e.sjparserError());
    ASSERT_EQ(
        R"(parse error: client cancelled parse via callback return value
                               {"1": 10}
                     (right here) ------^
)",
        e.parserError());
  } catch (...) {
    FAIL() << "Invalid exception thrown";
  }
}

TEST(Map, WithArgsStruct) {
  std::string buf(
      R"({"1": 10, "2": 15})");

  std::map<std::string, int64_t> values;

  using MapParser = Map<Value<int64_t>>;

  auto keyCb = [&](const std::string &key, MapParser::ParserType &parser) {
    values[key] = parser.get();
    return true;
  };

  MapParser::Args map_args = {keyCb};

  Parser<MapParser> parser(map_args);

  ASSERT_NO_THROW(parser.parse(buf));
  ASSERT_NO_THROW(parser.finish());

  ASSERT_EQ(10, values["1"]);
  ASSERT_EQ(15, values["2"]);
}

TEST(Map, MapOfObjects) {
  std::string buf(
      R"({
  "1": {
    "string": "value",
    "int": 10
  },
  "2": {
    "string": "value2",
    "int": 20
  }
})");

  struct ObjectStruct {
    std::string string_field;
    int64_t int_field;
  };

  std::map<std::string, ObjectStruct> values;

  using ObjectParser = Object<Value<std::string>, Value<int64_t>>;

  auto keyCb = [&](const std::string &key, ObjectParser &parser) {
    values[key] = {parser.get<0>(), parser.get<1>()};
    return true;
  };

  Parser<Map<ObjectParser>> parser({{"string", "int"}, keyCb});

  ASSERT_NO_THROW(parser.parse(buf));
  ASSERT_NO_THROW(parser.finish());

  ASSERT_EQ(2, values.size());
  ASSERT_EQ("value", values["1"].string_field);
  ASSERT_EQ(10, values["1"].int_field);
  ASSERT_EQ("value2", values["2"].string_field);
  ASSERT_EQ(20, values["2"].int_field);
}

TEST(Map, MapOfObjectsWithFinishCallback) {
  std::string buf(
      R"({
  "1": {
    "string": "value",
    "int": 10
  },
  "2": {
    "string": "value2",
    "int": 20
  }
})");

  bool finish_callback_called = false;

  using MapParser = Map<Object<Value<std::string>, Value<int64_t>>>;

  auto finishCb = [&](MapParser &) {
    finish_callback_called = true;
    return true;
  };

  std::vector<std::string> str_values;
  std::vector<int64_t> int_values;

  auto stringCb = [&](const std::string &value) {
    str_values.push_back(value);
    return true;
  };

  auto integerCb = [&](const int64_t &value) {
    int_values.push_back(value);
    return true;
  };

  Parser<MapParser> parser(
      {{{"string", stringCb}, {"int", integerCb}}, finishCb});

  ASSERT_NO_THROW(parser.parse(buf));
  ASSERT_NO_THROW(parser.finish());

  ASSERT_TRUE(finish_callback_called);
  ASSERT_EQ(2, str_values.size());
  ASSERT_EQ("value", str_values[0]);
  ASSERT_EQ("value2", str_values[1]);
  ASSERT_EQ(2, int_values.size());
  ASSERT_EQ(10, int_values[0]);
  ASSERT_EQ(20, int_values[1]);
}

TEST(Map, MapOfSCustomObjects) {
  std::string buf(
      R"({
  "1": {
    "string": "value",
    "int": 10
  },
  "2": {
    "string": "value2",
    "int": 20
  }
})");

  struct ObjectStruct {
    std::string string_field;
    int64_t int_field;
  };

  std::map<std::string, ObjectStruct> values;

  using ObjectParser =
      SObject<ObjectStruct, Value<std::string>, Value<int64_t>>;

  auto objectCb = [&](ObjectParser &parser, ObjectStruct &value) {
    value = {parser.pop<0>(), parser.pop<1>()};
    return true;
  };

  auto keyCb = [&](const std::string &key, ObjectParser &parser) {
    values[key] = parser.pop();
    return true;
  };

  Parser<Map<ObjectParser>> parser({{{"string", "int"}, objectCb}, keyCb});

  ASSERT_NO_THROW(parser.parse(buf));
  ASSERT_NO_THROW(parser.finish());

  ASSERT_EQ(2, values.size());
  ASSERT_EQ("value", values["1"].string_field);
  ASSERT_EQ(10, values["1"].int_field);
  ASSERT_EQ("value2", values["2"].string_field);
  ASSERT_EQ(20, values["2"].int_field);
}

TEST(Map, MapOfAutoObjects) {
  std::string buf(
      R"({
  "1": {
    "string": "value",
    "int": 10
  },
  "2": {
    "string": "value2",
    "int": 20
  }
})");

  using ObjectParser = SObject<Value<std::string>, Value<int64_t>>;

  std::map<std::string, ObjectParser::Type> values;

  auto keyCb = [&](const std::string &key, ObjectParser &parser) {
    values[key] = parser.pop();
    return true;
  };

  Parser<Map<ObjectParser>> parser({{"string", "int"}, keyCb});

  ASSERT_NO_THROW(parser.parse(buf));
  ASSERT_NO_THROW(parser.finish());

  ASSERT_EQ(2, values.size());
  ASSERT_EQ("value", std::get<0>(values["1"]));
  ASSERT_EQ(10, std::get<1>(values["1"]));
  ASSERT_EQ("value2", std::get<0>(values["2"]));
  ASSERT_EQ(20, std::get<1>(values["2"]));
}

TEST(Map, MapOfArrays) {
  std::string buf(
      R"({
  "1": [10, 20],
  "2": [30, 40]
})");

  std::map<std::string, std::vector<int64_t>> values;
  std::vector<int64_t> tmp_values;

  auto elementCb = [&](const int64_t &value) {
    tmp_values.push_back(value);
    return true;
  };

  using ArrayParser = Array<Value<int64_t>>;

  auto keyCb = [&](const std::string &key, ArrayParser &) {
    values[key] = tmp_values;
    tmp_values.clear();
    return true;
  };

  Parser<Map<ArrayParser>> parser({elementCb, keyCb});

  ASSERT_NO_THROW(parser.parse(buf));
  ASSERT_NO_THROW(parser.finish());

  ASSERT_EQ(2, values.size());
  ASSERT_EQ(10, values["1"][0]);
  ASSERT_EQ(20, values["1"][1]);
  ASSERT_EQ(30, values["2"][0]);
  ASSERT_EQ(40, values["2"][1]);
}

TEST(Map, MapOfSArrays) {
  std::string buf(
      R"({
  "1": [10, 20],
  "2": [30, 40]
})");

  std::map<std::string, std::vector<int64_t>> values;

  using ArrayParser = SArray<Value<int64_t>>;

  auto keyCb = [&](const std::string &key, ArrayParser &parser) {
    values[key] = parser.pop();
    return true;
  };

  Parser<Map<ArrayParser>> parser({keyCb});

  ASSERT_NO_THROW(parser.parse(buf));
  ASSERT_NO_THROW(parser.finish());

  ASSERT_EQ(2, values.size());
  ASSERT_EQ(10, values["1"][0]);
  ASSERT_EQ(20, values["1"][1]);
  ASSERT_EQ(30, values["2"][0]);
  ASSERT_EQ(40, values["2"][1]);
}

TEST(Map, MapOfStandaloneUnions) {
  std::string buf(
      R"({
  "1": {
    "type": "str",
    "string": "value"
  },
  "2": {
    "type": "int",
    "int": 20
  }
})");

  std::string str_value = "";
  int64_t int_value = 0;
  std::map<std::string, size_t> union_types;

  using UnionParser =
      Union<std::string, Object<Value<std::string>>, Object<Value<int64_t>>>;

  auto keyCb = [&](const std::string &key, UnionParser &parser) {
    union_types[key] = parser.currentMemberId();

    switch (parser.currentMemberId()) {
      case 0:
        str_value = parser.get<0>().get<0>();
        break;
      case 1:
        int_value = parser.get<1>().get<0>();
        break;
      default:
        return false;
    }
    return true;
  };

  Parser<Map<UnionParser>> parser(
      {{"type", {{"str", "string"}, {"int", "int"}}}, keyCb});

  ASSERT_NO_THROW(parser.parse(buf));
  ASSERT_NO_THROW(parser.finish());

  ASSERT_EQ(2, union_types.size());
  ASSERT_EQ(0, union_types["1"]);
  ASSERT_EQ(1, union_types["2"]);
  ASSERT_EQ("value", str_value);
  ASSERT_EQ(20, int_value);
}

TEST(Map, MapOfStandaloneSUnions) {
  std::string buf(
      R"({
  "1": {
    "type": "str",
    "string": "value"
  },
  "2": {
    "type": "int",
    "int": 20
  }
})");

  using UnionParser =
      SUnion<std::string, SObject<Value<std::string>>, SObject<Value<int64_t>>>;

  std::map<std::string, UnionParser::Type> values;

  auto keyCb = [&](const std::string &key, UnionParser &parser) {
    values[key] = parser.pop();
    return true;
  };

  Parser<Map<UnionParser>> parser(
      {{"type", {{"str", "string"}, {"int", "int"}}}, keyCb});

  ASSERT_NO_THROW(parser.parse(buf));
  ASSERT_NO_THROW(parser.finish());

  ASSERT_EQ(2, values.size());
  ASSERT_EQ("value", std::get<0>(std::get<0>(values["1"])));
  ASSERT_EQ(20, std::get<0>(std::get<1>(values["2"])));
}

TEST(Map, MapOfEmbeddedUnions) {
  std::string buf(
      R"({
  "1": {
    "id": 1,
    "type": "str",
    "string": "value"
  },
  "2": {
    "id": 2,
    "type": "int",
    "int": 20
  }
})");

  std::vector<int64_t> ids;
  std::string str_value = "";
  int64_t int_value = 0;
  std::map<std::string, size_t> union_types;

  // clang-format off
  using ObjectParser =
      Object<
        Value<int64_t>,
        Union<
          std::string,
          Object<Value<std::string>>,
          Object<Value<int64_t>>
      >>;
  // clang-format on

  auto keyCb = [&](const std::string &key, ObjectParser &parser) {
    ids.push_back(parser.get<0>());
    union_types[key] = parser.get<1>().currentMemberId();

    switch (parser.get<1>().currentMemberId()) {
      case 0:
        str_value = parser.get<1>().get<0>().get<0>();
        break;
      case 1:
        int_value = parser.get<1>().get<1>().get<0>();
        break;
      default:
        return false;
    }
    return true;
  };

  Parser<Map<ObjectParser>> parser(
      {{"id", {"type", {{"str", "string"}, {"int", "int"}}}}, keyCb});

  ASSERT_NO_THROW(parser.parse(buf));
  ASSERT_NO_THROW(parser.finish());

  ASSERT_EQ(2, ids.size());
  ASSERT_EQ(1, ids[0]);
  ASSERT_EQ(2, ids[1]);

  ASSERT_EQ(2, union_types.size());
  ASSERT_EQ(0, union_types["1"]);
  ASSERT_EQ(1, union_types["2"]);

  ASSERT_EQ("value", str_value);
  ASSERT_EQ(20, int_value);
}

TEST(Map, MapOfEmbeddedSUnions) {
  std::string buf(
      R"({
  "1": {
    "id": 1,
    "type": "str",
    "string": "value"
  },
  "2": {
    "id": 2,
    "type": "int",
    "int": 20
  }
})");

  // clang-format off
  using ObjectParser =
      SObject<
        Value<int64_t>,
        SUnion<
          std::string,
          SObject<Value<std::string>>,
          SObject<Value<int64_t>>
      >>;
  // clang-format on

  std::map<std::string, ObjectParser::Type> values;

  auto keyCb = [&](const std::string &key, ObjectParser &parser) {
    values[key] = parser.pop();
    return true;
  };

  Parser<Map<ObjectParser>> parser(
      {{"id", {"type", {{"str", "string"}, {"int", "int"}}}}, keyCb});

  ASSERT_NO_THROW(parser.parse(buf));
  ASSERT_NO_THROW(parser.finish());

  ASSERT_EQ(2, values.size());
  ASSERT_EQ(1, std::get<0>(values["1"]));
  ASSERT_EQ("value", std::get<0>(std::get<0>(std::get<1>(values["1"]))));
  ASSERT_EQ(2, std::get<0>(values["2"]));
  ASSERT_EQ(20, std::get<0>(std::get<1>(std::get<1>(values["2"]))));
}

TEST(Map, MapOfMaps) {
  std::string buf(
      R"({
  "1": {
    "1": [10, 20],
    "2": [30, 40]
  },
  "2": {
    "1": [11, 21],
    "2": [31, 41]
  }
})");

  std::map<std::string, std::map<std::string, std::vector<int64_t>>> values;
  std::map<std::string, std::vector<int64_t>> tmp_values;

  using ArrayParser = SArray<Value<int64_t>>;

  auto innerKeyCb = [&](const std::string &key, ArrayParser &parser) {
    tmp_values[key] = parser.pop();
    return true;
  };

  using InnerMapParser = Map<ArrayParser>;

  auto keyCb = [&](const std::string &key, InnerMapParser &) {
    values[key] = tmp_values;
    tmp_values.clear();
    return true;
  };

  Parser<Map<InnerMapParser>> parser({{innerKeyCb}, keyCb});

  ASSERT_NO_THROW(parser.parse(buf));
  ASSERT_NO_THROW(parser.finish());

  ASSERT_EQ(2, values["1"].size());
  ASSERT_EQ(10, values["1"]["1"][0]);
  ASSERT_EQ(20, values["1"]["1"][1]);
  ASSERT_EQ(30, values["1"]["2"][0]);
  ASSERT_EQ(40, values["1"]["2"][1]);

  ASSERT_EQ(2, values["2"].size());
  ASSERT_EQ(11, values["2"]["1"][0]);
  ASSERT_EQ(21, values["2"]["1"][1]);
  ASSERT_EQ(31, values["2"]["2"][0]);
  ASSERT_EQ(41, values["2"]["2"][1]);
}
