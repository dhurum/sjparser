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

TEST(Parser, boolean) {
  std::string buf(R"(true)");

  Parser<Value<bool>> parser;

  ASSERT_FALSE(parser.parser().isSet());

  ASSERT_TRUE(parser.parse(buf));
  ASSERT_TRUE(parser.finish());

  ASSERT_TRUE(parser.parser().isSet());
  ASSERT_EQ(true, parser.parser().get());

  ASSERT_TRUE(parser.parser().isSet());
  ASSERT_EQ(true, parser.parser().pop());
  ASSERT_FALSE(parser.parser().isSet());
}

TEST(Parser, integer) {
  std::string buf(R"(10)");

  Parser<Value<int64_t>> parser;

  ASSERT_FALSE(parser.parser().isSet());

  ASSERT_TRUE(parser.parse(buf));
  ASSERT_TRUE(parser.finish());

  ASSERT_TRUE(parser.parser().isSet());
  ASSERT_EQ(10, parser.parser().get());

  ASSERT_TRUE(parser.parser().isSet());
  ASSERT_EQ(10, parser.parser().pop());
  ASSERT_FALSE(parser.parser().isSet());
}

TEST(Parser, double) {
  std::string buf(R"(1.3)");

  Parser<Value<double>> parser;

  ASSERT_FALSE(parser.parser().isSet());

  ASSERT_TRUE(parser.parse(buf));
  ASSERT_TRUE(parser.finish());

  ASSERT_TRUE(parser.parser().isSet());
  ASSERT_EQ(1.3, parser.parser().get());

  ASSERT_TRUE(parser.parser().isSet());
  ASSERT_EQ(1.3, parser.parser().pop());
  ASSERT_FALSE(parser.parser().isSet());
}

TEST(Parser, string) {
  std::string buf(R"("value")");

  Parser<Value<std::string>> parser;

  ASSERT_FALSE(parser.parser().isSet());

  ASSERT_TRUE(parser.parse(buf));
  ASSERT_TRUE(parser.finish());

  ASSERT_TRUE(parser.parser().isSet());
  ASSERT_EQ("value", parser.parser().get());

  ASSERT_TRUE(parser.parser().isSet());
  ASSERT_EQ("value", parser.parser().pop());
  ASSERT_FALSE(parser.parser().isSet());
}

TEST(Parser, object) {
  std::string buf(R"({"key": "value", "key2": 10})");

  Parser<Object<Value<std::string>, Value<int64_t>>> parser({"key", "key2"});

  ASSERT_TRUE(parser.parse(buf));
  ASSERT_TRUE(parser.finish());

  ASSERT_EQ("value", parser.parser().get<0>().get());
  ASSERT_EQ(10, parser.parser().get<1>().get());
}

TEST(Parser, objectStdString) {
  std::string buf(R"({"key": "value", "key2": 10})");

  std::string key1 = "key";
  std::string key2 = "key2";
  Parser<Object<Value<std::string>, Value<int64_t>>> parser({key1, key2});

  ASSERT_TRUE(parser.parse(buf));
  ASSERT_TRUE(parser.finish());

  ASSERT_EQ("value", parser.parser().get<0>().get());
  ASSERT_EQ(10, parser.parser().get<1>().get());
}

TEST(Parser, emptyObject) {
  std::string buf(R"({})");

  Parser<Object<Value<std::string>, Value<int64_t>>> parser({"key", "key2"});

  ASSERT_TRUE(parser.parse(buf));
  ASSERT_TRUE(parser.finish());
}

TEST(Parser, objectWithOneField) {
  std::string buf(R"({"key": "value"})");

  Parser<Object<Value<std::string>>> parser("key");

  ASSERT_TRUE(parser.parse(buf));
  ASSERT_TRUE(parser.finish());

  ASSERT_EQ("value", parser.parser().get<0>().get());
}

TEST(Parser, objectWithObject) {
  std::string buf(
      R"(
{
  "key": "value",
  "key2": 10,
  "key3": {
    "key": 1,
    "key2": "in_value"
  },
  "key4": true
})");

  // clang-format off
  Parser<Object<
    Value<std::string>,
    Value<int64_t>,
    Object<
      Value<int64_t>,
      Value<std::string>
    >,
    Value<bool>
  >> parser({"key", "key2", {"key3", {"key", "key2"}}, "key4"});
  // clang-format on

  ASSERT_TRUE(parser.parse(buf));
  ASSERT_TRUE(parser.finish());

  ASSERT_EQ("value", parser.parser().get<0>().get());
  ASSERT_EQ(10, parser.parser().get<1>().get());
  ASSERT_EQ(1, parser.parser().get<2>().get<0>().get());
  ASSERT_EQ("in_value", parser.parser().get<2>().get<1>().get());
  ASSERT_EQ(true, parser.parser().get<3>().get());
}

TEST(Parser, objectWithObjectWithCallback) {
  std::string buf(
      R"(
{
  "key": "value",
  "key2": 10,
  "key3": {
    "key": 1,
    "key2": "in_value"
  },
  "key4": true
})");

  struct ObjectStruct {
    int64_t int_field;
    std::string str_field;
  };

  ObjectStruct value;

  using InnerObjectParser = Object<Value<int64_t>, Value<std::string>>;

  auto callback = [&](InnerObjectParser &parser) {
    value.int_field = parser.get<0>().get();
    value.str_field = parser.get<1>().get();
    return true;
  };

  InnerObjectParser::Args inner_object_args{{"key", "key2"}, callback};

  // clang-format off
  Parser<Object<
    Value<std::string>,
    Value<int64_t>,
    InnerObjectParser,
    Value<bool>
  >> parser({"key", "key2", {"key3", inner_object_args}, "key4"});
  // clang-format on

  ASSERT_TRUE(parser.parse(buf));
  ASSERT_TRUE(parser.finish());

  ASSERT_EQ("value", parser.parser().get<0>().get());
  ASSERT_EQ(10, parser.parser().get<1>().get());
  ASSERT_EQ(1, value.int_field);
  ASSERT_EQ("in_value", value.str_field);
  ASSERT_EQ(true, parser.parser().get<3>().get());
}

TEST(Parser, objectOfObjects) {
  std::string buf(
      R"(
{
  "key": {
    "key": "value",
    "key2": 10
  },
  "key2": {
    "key": 1,
    "key2": "value2"
  },
  "key3": {
    "key": true
  }
})");

  Parser<Object<Object<Value<std::string>, Value<int64_t>>,
                Object<Value<int64_t>, Value<std::string>>,
                Object<Value<bool>>>>
      parser({{"key", {"key", "key2"}},
              {"key2", {"key", "key2"}},
              {"key3", {"key"}}});

  ASSERT_TRUE(parser.parse(buf));
  ASSERT_TRUE(parser.finish());

  ASSERT_EQ("value", parser.parser().get<0>().get<0>().get());
  ASSERT_EQ(10, parser.parser().get<0>().get<1>().get());
  ASSERT_EQ(1, parser.parser().get<1>().get<0>().get());
  ASSERT_EQ("value2", parser.parser().get<1>().get<1>().get());
  ASSERT_EQ(true, parser.parser().get<2>().get<0>().get());
}

TEST(Parser, storageCustomObject) {
  std::string buf(R"({"key": "value", "key2": 10})");

  struct TestStruct {
    std::string str_value;
    int64_t int_value;
  };

  using ParserType = SObject<TestStruct, Value<std::string>, Value<int64_t>>;

  auto makeCb = [&](ParserType &parser, TestStruct &value) {
    value.str_value = parser.get<0>().pop();
    value.int_value = parser.get<1>().pop();
    return true;
  };

  Parser<ParserType> parser({{"key", "key2"}, makeCb});

  ASSERT_TRUE(parser.parse(buf));
  ASSERT_TRUE(parser.finish());

  ASSERT_EQ("value", parser.parser().get().str_value);
  ASSERT_EQ(10, parser.parser().get().int_value);

  ASSERT_TRUE(parser.parser().isSet());
  parser.parser().pop();
  ASSERT_FALSE(parser.parser().isSet());
}

TEST(Parser, storageAutoObject) {
  std::string buf(R"({"key": "value", "key2": 10})");

  using ParserType = SObject<Value<std::string>, Value<int64_t>>;

  Parser<ParserType> parser({{"key", "key2"}, {"test", 1}});

  ASSERT_TRUE(parser.parse(buf));
  ASSERT_TRUE(parser.finish());

  ASSERT_EQ("value", std::get<0>(parser.parser().get()));
  ASSERT_EQ(10, std::get<1>(parser.parser().get()));
}

TEST(Parser, storageAutoObjectDefault) {
  std::string buf(R"({})");

  using ParserType = SObject<Value<std::string>, Value<int64_t>>;

  Parser<ParserType> parser({{"key", "key2"}, {"test", 1}});

  ASSERT_TRUE(parser.parse(buf));
  ASSERT_TRUE(parser.finish());

  ASSERT_EQ("test", std::get<0>(parser.parser().get()));
  ASSERT_EQ(1, std::get<1>(parser.parser().get()));
}

TEST(Parser, storageAutoObjectMove) {
  std::string buf(R"({"key": "value", "key2": 10})");

  using ParserType = SObject<Value<std::string>, Value<int64_t>>;

  Parser<ParserType> parser({{"key", "key2"}, {"test", 1}});

  ASSERT_TRUE(parser.parse(buf));
  ASSERT_TRUE(parser.finish());

  ASSERT_TRUE(parser.parser().isSet());
  auto value = parser.parser().pop();
  ASSERT_FALSE(parser.parser().isSet());

  ASSERT_EQ("value", std::get<0>(value));
  ASSERT_EQ(10, std::get<1>(value));

  std::string buf2(R"({"key": "value2"})");

  ASSERT_TRUE(parser.parse(buf2));
  ASSERT_TRUE(parser.finish());

  ASSERT_TRUE(parser.parser().isSet());
  auto value2 = parser.parser().pop();
  ASSERT_FALSE(parser.parser().isSet());

  ASSERT_EQ("value2", std::get<0>(value2));
  ASSERT_EQ(1, std::get<1>(value2));
}

TEST(Parser, storageArray) {
  std::string buf(R"(["value", "value2"])");

  Parser<SArray<Value<std::string>>> parser;

  ASSERT_TRUE(parser.parse(buf));
  ASSERT_TRUE(parser.finish());

  ASSERT_EQ(2, parser.parser().get().size());
  ASSERT_EQ("value", parser.parser().get()[0]);
  ASSERT_EQ("value2", parser.parser().get()[1]);

  ASSERT_TRUE(parser.parser().isSet());
  parser.parser().pop();
  ASSERT_FALSE(parser.parser().isSet());
}

TEST(Parser, arrayWithEltCallback) {
  std::string buf(R"(["value", "value2"])");
  std::vector<std::string> values;

  auto elementCb = [&](const std::string &value) {
    values.push_back(value);
    return true;
  };

  Parser<Array<Value<std::string>>> parser(elementCb);

  ASSERT_TRUE(parser.parse(buf));
  ASSERT_TRUE(parser.finish());

  ASSERT_EQ(2, values.size());
  ASSERT_EQ("value", values[0]);
  ASSERT_EQ("value2", values[1]);
}

TEST(Parser, storageArrayWithCallback) {
  std::string buf(
      R"(["value", "value2"])");
  std::vector<std::string> values;

  auto arrayCb = [&](const std::vector<std::string> &array_values) {
    values = array_values;
    return true;
  };

  Parser<SArray<Value<std::string>>> parser(arrayCb);

  ASSERT_TRUE(parser.parse(buf));
  ASSERT_TRUE(parser.finish());

  ASSERT_EQ(2, values.size());
  ASSERT_EQ("value", values[0]);
  ASSERT_EQ("value2", values[1]);
}

TEST(Parser, emptyStorageArray) {
  std::string buf(R"([])");

  Parser<SArray<Value<std::string>>> parser;

  ASSERT_TRUE(parser.parse(buf));
  ASSERT_TRUE(parser.finish());

  ASSERT_EQ(0, parser.parser().get().size());
}

TEST(Parser, storageArrayOfStorageArrays) {
  std::string buf(R"([["value", "value2"], ["value3", "value4"]])");

  Parser<SArray<SArray<Value<std::string>>>> parser;

  ASSERT_TRUE(parser.parse(buf));
  ASSERT_TRUE(parser.finish());

  ASSERT_EQ(2, parser.parser().get().size());
  ASSERT_EQ(2, parser.parser().get()[0].size());
  ASSERT_EQ(2, parser.parser().get()[1].size());
  ASSERT_EQ("value", parser.parser().get()[0][0]);
  ASSERT_EQ("value2", parser.parser().get()[0][1]);
  ASSERT_EQ("value3", parser.parser().get()[1][0]);
  ASSERT_EQ("value4", parser.parser().get()[1][1]);
}

TEST(Parser, arrayOfObjects) {
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

  ObjectParser::Args object_args{{"key", "key2"}, objectCb};

  Parser<Array<ObjectParser>> parser(object_args);

  ASSERT_TRUE(parser.parse(buf));
  ASSERT_TRUE(parser.finish());

  ASSERT_EQ(2, values.size());
  ASSERT_EQ("value", values[0].field1);
  ASSERT_EQ(10, values[0].field2);
  ASSERT_EQ("value2", values[1].field1);
  ASSERT_EQ(20, values[1].field2);
}

TEST(Parser, storageArrayOfStorageCustomObjects) {
  std::string buf(
      R"([{"key": "value", "key2": 10}, {"key": "value2", "key2": 20}])");

  struct TestStruct {
    std::string str_value;
    int64_t int_value;
  };

  using ObjectParser = SObject<TestStruct, Value<std::string>, Value<int64_t>>;

  auto makeCb = [&](ObjectParser &parser, TestStruct &value) {
    value.str_value = parser.get<0>().pop();
    value.int_value = parser.get<1>().pop();
    return true;
  };

  ObjectParser::Args object_args{{"key", "key2"}, makeCb};

  Parser<SArray<ObjectParser>> parser(object_args);

  ASSERT_TRUE(parser.parse(buf));
  ASSERT_TRUE(parser.finish());

  ASSERT_EQ(2, parser.parser().get().size());
  ASSERT_EQ("value", parser.parser().get()[0].str_value);
  ASSERT_EQ(10, parser.parser().get()[0].int_value);
  ASSERT_EQ("value2", parser.parser().get()[1].str_value);
  ASSERT_EQ(20, parser.parser().get()[1].int_value);
}

TEST(Parser, storageArrayOfStorageAutoObjects) {
  std::string buf(
      R"([{"key": "value", "key2": 10}, {"key": "value2", "key2": 20}])");

  using ObjectParser = SObject<Value<std::string>, Value<int64_t>>;

  ObjectParser::Args object_args{{"key", "key2"}};

  Parser<SArray<ObjectParser>> parser(object_args);

  ASSERT_TRUE(parser.parse(buf));
  ASSERT_TRUE(parser.finish());

  ASSERT_EQ(2, parser.parser().get().size());
  ASSERT_EQ("value", std::get<0>(parser.parser().get()[0]));
  ASSERT_EQ(10, std::get<1>(parser.parser().get()[0]));
  ASSERT_EQ("value2", std::get<0>(parser.parser().get()[1]));
  ASSERT_EQ(20, std::get<1>(parser.parser().get()[1]));
}

TEST(Parser, objectWithArray) {
  std::string buf(
      R"(
{
  "key": "value",
  "key2": 10,
  "key3": [
    "elt1",
    "elt2",
    "elt3"
  ]
})");

  struct ObjectWArrayStruct {
    std::string field1;
    int64_t field2;
    std::vector<std::string> array;
  };

  using ParserType =
      Object<Value<std::string>, Value<int64_t>, Array<Value<std::string>>>;

  ObjectWArrayStruct object;

  auto arrayEltCb = [&](const std::string &value) {
    object.array.push_back(value);

    return true;
  };

  auto objectCb = [&](ParserType &parser) {
    object.field1 = parser.get<0>().pop();
    object.field2 = parser.get<1>().pop();

    return true;
  };

  Parser<ParserType> parser({{"key", "key2", {"key3", arrayEltCb}}, objectCb});

  ASSERT_TRUE(parser.parse(buf));
  ASSERT_TRUE(parser.finish());

  ASSERT_EQ("value", object.field1);
  ASSERT_EQ(10, object.field2);
  ASSERT_EQ(3, object.array.size());
  ASSERT_EQ("elt1", object.array[0]);
  ASSERT_EQ("elt2", object.array[1]);
  ASSERT_EQ("elt3", object.array[2]);
}

TEST(Parser, storageCustomObjectWithStorageArray) {
  std::string buf(
      R"(
{
  "key": "value",
  "key2": 10,
  "key3": [
    "elt1",
    "elt2",
    "elt3"
  ]
})");

  struct TestStruct {
    std::string str_value;
    int64_t int_value;
    std::vector<std::string> array;
  };

  using ParserType = SObject<TestStruct, Value<std::string>, Value<int64_t>,
                             SArray<Value<std::string>>>;

  auto makeCb = [&](ParserType &parser, TestStruct &value) {
    value.str_value = parser.get<0>().pop();
    value.int_value = parser.get<1>().pop();
    value.array = parser.get<2>().pop();
    return true;
  };

  Parser<ParserType> parser({{"key", "key2", "key3"}, makeCb});

  ASSERT_TRUE(parser.parse(buf));
  ASSERT_TRUE(parser.finish());

  ASSERT_EQ("value", parser.parser().get().str_value);
  ASSERT_EQ(10, parser.parser().get().int_value);
  ASSERT_EQ(3, parser.parser().get().array.size());
  ASSERT_EQ("elt1", parser.parser().get().array[0]);
  ASSERT_EQ("elt2", parser.parser().get().array[1]);
  ASSERT_EQ("elt3", parser.parser().get().array[2]);
}

TEST(Parser, storageAutoObjectWithStorageArray) {
  std::string buf(
      R"(
{
  "key": "value",
  "key2": 10,
  "key3": [
    "elt1",
    "elt2",
    "elt3"
  ]
})");

  using ParserType =
      SObject<Value<std::string>, Value<int64_t>, SArray<Value<std::string>>>;

  Parser<ParserType> parser({"key", "key2", "key3"});

  ASSERT_TRUE(parser.parse(buf));
  ASSERT_TRUE(parser.finish());

  auto value = parser.parser().get();

  ASSERT_EQ("value", std::get<0>(value));
  ASSERT_EQ(10, std::get<1>(value));
  ASSERT_EQ(3, std::get<2>(value).size());
  ASSERT_EQ("elt1", std::get<2>(value)[0]);
  ASSERT_EQ("elt2", std::get<2>(value)[1]);
  ASSERT_EQ("elt3", std::get<2>(value)[2]);
}

TEST(Parser, error) {
  std::string buf(R"("error")");

  Parser<Value<bool>> parser;

  ASSERT_FALSE(parser.parse(buf));
  ASSERT_EQ("Unexpected token string", parser.getError());
}

TEST(Parser, errorVerbose) {
  std::string buf(R"("error")");

  Parser<Value<bool>> parser;

  ASSERT_FALSE(parser.parse(buf));
  ASSERT_EQ(
      R"(parse error: client cancelled parse via callback return value
                                 "error"
                     (right here) ------^
Unexpected token string
)",
      parser.getError(true));
}

TEST(Parser, getUnsetValue) {
  Parser<Value<bool>> parser;

  ASSERT_ANY_THROW(parser.parser().get());
}

TEST(Parser, repeatedParsing) {
  std::string buf(R"(true)");

  Parser<Value<bool>> parser;

  ASSERT_TRUE(parser.parse(buf));
  ASSERT_TRUE(parser.finish());

  ASSERT_TRUE(parser.parse(buf));
  ASSERT_TRUE(parser.finish());
}

TEST(Parser, unionObject) {
  std::string buf(
      R"(
{
  "id": 10,
  "type": 1,
  "bool": true
})");

  // clang-format off
  Parser<Object<
    Value<int64_t>,
    Union<
      int64_t,
      Object<Value<bool>>,
      Object<Value<int64_t>>
  >>> parser({"id", {"type", {{1, "bool"}, {2, "int"}}}});
  // clang-format on

  ASSERT_TRUE(parser.parse(buf));
  ASSERT_TRUE(parser.finish());

  ASSERT_TRUE(parser.parser().get<1>().get<0>().isSet());
  ASSERT_FALSE(parser.parser().get<1>().get<1>().isSet());
  ASSERT_EQ(0, parser.parser().get<1>().currentMemberId());

  ASSERT_EQ(10, parser.parser().get<0>().get());
  ASSERT_EQ(true, parser.parser().get<1>().get<0>().get<0>().get());

  std::string buf2(
      R"(
{
  "id": 20,
  "type": 2,
  "int": 100
})");

  ASSERT_TRUE(parser.parse(buf2));
  ASSERT_TRUE(parser.finish());

  ASSERT_FALSE(parser.parser().get<1>().get<0>().isSet());
  ASSERT_TRUE(parser.parser().get<1>().get<1>().isSet());
  ASSERT_EQ(1, parser.parser().get<1>().currentMemberId());

  ASSERT_EQ(20, parser.parser().get<0>().get());
  ASSERT_EQ(100, parser.parser().get<1>().get<1>().get<0>().get());
}

TEST(Parser, unionObjectString) {
  std::string buf(
      R"(
{
  "id": 10,
  "type": "1",
  "bool": true
})");

  // clang-format off
  Parser<Object<
    Value<int64_t>,
    Union<
      std::string,
      Object<Value<bool>>,
      Object<Value<int64_t>>
  >>> parser({"id", {"type", {{"1", "bool"}, {"2", "int"}}}});
  // clang-format on

  ASSERT_TRUE(parser.parse(buf));
  ASSERT_TRUE(parser.finish());

  ASSERT_TRUE(parser.parser().get<1>().get<0>().isSet());
  ASSERT_FALSE(parser.parser().get<1>().get<1>().isSet());
  ASSERT_EQ(0, parser.parser().get<1>().currentMemberId());

  ASSERT_EQ(10, parser.parser().get<0>().get());
  ASSERT_EQ(true, parser.parser().get<1>().get<0>().get<0>().get());

  std::string buf2(
      R"(
{
  "id": 20,
  "type": "2",
  "int": 100
})");

  ASSERT_TRUE(parser.parse(buf2));
  ASSERT_TRUE(parser.finish());

  ASSERT_FALSE(parser.parser().get<1>().get<0>().isSet());
  ASSERT_TRUE(parser.parser().get<1>().get<1>().isSet());
  ASSERT_EQ(1, parser.parser().get<1>().currentMemberId());

  ASSERT_EQ(20, parser.parser().get<0>().get());
  ASSERT_EQ(100, parser.parser().get<1>().get<1>().get<0>().get());
}

TEST(Parser, unionStandAlone) {
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
  >> parser({"type", {{1, "bool"}, {2, "int"}}});
  // clang-format on

  ASSERT_TRUE(parser.parse(buf));
  ASSERT_TRUE(parser.finish());

  ASSERT_TRUE(parser.parser().get<0>().isSet());
  ASSERT_FALSE(parser.parser().get<1>().isSet());
  ASSERT_EQ(0, parser.parser().currentMemberId());

  ASSERT_EQ(true, parser.parser().get<0>().get<0>().get());

  std::string buf2(
      R"(
{
  "type": 2,
  "int": 100
})");

  ASSERT_TRUE(parser.parse(buf2));
  ASSERT_TRUE(parser.finish());

  ASSERT_FALSE(parser.parser().get<0>().isSet());
  ASSERT_TRUE(parser.parser().get<1>().isSet());
  ASSERT_EQ(1, parser.parser().currentMemberId());

  ASSERT_EQ(100, parser.parser().get<1>().get<0>().get());
}

TEST(Parser, unionStandAloneString) {
  std::string buf(
      R"(
{
  "type": "1",
  "bool": true
})");

  Parser<Union<std::string, Object<Value<bool>>, Object<Value<int64_t>>>>
      parser({"type", {{"1", "bool"}, {"2", "int"}}});

  ASSERT_TRUE(parser.parse(buf));
  ASSERT_TRUE(parser.finish());

  ASSERT_TRUE(parser.parser().get<0>().isSet());
  ASSERT_FALSE(parser.parser().get<1>().isSet());
  ASSERT_EQ(0, parser.parser().currentMemberId());

  ASSERT_EQ(true, parser.parser().get<0>().get<0>().get());

  std::string buf2(
      R"(
{
  "type": "2",
  "int": 100
})");

  ASSERT_TRUE(parser.parse(buf2));
  ASSERT_TRUE(parser.finish());

  ASSERT_FALSE(parser.parser().get<0>().isSet());
  ASSERT_TRUE(parser.parser().get<1>().isSet());
  ASSERT_EQ(1, parser.parser().currentMemberId());

  ASSERT_EQ(100, parser.parser().get<1>().get<0>().get());
}

TEST(Parser, unionStandAloneStdString) {
  std::string buf(
      R"(
{
  "type": "1",
  "bool": true
})");

  // clang-format off
  Parser<Union<
    std::string,
    Object<Value<bool>>,
    Object<Value<int64_t>>
  >> parser({"type", {{"1", "bool"}, {"2", "int"}}});
  // clang-format on

  ASSERT_TRUE(parser.parse(buf));
  ASSERT_TRUE(parser.finish());

  ASSERT_TRUE(parser.parser().get<0>().isSet());
  ASSERT_FALSE(parser.parser().get<1>().isSet());
  ASSERT_EQ(0, parser.parser().currentMemberId());

  ASSERT_EQ(true, parser.parser().get<0>().get<0>().get());

  std::string buf2(
      R"(
{
  "type": "2",
  "int": 100
})");

  ASSERT_TRUE(parser.parse(buf2));
  ASSERT_TRUE(parser.finish());

  ASSERT_FALSE(parser.parser().get<0>().isSet());
  ASSERT_TRUE(parser.parser().get<1>().isSet());
  ASSERT_EQ(1, parser.parser().currentMemberId());

  ASSERT_EQ(100, parser.parser().get<1>().get<0>().get());
}

TEST(Parser, objectWithUnion) {
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
  Parser<Object<
    Value<int64_t>,
    Union<
      std::string,
      Object<Value<bool>>,
      Object<Value<int64_t>>
  >>> parser({"id", {"data", {"type", {{"1", "bool"}, {"2", "int"}}}}});
  // clang-format on

  ASSERT_TRUE(parser.parse(buf));
  ASSERT_TRUE(parser.finish());

  ASSERT_TRUE(parser.parser().get<1>().get<0>().isSet());
  ASSERT_FALSE(parser.parser().get<1>().get<1>().isSet());
  ASSERT_EQ(0, parser.parser().get<1>().currentMemberId());

  ASSERT_EQ(10, parser.parser().get<0>().get());
  ASSERT_EQ(true, parser.parser().get<1>().get<0>().get<0>().get());

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

  ASSERT_FALSE(parser.parser().get<1>().get<0>().isSet());
  ASSERT_TRUE(parser.parser().get<1>().get<1>().isSet());
  ASSERT_EQ(1, parser.parser().get<1>().currentMemberId());

  ASSERT_EQ(10, parser.parser().get<0>().get());
  ASSERT_EQ(100, parser.parser().get<1>().get<1>().get<0>().get());
}
