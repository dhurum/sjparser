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

TEST(SArray, Empty) {
  std::string buf(R"([])");

  Parser parser{SArray{Value<bool>{}}};

  ASSERT_NO_THROW(parser.parse(buf));
  ASSERT_NO_THROW(parser.finish());

  ASSERT_EQ(0, parser.parser().get().size());

  ASSERT_TRUE(parser.parser().isSet());
}

TEST(SArray, Null) {
  std::string buf(R"(null)");

  Parser parser{SArray{Value<bool>{}}};

  ASSERT_NO_THROW(parser.parse(buf));
  ASSERT_NO_THROW(parser.finish());

  ASSERT_FALSE(parser.parser().isSet());
}

TEST(SArray, Reset) {
  std::string buf(R"([true])");

  Parser parser{SArray{Value<bool>{}}};

  ASSERT_NO_THROW(parser.parse(buf));
  ASSERT_NO_THROW(parser.finish());

  ASSERT_EQ(1, parser.parser().get().size());
  ASSERT_EQ(true, parser.parser().get()[0]);

  buf = R"(null)";

  ASSERT_NO_THROW(parser.parse(buf));
  ASSERT_NO_THROW(parser.finish());

  ASSERT_FALSE(parser.parser().isSet());
}

TEST(SArray, SArrayOfBooleans) {
  std::string buf(R"([true, false])");

  Parser parser{SArray{Value<bool>{}}};

  ASSERT_NO_THROW(parser.parse(buf));
  ASSERT_NO_THROW(parser.finish());

  ASSERT_EQ(2, parser.parser().get().size());
  ASSERT_EQ(true, parser.parser().get()[0]);
  ASSERT_EQ(false, parser.parser().get()[1]);

  ASSERT_TRUE(parser.parser().isSet());
}

TEST(SArray, SArrayOfIntegers) {
  std::string buf(R"([10, 11])");

  Parser parser{SArray{Value<int64_t>{}}};

  ASSERT_NO_THROW(parser.parse(buf));
  ASSERT_NO_THROW(parser.finish());

  ASSERT_EQ(2, parser.parser().get().size());
  ASSERT_EQ(10, parser.parser().get()[0]);
  ASSERT_EQ(11, parser.parser().get()[1]);

  ASSERT_TRUE(parser.parser().isSet());
}

TEST(SArray, SArrayOfDoubles) {
  std::string buf(R"([10.5, 11.2])");

  Parser parser{SArray{Value<double>{}}};

  ASSERT_NO_THROW(parser.parse(buf));
  ASSERT_NO_THROW(parser.finish());

  ASSERT_EQ(2, parser.parser().get().size());
  ASSERT_EQ(10.5, parser.parser().get()[0]);
  ASSERT_EQ(11.2, parser.parser().get()[1]);

  ASSERT_TRUE(parser.parser().isSet());
}

TEST(SArray, SArrayOfStrings) {
  std::string buf(R"(["value1", "value2"])");

  Parser parser{SArray{Value<std::string>{}}};

  ASSERT_NO_THROW(parser.parse(buf));
  ASSERT_NO_THROW(parser.finish());

  ASSERT_EQ(2, parser.parser().get().size());
  ASSERT_EQ("value1", parser.parser().get()[0]);
  ASSERT_EQ("value2", parser.parser().get()[1]);

  ASSERT_TRUE(parser.parser().isSet());
}

TEST(SArray, SArrayWithNull) {
  std::string buf(R"([null])");

  Parser parser{SArray{Value<bool>{}}};

  ASSERT_NO_THROW(parser.parse(buf));
  ASSERT_NO_THROW(parser.finish());

  ASSERT_EQ(0, parser.parser().get().size());

  ASSERT_TRUE(parser.parser().isSet());
}

TEST(SArray, SArrayWithNullAndValuse) {
  std::string buf(R"([null, true, null, false])");

  Parser parser{SArray{Value<bool>{}}};

  ASSERT_NO_THROW(parser.parse(buf));
  ASSERT_NO_THROW(parser.finish());

  ASSERT_EQ(2, parser.parser().get().size());
  ASSERT_EQ(true, parser.parser().get()[0]);
  ASSERT_EQ(false, parser.parser().get()[1]);

  ASSERT_TRUE(parser.parser().isSet());
}

TEST(SArray, UnexpectedBoolean) {
  std::string buf(R"(true)");

  Parser parser{SArray{Value<bool>{}}};

  try {
    parser.parse(buf);
    FAIL() << "No exception thrown";
  } catch (ParsingError &e) {
    ASSERT_FALSE(parser.parser().isSet());
    ASSERT_EQ("Unexpected token boolean", e.sjparserError());

    ASSERT_EQ(
        R"(parse error: client cancelled parse via callback return value
                                    true
                     (right here) ------^
)",
        e.parserError());
  } catch (...) {
    FAIL() << "Invalid exception thrown";
  }
}

TEST(SArray, UnexpectedInteger) {
  std::string buf(R"(10)");

  Parser parser{SArray{Value<int64_t>{}}};

  ASSERT_NO_THROW(parser.parse(buf));
  try {
    parser.finish();
    FAIL() << "No exception thrown";
  } catch (ParsingError &e) {
    ASSERT_FALSE(parser.parser().isSet());
    ASSERT_EQ("Unexpected token integer", e.sjparserError());

    ASSERT_EQ(
        R"(parse error: client cancelled parse via callback return value
                                        10
                     (right here) ------^
)",
        e.parserError());
  } catch (...) {
    FAIL() << "Invalid exception thrown";
  }
}

TEST(SArray, UnexpectedDouble) {
  std::string buf(R"(10.5)");

  Parser parser{SArray{Value<double>{}}};

  ASSERT_NO_THROW(parser.parse(buf));
  try {
    parser.finish();
    FAIL() << "No exception thrown";
  } catch (ParsingError &e) {
    ASSERT_FALSE(parser.parser().isSet());
    ASSERT_EQ("Unexpected token double", e.sjparserError());

    ASSERT_EQ(
        R"(parse error: client cancelled parse via callback return value
                                        10.5
                     (right here) ------^
)",
        e.parserError());
  } catch (...) {
    FAIL() << "Invalid exception thrown";
  }
}

TEST(SArray, UnexpectedString) {
  std::string buf(R"("value")");

  Parser parser{SArray{Value<std::string>{}}};

  try {
    parser.parse(buf);
    FAIL() << "No exception thrown";
  } catch (ParsingError &e) {
    ASSERT_FALSE(parser.parser().isSet());
    ASSERT_EQ("Unexpected token string", e.sjparserError());

    ASSERT_EQ(
        R"(parse error: client cancelled parse via callback return value
                                 "value"
                     (right here) ------^
)",
        e.parserError());
  } catch (...) {
    FAIL() << "Invalid exception thrown";
  }
}

TEST(SArray, SArrayWithUnexpectedBoolean) {
  std::string buf(R"([true])");

  Parser parser{SArray{Value<std::string>{}}};

  try {
    parser.parse(buf);
    FAIL() << "No exception thrown";
  } catch (ParsingError &e) {
    ASSERT_FALSE(parser.parser().isSet());
    ASSERT_EQ("Unexpected token boolean", e.sjparserError());

    ASSERT_EQ(
        R"(parse error: client cancelled parse via callback return value
                                   [true]
                     (right here) ------^
)",
        e.parserError());
  } catch (...) {
    FAIL() << "Invalid exception thrown";
  }
}

TEST(SArray, SArrayWithUnexpectedInteger) {
  std::string buf(R"([10])");

  Parser parser{SArray{Value<bool>{}}};

  try {
    parser.parse(buf);
    FAIL() << "No exception thrown";
  } catch (ParsingError &e) {
    ASSERT_FALSE(parser.parser().isSet());
    ASSERT_EQ("Unexpected token integer", e.sjparserError());

    ASSERT_EQ(
        R"(parse error: client cancelled parse via callback return value
                                     [10]
                     (right here) ------^
)",
        e.parserError());
  } catch (...) {
    FAIL() << "Invalid exception thrown";
  }
}

TEST(SArray, SArrayWithUnexpectedDouble) {
  std::string buf(R"([10.5])");

  Parser parser{SArray{Value<bool>{}}};

  try {
    parser.parse(buf);
    FAIL() << "No exception thrown";
  } catch (ParsingError &e) {
    ASSERT_FALSE(parser.parser().isSet());
    ASSERT_EQ("Unexpected token double", e.sjparserError());

    ASSERT_EQ(
        R"(parse error: client cancelled parse via callback return value
                                   [10.5]
                     (right here) ------^
)",
        e.parserError());
  } catch (...) {
    FAIL() << "Invalid exception thrown";
  }
}

TEST(SArray, SArrayWithUnexpectedString) {
  std::string buf(R"(["value"])");

  Parser parser{SArray{Value<bool>{}}};

  try {
    parser.parse(buf);
    FAIL() << "No exception thrown";
  } catch (ParsingError &e) {
    ASSERT_FALSE(parser.parser().isSet());
    ASSERT_EQ("Unexpected token string", e.sjparserError());

    ASSERT_EQ(
        R"(parse error: client cancelled parse via callback return value
                                ["value"]
                     (right here) ------^
)",
        e.parserError());
  } catch (...) {
    FAIL() << "Invalid exception thrown";
  }
}

TEST(SArray, SArrayWithElementCallbackError) {
  std::string buf(R"([true, false])");

  auto elementCb = [&](const bool &) { return false; };

  Parser parser{SArray{Value<bool>{elementCb}}};

  try {
    parser.parse(buf);
    FAIL() << "No exception thrown";
  } catch (ParsingError &e) {
    ASSERT_FALSE(parser.parser().isSet());

    ASSERT_EQ("Callback returned false", e.sjparserError());
    ASSERT_EQ(
        R"(parse error: client cancelled parse via callback return value
                                   [true, false]
                     (right here) ------^
)",
        e.parserError());
  } catch (...) {
    FAIL() << "Invalid exception thrown";
  }
}

TEST(SArray, SArrayWithCallback) {
  std::string buf(R"([true, false])");
  std::vector<bool> values;

  auto arrayCb = [&](const std::vector<bool> &value) {
    values = value;
    return true;
  };

  Parser parser{SArray{Value<bool>{}, arrayCb}};

  ASSERT_NO_THROW(parser.parse(buf));
  ASSERT_NO_THROW(parser.finish());

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

  auto arrayCb = [&](const std::vector<bool> &) { return false; };

  Parser parser{SArray{Value<bool>{}}};

  parser.parser().setFinishCallback(arrayCb);

  try {
    parser.parse(buf);
    FAIL() << "No exception thrown";
  } catch (ParsingError &e) {
    ASSERT_TRUE(parser.parser().isSet());

    ASSERT_EQ("Callback returned false", e.sjparserError());
    ASSERT_EQ(
        R"(parse error: client cancelled parse via callback return value
                           [true, false]
                     (right here) ------^
)",
        e.parserError());
  } catch (...) {
    FAIL() << "Invalid exception thrown";
  }
}

TEST(SArray, SArrayOfSCustomObjects) {
  std::string buf(
      R"([{"key": "value", "key2": 10}, {"key": "value2", "key2": 20}])");

  struct ObjectStruct {
    std::string member1;
    int64_t member2;
  };

  Parser parser{
      SArray{SCustomObject{TypeHolder<ObjectStruct>{},
                           std::tuple{Member{"key", Value<std::string>{}},
                                      Member{"key2", Value<int64_t>{}}}}}};

  auto objectCb = [&](decltype(parser)::ParserType::ParserType &parser,
                      ObjectStruct &value) {
    value = {parser.pop<0>(), parser.pop<1>()};
    return true;
  };

  parser.parser().parser().setFinishCallback(objectCb);

  ASSERT_NO_THROW(parser.parse(buf));
  ASSERT_NO_THROW(parser.finish());

  ASSERT_EQ(2, parser.parser().get().size());
  ASSERT_EQ("value", parser.parser().get()[0].member1);
  ASSERT_EQ(10, parser.parser().get()[0].member2);
  ASSERT_EQ("value2", parser.parser().get()[1].member1);
  ASSERT_EQ(20, parser.parser().get()[1].member2);
}

TEST(SArray, SArrayOfSAutoObjects) {
  std::string buf(
      R"([{"key": "value", "key2": 10}, {"key": "value2", "key2": 20}])");

  Parser parser{SArray{SAutoObject{std::tuple{
      Member{"key", Value<std::string>{}}, Member{"key2", Value<int64_t>{}}}}}};

  ASSERT_NO_THROW(parser.parse(buf));
  ASSERT_NO_THROW(parser.finish());

  ASSERT_EQ(2, parser.parser().get().size());
  ASSERT_EQ("value", std::get<0>(parser.parser().get()[0]));
  ASSERT_EQ(10, std::get<1>(parser.parser().get()[0]));
  ASSERT_EQ("value2", std::get<0>(parser.parser().get()[1]));
  ASSERT_EQ(20, std::get<1>(parser.parser().get()[1]));
}

TEST(SArray, SArrayOfSAutoObjectsWithDefaultValues) {
  std::string buf(
      R"([{"key": "value"}, {"key2": 20}])");

  Parser parser{
      SArray{SAutoObject{std::tuple{Member{"key", Value<std::string>{}},
                                    Member{"key2", Value<int64_t>{}}},
                         {"test", 10}}}};

  ASSERT_NO_THROW(parser.parse(buf));
  ASSERT_NO_THROW(parser.finish());

  ASSERT_EQ(2, parser.parser().get().size());
  ASSERT_EQ("value", std::get<0>(parser.parser().get()[0]));
  ASSERT_EQ(10, std::get<1>(parser.parser().get()[0]));
  ASSERT_EQ("test", std::get<0>(parser.parser().get()[1]));
  ASSERT_EQ(20, std::get<1>(parser.parser().get()[1]));
}

TEST(SArray, SArrayOfSArrays) {
  std::string buf(R"([[true, true], [false, false]])");

  Parser parser{SArray{SArray{Value<bool>{}}}};

  ASSERT_NO_THROW(parser.parse(buf));
  ASSERT_NO_THROW(parser.finish());

  ASSERT_EQ(2, parser.parser().get().size());
  ASSERT_EQ(2, parser.parser().get()[0].size());
  ASSERT_EQ(true, parser.parser().get()[0][0]);
  ASSERT_EQ(true, parser.parser().get()[0][1]);

  ASSERT_EQ(2, parser.parser().get()[1].size());
  ASSERT_EQ(false, parser.parser().get()[1][0]);
  ASSERT_EQ(false, parser.parser().get()[1][1]);
}

TEST(SArray, SArrayOfStandaloneSUnions) {
  std::string buf(
      R"([{"type": "str", "key": "value"}, {"type": "int", "key": 10}])");

  Parser parser{SArray{SUnion{
      TypeHolder<std::string>{}, "type",
      std::tuple{Member{"str", SAutoObject{std::tuple{
                                   Member{"key", Value<std::string>{}}}}},
                 Member{"int", SAutoObject{std::tuple{
                                   Member{"key", Value<int64_t>{}}}}}}}}};

  ASSERT_NO_THROW(parser.parse(buf));
  ASSERT_NO_THROW(parser.finish());

  ASSERT_EQ(2, parser.parser().get().size());
  ASSERT_EQ("value", std::get<0>(std::get<0>(parser.parser().get()[0])));
  ASSERT_EQ(10, std::get<0>(std::get<1>(parser.parser().get()[1])));
}

TEST(SArray, SArrayOfEmbeddedSUnions) {
  std::string buf(
      R"([{"id": 1, "type": "str", "key": "value"},
          {"id": 2, "type": "int", "key": 10}])");

  Parser parser{SArray{SAutoObject{std::tuple{
      Member{"id", Value<int64_t>{}},
      Member{"type",
             SUnion{TypeHolder<std::string>{},
                    std::tuple{
                        Member{"str", SAutoObject{std::tuple{Member{
                                          "key", Value<std::string>{}}}}},
                        Member{"int", SAutoObject{std::tuple{Member{
                                          "key", Value<int64_t>{}}}}}}}}}}}};

  ASSERT_NO_THROW(parser.parse(buf));
  ASSERT_NO_THROW(parser.finish());

  ASSERT_EQ(2, parser.parser().get().size());
  ASSERT_EQ(1, std::get<0>(parser.parser().get()[0]));
  ASSERT_EQ(2, std::get<0>(parser.parser().get()[1]));
  ASSERT_EQ("value",
            std::get<0>(std::get<0>(std::get<1>(parser.parser().get()[0]))));
  ASSERT_EQ(10,
            std::get<0>(std::get<1>(std::get<1>(parser.parser().get()[1]))));
}

TEST(SArray, Move) {
  std::string buf(R"(["value1", "value2"])");

  Parser parser{SArray{Value<std::string>{}}};

  ASSERT_NO_THROW(parser.parse(buf));
  ASSERT_NO_THROW(parser.finish());

  auto value = parser.parser().pop();
  ASSERT_FALSE(parser.parser().isSet());

  ASSERT_EQ(2, value.size());
  ASSERT_EQ("value1", value[0]);
  ASSERT_EQ("value2", value[1]);
}

TEST(SArray, SArrayWithParserReference) {
  std::string buf(R"([[13, 15, 16]])");

  SArray inner_sarray{Value<int64_t>{}};

  Parser parser{SArray{inner_sarray}};

  ASSERT_NO_THROW(parser.parse(buf));
  ASSERT_NO_THROW(parser.finish());

  ASSERT_EQ(1, parser.parser().get().size());
  ASSERT_EQ(3, parser.parser().get()[0].size());
  ASSERT_EQ(13, parser.parser().get()[0][0]);
  ASSERT_EQ(15, parser.parser().get()[0][1]);
  ASSERT_EQ(16, parser.parser().get()[0][2]);

  ASSERT_EQ(&(parser.parser().parser()), &inner_sarray);
}
