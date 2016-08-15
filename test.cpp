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

class TestObject : public SJParser::ObjectParser {
 public:
  TestObject() {
    addField("key", &_key_parser);
    addField("key2", &_key_2_parser);
  }

  SJParser::ValueParser<std::string> _key_parser;
  SJParser::ValueParser<int64_t> _key_2_parser;
};

TEST(Parser, object) {
  std::string buf(R"({"key": "value", "key2": 10})");
  auto value_parser = std::make_shared<TestObject>();

  SJParser::Parser parser(value_parser);

  ASSERT_TRUE(parser.parse(buf));
  ASSERT_TRUE(parser.finish());

  ASSERT_EQ("value", value_parser->_key_parser.get());
  ASSERT_EQ(10, value_parser->_key_2_parser.get());
}

TEST(Parser, emptyObject) {
  std::string buf(R"({})");
  auto value_parser = std::make_shared<TestObject>();

  SJParser::Parser parser(value_parser);

  ASSERT_TRUE(parser.parse(buf));
  ASSERT_TRUE(parser.finish());
}

class TestArray : public SJParser::ArrayParser {
 public:
  TestArray()
      : _elements_parser([this](const std::string &value) {
          this->_values.push_back(value);
          return true;
        }) {
    setElementsParser(&_elements_parser);
  }

  std::vector<std::string> _values;
  SJParser::ValueParser<std::string> _elements_parser;
};

TEST(Parser, array) {
  std::string buf(R"(["value", "value2"])");
  auto value_parser = std::make_shared<TestArray>();

  SJParser::Parser parser(value_parser);

  ASSERT_TRUE(parser.parse(buf));
  ASSERT_TRUE(parser.finish());

  ASSERT_EQ(2, value_parser->_values.size());
  ASSERT_EQ("value", value_parser->_values[0]);
  ASSERT_EQ("value2", value_parser->_values[1]);
}

TEST(Parser, emptyArray) {
  std::string buf(R"([])");
  auto value_parser = std::make_shared<TestArray>();

  SJParser::Parser parser(value_parser);

  ASSERT_TRUE(parser.parse(buf));
  ASSERT_TRUE(parser.finish());

  ASSERT_EQ(0, value_parser->_values.size());
}

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

struct TestStruct {
  std::string field1;
  int64_t field2;
};

class TestObjectCreator : public SJParser::ObjectParser {
 public:
  TestObjectCreator(std::vector<TestStruct> &result) : _result(result) {
    addField("key", &_key_parser);
    addField("key2", &_key_2_parser);
  }

  virtual bool finish() override {
    _result.push_back({_key_parser.get(), _key_2_parser.get()});
    return true;
  }

  SJParser::ValueParser<std::string> _key_parser;
  SJParser::ValueParser<int64_t> _key_2_parser;
  std::vector<TestStruct> &_result;
};

class TestObjectsArray : public SJParser::ArrayParser {
 public:
  TestObjectsArray() : _elements_parser(_values) {
    setElementsParser(&_elements_parser);
  }

  std::vector<TestStruct> _values;
  TestObjectCreator _elements_parser;
};

TEST(Parser, objectsArray) {
  std::string buf(
      R"([{"key": "value", "key2": 10}, {"key": "value2", "key2": 20}])");
  auto value_parser = std::make_shared<TestObjectsArray>();

  SJParser::Parser parser(value_parser);

  ASSERT_TRUE(parser.parse(buf));
  ASSERT_TRUE(parser.finish());

  ASSERT_EQ(2, value_parser->_values.size());
  ASSERT_EQ("value", value_parser->_values[0].field1);
  ASSERT_EQ(10, value_parser->_values[0].field2);
  ASSERT_EQ("value2", value_parser->_values[1].field1);
  ASSERT_EQ(20, value_parser->_values[1].field2);
}
