#include <gtest/gtest.h>
#include "sjparser.h"

TEST(Parser, boolean) {
  std::string buf(R"(true)");
  auto value_parser = std::make_shared<SJParser::ValueParser<bool>>();

  SJParser::Parser parser(value_parser);

  ASSERT_FALSE(value_parser->isSet());

  ASSERT_TRUE(parser.parse(buf));
  ASSERT_TRUE(parser.finish());

  ASSERT_TRUE(value_parser->isSet());
  ASSERT_EQ(true, value_parser->get());
}

TEST(Parser, integer) {
  std::string buf(R"(10)");
  auto value_parser = std::make_shared<SJParser::ValueParser<int64_t>>();

  SJParser::Parser parser(value_parser);

  ASSERT_FALSE(value_parser->isSet());

  ASSERT_TRUE(parser.parse(buf));
  ASSERT_TRUE(parser.finish());

  ASSERT_TRUE(value_parser->isSet());
  ASSERT_EQ(10, value_parser->get());
}

TEST(Parser, double) {
  std::string buf(R"(1.3)");
  auto value_parser = std::make_shared<SJParser::ValueParser<double>>();

  SJParser::Parser parser(value_parser);

  ASSERT_FALSE(value_parser->isSet());

  ASSERT_TRUE(parser.parse(buf));
  ASSERT_TRUE(parser.finish());

  ASSERT_TRUE(value_parser->isSet());
  ASSERT_EQ(1.3, value_parser->get());
}

TEST(Parser, string) {
  std::string buf(R"("value")");
  auto value_parser = std::make_shared<SJParser::ValueParser<std::string>>();

  SJParser::Parser parser(value_parser);

  ASSERT_FALSE(value_parser->isSet());

  ASSERT_TRUE(parser.parse(buf));
  ASSERT_TRUE(parser.finish());

  ASSERT_TRUE(value_parser->isSet());
  ASSERT_EQ("value", value_parser->get());
}

TEST(Parser, object) {
  std::string buf(R"({"key": "value", "key2": 10})");
  auto value_parser =
      SJParser::makeObjectParser<SJParser::ValueParser<std::string>,
                                 SJParser::ValueParser<int64_t>>(
          {{{"key"}, {"key2"}}});

  SJParser::Parser parser(value_parser);

  ASSERT_TRUE(parser.parse(buf));
  ASSERT_TRUE(parser.finish());

  ASSERT_EQ("value", value_parser->get<0>().get());
  ASSERT_EQ(10, value_parser->get<1>().get());
}

TEST(Parser, emptyObject) {
  std::string buf(R"({})");
  auto value_parser =
      SJParser::makeObjectParser<SJParser::ValueParser<std::string>,
                                 SJParser::ValueParser<int64_t>>(
          {{{"key"}, {"key2"}}});

  SJParser::Parser parser(value_parser);

  ASSERT_TRUE(parser.parse(buf));
  ASSERT_TRUE(parser.finish());
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

  auto value_parser = SJParser::
      makeObjectParser<SJParser::ValueParser<std::string>,
                       SJParser::ValueParser<int64_t>,
                       SJParser::
                           ObjectParser<SJParser::ValueParser<int64_t>,
                                        SJParser::ValueParser<std::string>>,
                       SJParser::ValueParser<bool>>(
          {{{"key"}, {"key2"}, {"key3", {{{"key"}, {"key2"}}}}, {"key4"}}});

  SJParser::Parser parser(value_parser);

  ASSERT_TRUE(parser.parse(buf));
  ASSERT_TRUE(parser.finish());

  ASSERT_EQ("value", value_parser->get<0>().get());
  ASSERT_EQ(10, value_parser->get<1>().get());
  ASSERT_EQ(1, value_parser->get<2>().get<0>().get());
  ASSERT_EQ("in_value", value_parser->get<2>().get<1>().get());
  ASSERT_EQ(true, value_parser->get<3>().get());
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

  auto value_parser = SJParser::
      makeObjectParser<SJParser::
                           ObjectParser<SJParser::ValueParser<std::string>,
                                        SJParser::ValueParser<int64_t>>,
                       SJParser::
                           ObjectParser<SJParser::ValueParser<int64_t>,
                                        SJParser::ValueParser<std::string>>,
                       SJParser::ObjectParser<SJParser::ValueParser<bool>>>(
          {{{"key", {{{"key"}, {"key2"}}}},
            {"key2", {{{"key"}, {"key2"}}}},
            {"key3", {{{"key"}}}}}});

  SJParser::Parser parser(value_parser);

  ASSERT_TRUE(parser.parse(buf));
  ASSERT_TRUE(parser.finish());

  ASSERT_EQ("value", value_parser->get<0>().get<0>().get());
  ASSERT_EQ(10, value_parser->get<0>().get<1>().get());
  ASSERT_EQ(1, value_parser->get<1>().get<0>().get());
  ASSERT_EQ("value2", value_parser->get<1>().get<1>().get());
  ASSERT_EQ(true, value_parser->get<2>().get<0>().get());
}

TEST(Parser, array) {
  std::string buf(R"(["value", "value2"])");
  std::vector<std::string> values;

  auto value_parser =
      SJParser::makeArrayParser<SJParser::ValueParser<std::string>>(
          {[&](const std::string &value) {
            values.push_back(value);
            return true;
          }});

  SJParser::Parser parser(value_parser);

  ASSERT_TRUE(parser.parse(buf));
  ASSERT_TRUE(parser.finish());

  ASSERT_EQ(2, values.size());
  ASSERT_EQ("value", values[0]);
  ASSERT_EQ("value2", values[1]);
}

TEST(Parser, emptyArray) {
  std::string buf(R"([])");
  std::vector<std::string> values;

  auto elementCb = [&](const std::string &value) {
    values.push_back(value);
    return true;
  };

  auto value_parser =
      SJParser::makeArrayParser<SJParser::ValueParser<std::string>>(
          {elementCb});

  SJParser::Parser parser(value_parser);

  ASSERT_TRUE(parser.parse(buf));
  ASSERT_TRUE(parser.finish());

  ASSERT_EQ(0, values.size());
}

TEST(Parser, arrayOfArrays) {
  std::string buf(R"([["value", "value2"], ["value3", "value4"]])");
  std::vector<std::vector<std::string>> values;
  std::vector<std::string> tmp;

  auto elementCb = [&](const std::string &value) {
    tmp.push_back(value);
    return true;
  };

  auto innerArrayCb = [&]() {
    values.push_back(tmp);
    tmp.clear();
    return true;
  };

  auto value_parser = SJParser::
      makeArrayParser<SJParser::
                          ArrayParser<SJParser::ValueParser<std::string>>>(
          {{elementCb, innerArrayCb}});

  SJParser::Parser parser(value_parser);

  ASSERT_TRUE(parser.parse(buf));
  ASSERT_TRUE(parser.finish());

  ASSERT_EQ(2, values.size());
  ASSERT_EQ(2, values[0].size());
  ASSERT_EQ(2, values[1].size());
  ASSERT_EQ("value", values[0][0]);
  ASSERT_EQ("value2", values[0][1]);
  ASSERT_EQ("value3", values[1][0]);
  ASSERT_EQ("value4", values[1][1]);
}

struct ObjectStruct {
  std::string field1;
  int64_t field2;
};

TEST(Parser, arrayOfObjects) {
  std::string buf(
      R"([{"key": "value", "key2": 10}, {"key": "value2", "key2": 20}])");
  std::vector<ObjectStruct> values;

  using ParserType = SJParser::ObjectParser<SJParser::ValueParser<std::string>,
                                            SJParser::ValueParser<int64_t>>;

  auto objectCb = [&](ParserType &parser) {
    values.push_back({parser.get<0>().get(), parser.get<1>().get()});
    return true;
  };

  auto value_parser =
      SJParser::makeArrayParser<ParserType>({{{{"key"}, {"key2"}}, objectCb}});

  SJParser::Parser parser(value_parser);

  ASSERT_TRUE(parser.parse(buf));
  ASSERT_TRUE(parser.finish());

  ASSERT_EQ(2, values.size());
  ASSERT_EQ("value", values[0].field1);
  ASSERT_EQ(10, values[0].field2);
  ASSERT_EQ("value2", values[1].field1);
  ASSERT_EQ(20, values[1].field2);
}

struct ObjectWArrayStruct {
  std::string field1;
  int64_t field2;
  std::vector<std::string> array;
};

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

  using ParserType = SJParser::
      ObjectParser<SJParser::ValueParser<std::string>,
                   SJParser::ValueParser<int64_t>,
                   SJParser::ArrayParser<SJParser::ValueParser<std::string>>>;

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

  auto value_parser = SJParser::
      makeObjectParser<SJParser::ValueParser<std::string>,
                       SJParser::ValueParser<int64_t>,
                       SJParser::
                           ArrayParser<SJParser::ValueParser<std::string>>>(
          {{{"key"}, {"key2"}, {"key3", {{arrayEltCb}}}}, objectCb});
  SJParser::Parser parser(value_parser);

  ASSERT_TRUE(parser.parse(buf));
  ASSERT_TRUE(parser.finish());

  ASSERT_EQ("value", object.field1);
  ASSERT_EQ(10, object.field2);
  ASSERT_EQ(3, object.array.size());
  ASSERT_EQ("elt1", object.array[0]);
  ASSERT_EQ("elt2", object.array[1]);
  ASSERT_EQ("elt3", object.array[2]);
}
