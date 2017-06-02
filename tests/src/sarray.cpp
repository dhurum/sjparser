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

TEST(SArray, Empty) {
  std::string buf(R"([])");

  Parser<SArray<Value<bool>>> parser;

  ASSERT_TRUE(parser.parse(buf));
  ASSERT_TRUE(parser.finish());

  ASSERT_EQ(0, parser.parser().get().size());

  ASSERT_TRUE(parser.parser().isSet());
}

TEST(SArray, SArrayOfBooleans) {
  std::string buf(R"([true, false])");

  Parser<SArray<Value<bool>>> parser;

  ASSERT_TRUE(parser.parse(buf));
  ASSERT_TRUE(parser.finish());

  ASSERT_EQ(2, parser.parser().get().size());
  ASSERT_EQ(true, parser.parser().get()[0]);
  ASSERT_EQ(false, parser.parser().get()[1]);

  ASSERT_TRUE(parser.parser().isSet());
}

TEST(SArray, SArrayOfIntegers) {
  std::string buf(R"([10, 11])");

  Parser<SArray<Value<int64_t>>> parser;

  ASSERT_TRUE(parser.parse(buf));
  ASSERT_TRUE(parser.finish());

  ASSERT_EQ(2, parser.parser().get().size());
  ASSERT_EQ(10, parser.parser().get()[0]);
  ASSERT_EQ(11, parser.parser().get()[1]);

  ASSERT_TRUE(parser.parser().isSet());
}

TEST(SArray, SArrayOfDoubles) {
  std::string buf(R"([10.5, 11.2])");

  Parser<SArray<Value<double>>> parser;

  ASSERT_TRUE(parser.parse(buf));
  ASSERT_TRUE(parser.finish());

  ASSERT_EQ(2, parser.parser().get().size());
  ASSERT_EQ(10.5, parser.parser().get()[0]);
  ASSERT_EQ(11.2, parser.parser().get()[1]);

  ASSERT_TRUE(parser.parser().isSet());
}

TEST(SArray, SArrayOfStrings) {
  std::string buf(R"(["value1", "value2"])");

  Parser<SArray<Value<std::string>>> parser;

  ASSERT_TRUE(parser.parse(buf));
  ASSERT_TRUE(parser.finish());

  ASSERT_EQ(2, parser.parser().get().size());
  ASSERT_EQ("value1", parser.parser().get()[0]);
  ASSERT_EQ("value2", parser.parser().get()[1]);

  ASSERT_TRUE(parser.parser().isSet());
}

// FIXME
TEST(SArray, DISABLED_SArrayWithUnexpectedBoolean) {
  std::string buf(R"([true])");

  Parser<SArray<Value<std::string>>> parser;

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

// FIXME
TEST(SArray, DISABLED_SArrayWithUnexpectedInteger) {
  std::string buf(R"([10])");

  Parser<SArray<Value<bool>>> parser;

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

// FIXME
TEST(SArray, DISABLED_SArrayWithUnexpectedDouble) {
  std::string buf(R"([10.5])");

  Parser<SArray<Value<bool>>> parser;

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

// FIXME
TEST(SArray, DISABLED_SArrayWithUnexpectedString) {
  std::string buf(R"(["value"])");

  Parser<SArray<Value<bool>>> parser;

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

TEST(SArray, SArrayWithElementCallbackError) {
  std::string buf(R"([true, false])");

  auto elementCb = [&](const bool &) {
    return false;
  };

  Parser<SArray<Value<bool>>> parser(elementCb);

  ASSERT_FALSE(parser.parse(buf));
  ASSERT_FALSE(parser.parser().isSet());

  ASSERT_EQ(
      R"(parse error: client cancelled parse via callback return value
                                   [true, false]
                     (right here) ------^

)",
      parser.getError());
}

TEST(SArray, SArrayWithCallback) {
  std::string buf(R"([true, false])");
  std::vector<bool> values;

  auto arrayCb = [&](const std::vector<bool> &value) {
    values = value;
    return true;
  };

  Parser<SArray<Value<bool>>> parser(arrayCb);

  ASSERT_TRUE(parser.parse(buf));
  ASSERT_TRUE(parser.finish());

  ASSERT_EQ(2, parser.parser().get().size());
  ASSERT_EQ(true, parser.parser().get()[0]);
  ASSERT_EQ(false, parser.parser().get()[1]);

  ASSERT_EQ(2, values.size());
  ASSERT_EQ(true, values[0]);
  ASSERT_EQ(false, values[1]);

  ASSERT_TRUE(parser.parser().isSet());
}

TEST(SArray, SArrayWithCallbackError) {
  std::string buf(R"([true, false])");

  auto arrayCb = [&](const std::vector<bool> &) {
    return false;
  };

  Parser<SArray<Value<bool>>> parser(arrayCb);

  ASSERT_FALSE(parser.parse(buf));
  ASSERT_TRUE(parser.parser().isSet());

  ASSERT_EQ(
      R"(parse error: client cancelled parse via callback return value
                           [true, false]
                     (right here) ------^

)",
      parser.getError());
}

TEST(SArray, SArrayWithArgsStruct) {
  std::string buf(R"([true, false])");
  std::vector<bool> values;

  auto elementCb = [&](const bool &value) {
    values.push_back(value);
    return true;
  };

  using SArrayParser = SArray<Value<bool>>;

  SArrayParser::Args array_args = {elementCb};

  Parser<SArrayParser> parser(array_args);

  ASSERT_TRUE(parser.parse(buf));
  ASSERT_TRUE(parser.finish());

  ASSERT_EQ(2, parser.parser().get().size());
  ASSERT_EQ(true, parser.parser().get()[0]);
  ASSERT_EQ(false, parser.parser().get()[1]);

  ASSERT_EQ(2, values.size());
  ASSERT_EQ(true, values[0]);
  ASSERT_EQ(false, values[1]);

  ASSERT_TRUE(parser.parser().isSet());
}

TEST(SArray, SArrayWithUnexpectedSObject) {
  std::string buf(
      R"([{"key2": "value"}])");

  Parser<SArray<SObject<Value<std::string>>>> parser("key");

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

TEST(SArray, SArrayOfSCustomObjects) {
  std::string buf(
      R"([{"key": "value", "key2": 10}, {"key": "value2", "key2": 20}])");

  struct ObjectStruct {
    std::string field1;
    int64_t field2;
  };

  using ObjectParser =
      SObject<ObjectStruct, Value<std::string>, Value<int64_t>>;

  auto objectCb = [&](ObjectParser &parser, ObjectStruct &value) {
    value = {parser.get<0>().pop(), parser.get<1>().pop()};
    return true;
  };

  Parser<SArray<ObjectParser>> parser({{"key", "key2"}, objectCb});

  ASSERT_TRUE(parser.parse(buf));
  ASSERT_TRUE(parser.finish());

  ASSERT_EQ(2, parser.parser().get().size());
  ASSERT_EQ("value", parser.parser().get()[0].field1);
  ASSERT_EQ(10, parser.parser().get()[0].field2);
  ASSERT_EQ("value2", parser.parser().get()[1].field1);
  ASSERT_EQ(20, parser.parser().get()[1].field2);
}

TEST(SArray, SArrayOfSAutoObjects) {
  std::string buf(
      R"([{"key": "value", "key2": 10}, {"key": "value2", "key2": 20}])");

  using ObjectParser = SObject<Value<std::string>, Value<int64_t>>;

  Parser<SArray<ObjectParser>> parser({"key", "key2"});

  ASSERT_TRUE(parser.parse(buf));
  ASSERT_TRUE(parser.finish());

  ASSERT_EQ(2, parser.parser().get().size());
  ASSERT_EQ("value", std::get<0>(parser.parser().get()[0]));
  ASSERT_EQ(10, std::get<1>(parser.parser().get()[0]));
  ASSERT_EQ("value2", std::get<0>(parser.parser().get()[1]));
  ASSERT_EQ(20, std::get<1>(parser.parser().get()[1]));
}

TEST(SArray, SArrayOfSArrays) {
  std::string buf(R"([[true, true], [false, false]])");

  Parser<SArray<SArray<Value<bool>>>> parser;

  ASSERT_TRUE(parser.parse(buf));
  ASSERT_TRUE(parser.finish());

  ASSERT_EQ(2, parser.parser().get().size());
  ASSERT_EQ(2, parser.parser().get()[0].size());
  ASSERT_EQ(true, parser.parser().get()[0][0]);
  ASSERT_EQ(true, parser.parser().get()[0][1]);

  ASSERT_EQ(2, parser.parser().get()[1].size());
  ASSERT_EQ(false, parser.parser().get()[1][0]);
  ASSERT_EQ(false, parser.parser().get()[1][1]);
}
