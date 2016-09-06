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
          {"key", "key2"});

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
          {"key", "key2"});

  SJParser::Parser parser(value_parser);

  ASSERT_TRUE(parser.parse(buf));
  ASSERT_TRUE(parser.finish());
}

TEST(Parser, array) {
  std::string buf(R"(["value", "value2"])");

  auto value_parser = std::make_shared<SJParser::ArrayParser<SJParser::ValueParser<std::string>>>();

  SJParser::Parser parser(value_parser);

  ASSERT_TRUE(parser.parse(buf));
  ASSERT_TRUE(parser.finish());

  ASSERT_EQ(2, value_parser->get().size());
  ASSERT_EQ("value", value_parser->get()[0]);
  ASSERT_EQ("value2", value_parser->get()[1]);
}

TEST(Parser, emptyArray) {
  std::string buf(R"([])");

  auto value_parser = std::make_shared<SJParser::ArrayParser<SJParser::ValueParser<std::string>>>();

  SJParser::Parser parser(value_parser);

  ASSERT_TRUE(parser.parse(buf));
  ASSERT_TRUE(parser.finish());

  ASSERT_EQ(0, value_parser->get().size());
}

TEST(Parser, arrayWithCallback) {
  std::string buf(R"(["value", "value2"])");
  std::vector<std::string> values;

  auto value_parser = std::make_shared<SJParser::ArrayParser<SJParser::ValueParser<std::string>>>(
      [&](const std::string &value) { values.push_back(value); return true; });

  SJParser::Parser parser(value_parser);

  ASSERT_TRUE(parser.parse(buf));
  ASSERT_TRUE(parser.finish());

  ASSERT_EQ(2, values.size());
  ASSERT_EQ("value", values[0]);
  ASSERT_EQ("value2", values[1]);
}

/*
class TestInnerArray : public SJParser::ArrayParser {
 public:
  TestInnerArray(std::vector<std::vector<std::string>> &result)
      : _elements_parser([this](const std::string &value) {
          this->_values.push_back(value);
          return true;
        }),
        _result(result) {
    setElementsParser(&_elements_parser);
  }

  virtual bool finish() override {
    _result.push_back(_values);
    _values.clear();
    return true;
  }

  std::vector<std::string> _values;
  SJParser::ValueParser<std::string> _elements_parser;
  std::vector<std::vector<std::string>> &_result;
};

class TestOuterArray : public SJParser::ArrayParser {
 public:
  TestOuterArray() : _elements_parser(_values) {
    setElementsParser(&_elements_parser);
  }

  std::vector<std::vector<std::string>> _values;
  TestInnerArray _elements_parser;
};

TEST(Parser, insetArray) {
  std::string buf(R"([["value", "value2"], ["value3", "value4"]])");
  auto value_parser = std::make_shared<TestOuterArray>();

  SJParser::Parser parser(value_parser);

  ASSERT_TRUE(parser.parse(buf));
  ASSERT_TRUE(parser.finish());

  ASSERT_EQ(2, value_parser->_values.size());
  ASSERT_EQ(2, value_parser->_values[0].size());
  ASSERT_EQ(2, value_parser->_values[1].size());
  ASSERT_EQ("value", value_parser->_values[0][0]);
  ASSERT_EQ("value2", value_parser->_values[0][1]);
  ASSERT_EQ("value3", value_parser->_values[1][0]);
  ASSERT_EQ("value4", value_parser->_values[1][1]);
}
*/

struct TestStruct {
  std::string field1;
  int64_t field2;
};

TEST(Parser, arrayOfObjects) {
  std::string buf(
      R"([{"key": "value", "key2": 10}, {"key": "value2", "key2": 20}])");
  std::vector<TestStruct> values;

  using ParserType = SJParser::ObjectParser<SJParser::ValueParser<std::string>,
                                            SJParser::ValueParser<int64_t>>;

  auto value_parser = std::make_shared<SJParser::ArrayParser<ParserType>>(
      [&](ParserType &parser) {
        values.push_back({parser.get<0>().get(), parser.get<1>().get()});
        return true;
      }, std::initializer_list<SJParser::Param>{"key", "key2"});

  SJParser::Parser parser(value_parser);

  ASSERT_TRUE(parser.parse(buf));
  ASSERT_TRUE(parser.finish());

  ASSERT_EQ(2, values.size());
  ASSERT_EQ("value", values[0].field1);
  ASSERT_EQ(10, values[0].field2);
  ASSERT_EQ("value2", values[1].field1);
  ASSERT_EQ(20, values[1].field2);
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

  auto value_parser = SJParser::makeObjectParser<
      SJParser::ValueParser<std::string>, SJParser::ValueParser<int64_t>,
      SJParser::ObjectParser<SJParser::ValueParser<int64_t>,
                             SJParser::ValueParser<std::string>>,
      SJParser::ValueParser<bool>>(
      {"key", "key2", "key3", {"key", "key2"}, "key4"});

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

  auto value_parser = SJParser::makeObjectParser<
      SJParser::ObjectParser<SJParser::ValueParser<std::string>,
                             SJParser::ValueParser<int64_t>>,
      SJParser::ObjectParser<SJParser::ValueParser<int64_t>,
                             SJParser::ValueParser<std::string>>,
      SJParser::ObjectParser<SJParser::ValueParser<bool>>>(
      {"key", {"key", "key2"}, "key2", {"key", "key2"}, "key3", {"key"}});

  SJParser::Parser parser(value_parser);

  ASSERT_TRUE(parser.parse(buf));
  ASSERT_TRUE(parser.finish());

  ASSERT_EQ("value", value_parser->get<0>().get<0>().get());
  ASSERT_EQ(10, value_parser->get<0>().get<1>().get());
  ASSERT_EQ(1, value_parser->get<1>().get<0>().get());
  ASSERT_EQ("value2", value_parser->get<1>().get<1>().get());
  ASSERT_EQ(true, value_parser->get<2>().get<0>().get());
}

//TODO: object with array and object with array of objects
