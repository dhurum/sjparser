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

TEST(Value, Boolean) {
  std::string buf(R"(true)");

  Parser<Value<bool>> parser;

  ASSERT_FALSE(parser.parser().isSet());

  ASSERT_NO_THROW(parser.parse(buf));
  ASSERT_NO_THROW(parser.finish());

  ASSERT_TRUE(parser.parser().isSet());
  ASSERT_EQ(true, parser.parser().get());

  ASSERT_TRUE(parser.parser().isSet());
  ASSERT_EQ(true, parser.parser().pop());
  ASSERT_FALSE(parser.parser().isSet());
}

TEST(Value, Integer) {
  std::string buf(R"(10)");

  Parser<Value<int64_t>> parser;

  ASSERT_FALSE(parser.parser().isSet());

  ASSERT_NO_THROW(parser.parse(buf));
  ASSERT_NO_THROW(parser.finish());

  ASSERT_TRUE(parser.parser().isSet());
  ASSERT_EQ(10, parser.parser().get());

  ASSERT_TRUE(parser.parser().isSet());
  ASSERT_EQ(10, parser.parser().pop());
  ASSERT_FALSE(parser.parser().isSet());
}

TEST(Value, Double) {
  std::string buf(R"(1.3)");

  Parser<Value<double>> parser;

  ASSERT_FALSE(parser.parser().isSet());

  ASSERT_NO_THROW(parser.parse(buf));
  ASSERT_NO_THROW(parser.finish());

  ASSERT_TRUE(parser.parser().isSet());
  ASSERT_EQ(1.3, parser.parser().get());

  ASSERT_TRUE(parser.parser().isSet());
  ASSERT_EQ(1.3, parser.parser().pop());
  ASSERT_FALSE(parser.parser().isSet());
}

TEST(Value, String) {
  std::string buf(R"("value")");

  Parser<Value<std::string>> parser;

  ASSERT_FALSE(parser.parser().isSet());

  ASSERT_NO_THROW(parser.parse(buf));
  ASSERT_NO_THROW(parser.finish());

  ASSERT_TRUE(parser.parser().isSet());
  ASSERT_EQ("value", parser.parser().get());

  ASSERT_TRUE(parser.parser().isSet());
  ASSERT_EQ("value", parser.parser().pop());
  ASSERT_FALSE(parser.parser().isSet());
}

TEST(Value, Null) {
  std::string buf(R"(null)");

  Parser<Value<bool>> parser;

  ASSERT_NO_THROW(parser.parse(buf));
  ASSERT_NO_THROW(parser.finish());

  ASSERT_FALSE(parser.parser().isSet());
}

TEST(Value, UnexpectedBoolean) {
  std::string buf(R"(true)");

  Parser<Value<std::string>> parser;

  try {
    parser.parse(buf);
    FAIL() << "No exception thrown";
  } catch (ParseError &e) {
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

TEST(Value, UnexpectedString) {
  std::string buf(R"("error")");

  Parser<Value<bool>> parser;

  try {
    parser.parse(buf);
    FAIL() << "No exception thrown";
  } catch (ParseError &e) {
    ASSERT_FALSE(parser.parser().isSet());
    ASSERT_EQ("Unexpected token string", e.sjparserError());

    ASSERT_EQ(
        R"(parse error: client cancelled parse via callback return value
                                 "error"
                     (right here) ------^
)",
        e.parserError());
  } catch (...) {
    FAIL() << "Invalid exception thrown";
  }
}

TEST(Value, UnexpectedInteger) {
  std::string buf(R"(10)");

  Parser<Value<bool>> parser;

  ASSERT_NO_THROW(parser.parse(buf));
  try {
    parser.finish();
    FAIL() << "No exception thrown";
  } catch (ParseError &e) {
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

TEST(Value, UnexpectedDouble) {
  std::string buf(R"(10.5)");

  Parser<Value<bool>> parser;

  ASSERT_NO_THROW(parser.parse(buf));
  try {
    parser.finish();
    FAIL() << "No exception thrown";
  } catch (ParseError &e) {
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

TEST(Value, UnexpectedMapStart) {
  std::string buf(R"({)");

  Parser<Value<bool>> parser;

  try {
    parser.parse(buf);
    FAIL() << "No exception thrown";
  } catch (ParseError &e) {
    ASSERT_FALSE(parser.parser().isSet());
    ASSERT_EQ("Unexpected token map start", e.sjparserError());

    ASSERT_EQ(
        R"(parse error: client cancelled parse via callback return value
                                       {
                     (right here) ------^
)",
        e.parserError());
  } catch (...) {
    FAIL() << "Invalid exception thrown";
  }
}

TEST(Value, UnexpectedArrayStart) {
  std::string buf(R"([)");

  Parser<Value<bool>> parser;

  try {
    parser.parse(buf);
    FAIL() << "No exception thrown";
  } catch (ParseError &e) {
    ASSERT_FALSE(parser.parser().isSet());
    ASSERT_EQ("Unexpected token array start", e.sjparserError());

    ASSERT_EQ(
        R"(parse error: client cancelled parse via callback return value
                                       [
                     (right here) ------^
)",
        e.parserError());
  } catch (...) {
    FAIL() << "Invalid exception thrown";
  }
}

TEST(Value, UnsetValue) {
  Parser<Value<bool>> parser;

  ASSERT_FALSE(parser.parser().isSet());
  try {
    parser.parser().get();
    FAIL() << "No exception thrown";
  } catch (std::runtime_error &e) {
    ASSERT_STREQ("Can't get value, parser is unset", e.what());
  } catch (...) {
    FAIL() << "Invalid exception thrown";
  }
}

TEST(Value, ValueWithCallback) {
  std::string buf(R"("value")");
  std::string value;

  auto elementCb = [&](const std::string &str) {
    value = str;
    return true;
  };

  Parser<Value<std::string>> parser(elementCb);

  ASSERT_NO_THROW(parser.parse(buf));
  ASSERT_NO_THROW(parser.finish());

  ASSERT_TRUE(parser.parser().isSet());
  ASSERT_EQ("value", parser.parser().get());
  ASSERT_EQ("value", value);
}

TEST(Value, ValueWithCallbackError) {
  std::string buf(R"("value")");

  auto elementCb = [&](const std::string &) { return false; };

  Parser<Value<std::string>> parser(elementCb);

  try {
    parser.parse(buf);
    FAIL() << "No exception thrown";
  } catch (ParseError &e) {
    ASSERT_EQ("Callback returned false", e.sjparserError());
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
