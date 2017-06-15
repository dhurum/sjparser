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

TEST(Array, Empty) {
  std::string buf(R"([])");
  std::vector<bool> values;

  auto elementCb = [&](const bool &value) {
    values.push_back(value);
    return true;
  };

  Parser<Array<Value<bool>>> parser(elementCb);

  ASSERT_TRUE(parser.parse(buf));
  ASSERT_TRUE(parser.finish());

  ASSERT_EQ(0, values.size());

  ASSERT_TRUE(parser.parser().isSet());
}

TEST(Array, Null) {
  std::string buf(R"(null)");
  std::vector<bool> values;

  auto elementCb = [&](const bool &value) {
    values.push_back(value);
    return true;
  };

  Parser<Array<Value<bool>>> parser(elementCb);

  ASSERT_TRUE(parser.parse(buf));
  ASSERT_TRUE(parser.finish());

  ASSERT_EQ(0, values.size());

  ASSERT_FALSE(parser.parser().isSet());
}

TEST(Array, ArrayOfBooleans) {
  std::string buf(R"([true, false])");
  std::vector<bool> values;

  auto elementCb = [&](const bool &value) {
    values.push_back(value);
    return true;
  };

  Parser<Array<Value<bool>>> parser(elementCb);

  ASSERT_TRUE(parser.parse(buf));
  ASSERT_TRUE(parser.finish());

  ASSERT_EQ(2, values.size());
  ASSERT_EQ(true, values[0]);
  ASSERT_EQ(false, values[1]);

  ASSERT_TRUE(parser.parser().isSet());
}

TEST(Array, ArrayOfIntegers) {
  std::string buf(R"([10, 11])");
  std::vector<int64_t> values;

  auto elementCb = [&](const int64_t &value) {
    values.push_back(value);
    return true;
  };

  Parser<Array<Value<int64_t>>> parser(elementCb);

  ASSERT_TRUE(parser.parse(buf));
  ASSERT_TRUE(parser.finish());

  ASSERT_EQ(2, values.size());
  ASSERT_EQ(10, values[0]);
  ASSERT_EQ(11, values[1]);

  ASSERT_TRUE(parser.parser().isSet());
}

TEST(Array, ArrayOfDoubles) {
  std::string buf(R"([10.5, 11.2])");
  std::vector<double> values;

  auto elementCb = [&](const double &value) {
    values.push_back(value);
    return true;
  };

  Parser<Array<Value<double>>> parser(elementCb);

  ASSERT_TRUE(parser.parse(buf));
  ASSERT_TRUE(parser.finish());

  ASSERT_EQ(2, values.size());
  ASSERT_EQ(10.5, values[0]);
  ASSERT_EQ(11.2, values[1]);

  ASSERT_TRUE(parser.parser().isSet());
}

TEST(Array, ArrayOfStrings) {
  std::string buf(R"(["value1", "value2"])");
  std::vector<std::string> values;

  auto elementCb = [&](const std::string &value) {
    values.push_back(value);
    return true;
  };

  Parser<Array<Value<std::string>>> parser(elementCb);

  ASSERT_TRUE(parser.parse(buf));
  ASSERT_TRUE(parser.finish());

  ASSERT_EQ(2, values.size());
  ASSERT_EQ("value1", values[0]);
  ASSERT_EQ("value2", values[1]);

  ASSERT_TRUE(parser.parser().isSet());
}

TEST(Array, ArrayWithNull) {
  std::string buf(R"([null])");
  std::vector<bool> values;

  auto elementCb = [&](const bool &value) {
    values.push_back(value);
    return true;
  };

  Parser<Array<Value<bool>>> parser(elementCb);

  ASSERT_TRUE(parser.parse(buf));
  ASSERT_TRUE(parser.finish());

  ASSERT_EQ(0, values.size());

  ASSERT_TRUE(parser.parser().isSet());
}

TEST(Array, ArrayWithNullAndValues) {
  std::string buf(R"([null, true, null, false])");
  std::vector<bool> values;

  auto elementCb = [&](const bool &value) {
    values.push_back(value);
    return true;
  };

  Parser<Array<Value<bool>>> parser(elementCb);

  ASSERT_TRUE(parser.parse(buf));
  ASSERT_TRUE(parser.finish());

  ASSERT_EQ(2, values.size());
  ASSERT_EQ(true, values[0]);
  ASSERT_EQ(false, values[1]);

  ASSERT_TRUE(parser.parser().isSet());
}

TEST(Array, ArrayWithUnexpectedBoolean) {
  std::string buf(R"([true])");

  auto elementCb = [&](const std::string &) { return true; };

  Parser<Array<Value<std::string>>> parser(elementCb);

  ASSERT_FALSE(parser.parse(buf));

  ASSERT_FALSE(parser.parser().isSet());
  ASSERT_EQ("Unexpected token boolean", parser.getError());

  ASSERT_EQ(
      R"(parse error: client cancelled parse via callback return value
                                   [true]
                     (right here) ------^
Unexpected token boolean
)",
      parser.getError(true));
}

TEST(Array, ArrayWithUnexpectedInteger) {
  std::string buf(R"([10])");

  auto elementCb = [&](const bool &) { return true; };

  Parser<Array<Value<bool>>> parser(elementCb);

  ASSERT_FALSE(parser.parse(buf));

  ASSERT_FALSE(parser.parser().isSet());
  ASSERT_EQ("Unexpected token integer", parser.getError());

  ASSERT_EQ(
      R"(parse error: client cancelled parse via callback return value
                                     [10]
                     (right here) ------^
Unexpected token integer
)",
      parser.getError(true));
}

TEST(Array, ArrayWithUnexpectedDouble) {
  std::string buf(R"([10.5])");

  auto elementCb = [&](const bool &) { return true; };

  Parser<Array<Value<bool>>> parser(elementCb);

  ASSERT_FALSE(parser.parse(buf));

  ASSERT_FALSE(parser.parser().isSet());
  ASSERT_EQ("Unexpected token double", parser.getError());

  ASSERT_EQ(
      R"(parse error: client cancelled parse via callback return value
                                   [10.5]
                     (right here) ------^
Unexpected token double
)",
      parser.getError(true));
}

TEST(Array, ArrayWithUnexpectedString) {
  std::string buf(R"(["value"])");

  auto elementCb = [&](const bool &) { return true; };

  Parser<Array<Value<bool>>> parser(elementCb);

  ASSERT_FALSE(parser.parse(buf));

  ASSERT_FALSE(parser.parser().isSet());
  ASSERT_EQ("Unexpected token string", parser.getError());

  ASSERT_EQ(
      R"(parse error: client cancelled parse via callback return value
                                ["value"]
                     (right here) ------^
Unexpected token string
)",
      parser.getError(true));
}

TEST(Array, ArrayWithElementCallbackError) {
  std::string buf(R"([true, false])");

  auto elementCb = [&](const bool &) { return false; };

  Parser<Array<Value<bool>>> parser(elementCb);

  ASSERT_FALSE(parser.parse(buf));
  ASSERT_FALSE(parser.parser().isSet());

  ASSERT_EQ("Callback returned false", parser.getError());
  ASSERT_EQ(
      R"(parse error: client cancelled parse via callback return value
                                   [true, false]
                     (right here) ------^
Callback returned false
)",
      parser.getError(true));
}

TEST(Array, ArrayWithCallback) {
  std::string buf(R"([true, false])");
  std::vector<bool> values;
  bool callback_called = false;

  auto elementCb = [&](const bool &value) {
    values.push_back(value);
    return true;
  };

  using ArrayParser = Array<Value<bool>>;

  auto arrayCb = [&](ArrayParser &) {
    callback_called = true;
    return true;
  };

  Parser<ArrayParser> parser({elementCb, arrayCb});

  ASSERT_TRUE(parser.parse(buf));
  ASSERT_TRUE(parser.finish());

  ASSERT_EQ(2, values.size());
  ASSERT_EQ(true, values[0]);
  ASSERT_EQ(false, values[1]);

  ASSERT_EQ(true, callback_called);

  ASSERT_TRUE(parser.parser().isSet());
}

TEST(Array, ArrayWithCallbackError) {
  std::string buf(R"([true, false])");

  auto elementCb = [&](const bool &) { return true; };

  using ArrayParser = Array<Value<bool>>;

  auto arrayCb = [&](ArrayParser &) { return false; };

  Parser<ArrayParser> parser({elementCb, arrayCb});

  ASSERT_FALSE(parser.parse(buf));
  ASSERT_TRUE(parser.parser().isSet());

  ASSERT_EQ("Callback returned false", parser.getError());
  ASSERT_EQ(
      R"(parse error: client cancelled parse via callback return value
                           [true, false]
                     (right here) ------^
Callback returned false
)",
      parser.getError(true));
}

TEST(Array, ArrayWithArgsStruct) {
  std::string buf(R"([true, false])");
  std::vector<bool> values;

  auto elementCb = [&](const bool &value) {
    values.push_back(value);
    return true;
  };

  using ArrayParser = Array<Value<bool>>;

  ArrayParser::Args array_args = {elementCb};

  Parser<ArrayParser> parser(array_args);

  ASSERT_TRUE(parser.parse(buf));
  ASSERT_TRUE(parser.finish());

  ASSERT_EQ(2, values.size());
  ASSERT_EQ(true, values[0]);
  ASSERT_EQ(false, values[1]);

  ASSERT_TRUE(parser.parser().isSet());
}

TEST(Array, ArrayOfObjects) {
  std::string buf(
      R"([{"key": "value", "key2": 10}, {"key": "value2", "key2": 20}])");

  struct ObjectStruct {
    std::string field1;
    int64_t field2;
  };

  std::vector<ObjectStruct> values;

  using ObjectParser = Object<Value<std::string>, Value<int64_t>>;

  auto objectCb = [&](ObjectParser &parser) {
    values.push_back({parser.get<0>().pop(), parser.get<1>().pop()});
    return true;
  };

  Parser<Array<ObjectParser>> parser({{"key", "key2"}, objectCb});

  ASSERT_TRUE(parser.parse(buf));
  ASSERT_TRUE(parser.finish());

  ASSERT_EQ(2, values.size());
  ASSERT_EQ("value", values[0].field1);
  ASSERT_EQ(10, values[0].field2);
  ASSERT_EQ("value2", values[1].field1);
  ASSERT_EQ(20, values[1].field2);
}

TEST(Array, ArrayOfObjectsWithoutCallbacks) {
  std::string buf(
      R"([{"key": "value", "key2": 10}, {"key": "value2", "key2": 20}])");

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

  Parser<Array<Object<Value<std::string>, Value<int64_t>>>> parser(
      {{"key", stringCb}, {"key2", integerCb}});

  ASSERT_TRUE(parser.parse(buf));
  ASSERT_TRUE(parser.finish());

  ASSERT_EQ(2, str_values.size());
  ASSERT_EQ("value", str_values[0]);
  ASSERT_EQ("value2", str_values[1]);
  ASSERT_EQ(2, int_values.size());
  ASSERT_EQ(10, int_values[0]);
  ASSERT_EQ(20, int_values[1]);
}

TEST(Array, ArrayWithUnexpectedObject) {
  std::string buf(
      R"([{"key2": "value"}])");

  Parser<Array<Object<Value<std::string>>>> parser("key");

  ASSERT_FALSE(parser.parse(buf));

  ASSERT_FALSE(parser.parser().isSet());
  ASSERT_EQ("Unexpected field key2", parser.getError());

  ASSERT_EQ(
      R"(parse error: client cancelled parse via callback return value
                                [{"key2": "value"}]
                     (right here) ------^
Unexpected field key2
)",
      parser.getError(true));
}

TEST(Array, ArrayOfSCustomObjects) {
  std::string buf(
      R"([{"key": "value", "key2": 10}, {"key": "value2", "key2": 20}])");

  struct ObjectStruct {
    std::string field1;
    int64_t field2;
  };

  std::vector<ObjectStruct> values;

  using ObjectParser =
      SObject<ObjectStruct, Value<std::string>, Value<int64_t>>;

  auto objectCb = [&](ObjectParser &parser, ObjectStruct &value) {
    value = {parser.get<0>().pop(), parser.get<1>().pop()};
    values.push_back(value);
    return true;
  };

  Parser<Array<ObjectParser>> parser({{"key", "key2"}, objectCb});

  ASSERT_TRUE(parser.parse(buf));
  ASSERT_TRUE(parser.finish());

  ASSERT_EQ(2, values.size());
  ASSERT_EQ("value", values[0].field1);
  ASSERT_EQ(10, values[0].field2);
  ASSERT_EQ("value2", values[1].field1);
  ASSERT_EQ(20, values[1].field2);
}

TEST(Array, ArrayOfSAutoObjects) {
  std::string buf(
      R"([{"key": "value", "key2": 10}, {"key": "value2", "key2": 20}])");

  std::vector<std::tuple<std::string, int64_t>> values;

  using ObjectParser = SObject<Value<std::string>, Value<int64_t>>;

  auto objectCb = [&](const std::tuple<std::string, int64_t> &value) {
    values.push_back(value);
    return true;
  };

  Parser<Array<ObjectParser>> parser({{"key", "key2"}, objectCb});

  ASSERT_TRUE(parser.parse(buf));
  ASSERT_TRUE(parser.finish());

  ASSERT_EQ(2, values.size());
  ASSERT_EQ("value", std::get<0>(values[0]));
  ASSERT_EQ(10, std::get<1>(values[0]));
  ASSERT_EQ("value2", std::get<0>(values[1]));
  ASSERT_EQ(20, std::get<1>(values[1]));
}

TEST(Array, ArrayOfStandaloneUnions) {
  std::string buf(
      R"([{"type": "str", "key": "value"}, {"type": "int", "key": 10}])");

  std::string value_str = "";
  int64_t value_int = 0;

  using UnionParser =
      Union<std::string, Object<Value<std::string>>, Object<Value<int64_t>>>;

  auto unionCb = [&](UnionParser &parser) {
    switch (parser.currentMemberId()) {
      case 0:
        value_str = parser.get<0>().get<0>().get();
        break;
      case 1:
        value_int = parser.get<1>().get<0>().get();
        break;
      default:
        return false;
    }
    return true;
  };

  Parser<Array<UnionParser>> parser(
      {"type", {{"str", "key"}, {"int", "key"}}, unionCb});

  ASSERT_TRUE(parser.parse(buf));
  ASSERT_TRUE(parser.finish());

  ASSERT_EQ("value", value_str);
  ASSERT_EQ(10, value_int);
}

TEST(Array, ArrayOfObjectUnions) {
  std::string buf(
      R"([{"id": 1, "type": "str", "key": "value"},
          {"id": 2, "type": "int", "key": 10}])");

  std::vector<int64_t> ids;
  std::string value_str = "";
  int64_t value_int = 0;

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

  auto objectCb = [&](ObjectParser &parser) {
    ids.push_back(parser.get<0>().get());

    switch (parser.get<1>().currentMemberId()) {
      case 0:
        value_str = parser.get<1>().get<0>().get<0>().get();
        break;
      case 1:
        value_int = parser.get<1>().get<1>().get<0>().get();
        break;
      default:
        return false;
    }
    return true;
  };

  Parser<Array<ObjectParser>> parser(
      {{"id", {"type", {{"str", "key"}, {"int", "key"}}}}, objectCb});

  ASSERT_TRUE(parser.parse(buf));
  ASSERT_TRUE(parser.finish());

  ASSERT_EQ(2, ids.size());
  ASSERT_EQ(1, ids[0]);
  ASSERT_EQ(2, ids[1]);
  ASSERT_EQ("value", value_str);
  ASSERT_EQ(10, value_int);
}

TEST(Array, ArrayOfArrays) {
  std::string buf(R"([[true, true], [false, false]])");
  std::vector<std::vector<bool>> values;
  std::vector<bool> tmp_values;

  auto elementCb = [&](const bool &value) {
    tmp_values.push_back(value);
    return true;
  };

  using InnerArrayParser = Array<Value<bool>>;

  auto innerArrayCb = [&](InnerArrayParser &) {
    values.push_back(tmp_values);
    tmp_values.clear();
    return true;
  };

  Parser<Array<InnerArrayParser>> parser({elementCb, innerArrayCb});

  ASSERT_TRUE(parser.parse(buf));
  ASSERT_TRUE(parser.finish());

  ASSERT_EQ(2, values.size());
  ASSERT_EQ(2, values[0].size());
  ASSERT_EQ(true, values[0][0]);
  ASSERT_EQ(true, values[0][1]);

  ASSERT_EQ(2, values[1].size());
  ASSERT_EQ(false, values[1][0]);
  ASSERT_EQ(false, values[1][1]);
}

TEST(Array, ArrayOfSArrays) {
  std::string buf(R"([[true, true], [false, false]])");
  std::vector<std::vector<bool>> values;

  auto innerArrayCb = [&](const std::vector<bool> &value) {
    values.push_back(value);
    return true;
  };

  Parser<Array<SArray<Value<bool>>>> parser({innerArrayCb});

  ASSERT_TRUE(parser.parse(buf));
  ASSERT_TRUE(parser.finish());

  ASSERT_EQ(2, values.size());
  ASSERT_EQ(2, values[0].size());
  ASSERT_EQ(true, values[0][0]);
  ASSERT_EQ(true, values[0][1]);

  ASSERT_EQ(2, values[1].size());
  ASSERT_EQ(false, values[1][0]);
  ASSERT_EQ(false, values[1][1]);
}
