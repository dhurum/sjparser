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
}

TEST(Parser, integer) {
  std::string buf(R"(10)");

  Parser<Value<int64_t>> parser;

  ASSERT_FALSE(parser.parser().isSet());

  ASSERT_TRUE(parser.parse(buf));
  ASSERT_TRUE(parser.finish());

  ASSERT_TRUE(parser.parser().isSet());
  ASSERT_EQ(10, parser.parser().get());
}

TEST(Parser, double) {
  std::string buf(R"(1.3)");

  Parser<Value<double>> parser;

  ASSERT_FALSE(parser.parser().isSet());

  ASSERT_TRUE(parser.parse(buf));
  ASSERT_TRUE(parser.finish());

  ASSERT_TRUE(parser.parser().isSet());
  ASSERT_EQ(1.3, parser.parser().get());
}

TEST(Parser, string) {
  std::string buf(R"("value")");

  Parser<Value<std::string>> parser;

  ASSERT_FALSE(parser.parser().isSet());

  ASSERT_TRUE(parser.parse(buf));
  ASSERT_TRUE(parser.finish());

  ASSERT_TRUE(parser.parser().isSet());
  ASSERT_EQ("value", parser.parser().get());
}

TEST(Parser, object) {
  std::string buf(R"({"key": "value", "key2": 10})");

  Parser<Object<Value<std::string>, Value<int64_t>>> parser({{"key", "key2"}});

  ASSERT_TRUE(parser.parse(buf));
  ASSERT_TRUE(parser.finish());

  ASSERT_EQ("value", parser.parser().get<0>().get());
  ASSERT_EQ(10, parser.parser().get<1>().get());
}

TEST(Parser, emptyObject) {
  std::string buf(R"({})");

  Parser<Object<Value<std::string>, Value<int64_t>>> parser({{"key", "key2"}});

  ASSERT_TRUE(parser.parse(buf));
  ASSERT_TRUE(parser.finish());

  ASSERT_FALSE(parser.parser().isSet());
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

  Parser<Object<Value<std::string>, Value<int64_t>,
                Object<Value<int64_t>, Value<std::string>>, Value<bool>>>
      parser({{"key", "key2", {"key3", {{"key", "key2"}}}, "key4"}});

  ASSERT_TRUE(parser.parse(buf));
  ASSERT_TRUE(parser.finish());

  ASSERT_EQ("value", parser.parser().get<0>().get());
  ASSERT_EQ(10, parser.parser().get<1>().get());
  ASSERT_EQ(1, parser.parser().get<2>().get<0>().get());
  ASSERT_EQ("in_value", parser.parser().get<2>().get<1>().get());
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
      parser({{{"key", {{"key", "key2"}}},
               {"key2", {{"key", "key2"}}},
               {"key3", {{"key"}}}}});

  ASSERT_TRUE(parser.parse(buf));
  ASSERT_TRUE(parser.finish());

  ASSERT_EQ("value", parser.parser().get<0>().get<0>().get());
  ASSERT_EQ(10, parser.parser().get<0>().get<1>().get());
  ASSERT_EQ(1, parser.parser().get<1>().get<0>().get());
  ASSERT_EQ("value2", parser.parser().get<1>().get<1>().get());
  ASSERT_EQ(true, parser.parser().get<2>().get<0>().get());
}

TEST(Parser, storageObject) {
  std::string buf(R"({"key": "value", "key2": 10})");

  struct TestStruct {
    std::string str_value;
    int64_t int_value;
  };

  using ParserType = SObject<TestStruct, Value<std::string>, Value<int64_t>>;

  auto makeCb = [&](ParserType &parser, TestStruct &value) {
    value.str_value = parser.get<0>().get();
    value.int_value = parser.get<1>().get();
    return true;
  };

  Parser<ParserType> parser({{"key", "key2"}, makeCb});

  ASSERT_TRUE(parser.parse(buf));
  ASSERT_TRUE(parser.finish());

  ASSERT_EQ("value", parser.parser().get().str_value);
  ASSERT_EQ(10, parser.parser().get().int_value);
}

TEST(Parser, storageArray) {
  std::string buf(R"(["value", "value2"])");

  Parser<SArray<Value<std::string>>> parser;

  ASSERT_TRUE(parser.parse(buf));
  ASSERT_TRUE(parser.finish());

  ASSERT_EQ(2, parser.parser().get().size());
  ASSERT_EQ("value", parser.parser().get()[0]);
  ASSERT_EQ("value2", parser.parser().get()[1]);
}

TEST(Parser, arrayWithEltCallback) {
  std::string buf(R"(["value", "value2"])");
  std::vector<std::string> values;

  auto elementCb = [&](const std::string &value) {
    values.push_back(value);
    return true;
  };

  Parser<Array<Value<std::string>>> parser({elementCb});

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

  Parser<SArray<Value<std::string>>> parser({{}, arrayCb});

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

  using ParserType = Object<Value<std::string>, Value<int64_t>>;

  auto objectCb = [&](ParserType &parser) {
    values.push_back({parser.get<0>().get(), parser.get<1>().get()});
    return true;
  };

  Parser<Array<ParserType>> parser({{{"key", "key2"}, objectCb}});

  ASSERT_TRUE(parser.parse(buf));
  ASSERT_TRUE(parser.finish());

  ASSERT_EQ(2, values.size());
  ASSERT_EQ("value", values[0].field1);
  ASSERT_EQ(10, values[0].field2);
  ASSERT_EQ("value2", values[1].field1);
  ASSERT_EQ(20, values[1].field2);
}

TEST(Parser, storageArrayOfStorageObjects) {
  std::string buf(
      R"([{"key": "value", "key2": 10}, {"key": "value2", "key2": 20}])");

  struct TestStruct {
    std::string str_value;
    int64_t int_value;
  };

  using ParserType = SObject<TestStruct, Value<std::string>, Value<int64_t>>;

  auto makeCb = [&](ParserType &parser, TestStruct &value) {
    value.str_value = parser.get<0>().get();
    value.int_value = parser.get<1>().get();
    return true;
  };

  Parser<SArray<ParserType>> parser({{{"key", "key2"}, makeCb}});

  ASSERT_TRUE(parser.parse(buf));
  ASSERT_TRUE(parser.finish());

  ASSERT_EQ(2, parser.parser().get().size());
  ASSERT_EQ("value", parser.parser().get()[0].str_value);
  ASSERT_EQ(10, parser.parser().get()[0].int_value);
  ASSERT_EQ("value2", parser.parser().get()[1].str_value);
  ASSERT_EQ(20, parser.parser().get()[1].int_value);
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
    object.field1 = parser.get<0>().get();
    object.field2 = parser.get<1>().get();

    return true;
  };

  Parser<ParserType> parser(
      {{"key", "key2", {"key3", {{arrayEltCb}}}}, objectCb});

  ASSERT_TRUE(parser.parse(buf));
  ASSERT_TRUE(parser.finish());

  ASSERT_EQ("value", object.field1);
  ASSERT_EQ(10, object.field2);
  ASSERT_EQ(3, object.array.size());
  ASSERT_EQ("elt1", object.array[0]);
  ASSERT_EQ("elt2", object.array[1]);
  ASSERT_EQ("elt3", object.array[2]);
}

TEST(Parser, storageObjectWithArray) {
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
    value.str_value = parser.get<0>().get();
    value.int_value = parser.get<1>().get();
    value.array = parser.get<2>().get();
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
