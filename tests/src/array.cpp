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
#include <map>
#include "sjparser/sjparser.h"

using namespace SJParser;

TEST(Array, Empty) {
  std::string buf(R"([])");
  std::vector<bool> values;

  auto elementCb = [&](const bool &value) {
    values.push_back(value);
    return true;
  };

  Parser parser{Array{Value<bool>{elementCb}}};

  ASSERT_NO_THROW(parser.parse(buf));
  ASSERT_NO_THROW(parser.finish());

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

  Parser parser{Array{Value<bool>{elementCb}}};

  ASSERT_NO_THROW(parser.parse(buf));
  ASSERT_NO_THROW(parser.finish());

  ASSERT_EQ(0, values.size());

  ASSERT_FALSE(parser.parser().isSet());
}

TEST(Array, Reset) {
  std::string buf(R"([true])");
  std::vector<bool> values;

  auto elementCb = [&](const bool &value) {
    values.push_back(value);
    return true;
  };

  Parser parser{Array{Value<bool>{elementCb}}};

  ASSERT_NO_THROW(parser.parse(buf));
  ASSERT_NO_THROW(parser.finish());

  ASSERT_EQ(1, values.size());
  ASSERT_EQ(true, values[0]);

  buf = R"(null)";

  ASSERT_NO_THROW(parser.parse(buf));
  ASSERT_NO_THROW(parser.finish());

  ASSERT_FALSE(parser.parser().isSet());
}

TEST(Array, ArrayOfBooleans) {
  std::string buf(R"([true, false])");
  std::vector<bool> values;

  auto elementCb = [&](const bool &value) {
    values.push_back(value);
    return true;
  };

  Parser parser{Array{Value<bool>{elementCb}}};

  ASSERT_NO_THROW(parser.parse(buf));
  ASSERT_NO_THROW(parser.finish());

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

  Parser parser{Array{Value<int64_t>{elementCb}}};

  ASSERT_NO_THROW(parser.parse(buf));
  ASSERT_NO_THROW(parser.finish());

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

  Parser parser{Array{Value<double>{elementCb}}};

  ASSERT_NO_THROW(parser.parse(buf));
  ASSERT_NO_THROW(parser.finish());

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

  Parser parser{Array{Value<std::string>{elementCb}}};

  ASSERT_NO_THROW(parser.parse(buf));
  ASSERT_NO_THROW(parser.finish());

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

  Parser parser{Array{Value<bool>{elementCb}}};

  ASSERT_NO_THROW(parser.parse(buf));
  ASSERT_NO_THROW(parser.finish());

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

  Parser parser{Array{Value<bool>{elementCb}}};

  ASSERT_NO_THROW(parser.parse(buf));
  ASSERT_NO_THROW(parser.finish());

  ASSERT_EQ(2, values.size());
  ASSERT_EQ(true, values[0]);
  ASSERT_EQ(false, values[1]);

  ASSERT_TRUE(parser.parser().isSet());
}

TEST(Array, UnexpectedBoolean) {
  std::string buf(R"(true)");

  auto elementCb = [&](const bool &) { return true; };

  Parser parser{Array{Value<bool>{elementCb}}};

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

TEST(Array, UnexpectedInteger) {
  std::string buf(R"(10)");

  auto elementCb = [&](const int64_t &) { return true; };

  Parser parser{Array{Value<int64_t>{elementCb}}};

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

TEST(Array, UnexpectedDouble) {
  std::string buf(R"(10.5)");

  auto elementCb = [&](const double &) { return true; };

  Parser parser{Array{Value<double>{elementCb}}};

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

TEST(Array, UnexpectedString) {
  std::string buf(R"("value")");

  auto elementCb = [&](const std::string &) { return true; };

  Parser parser{Array{Value<std::string>{elementCb}}};

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

TEST(Array, ArrayWithUnexpectedBoolean) {
  std::string buf(R"([true])");

  auto elementCb = [&](const std::string &) { return true; };

  Parser parser{Array{Value<std::string>{elementCb}}};

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

TEST(Array, ArrayWithUnexpectedInteger) {
  std::string buf(R"([10])");

  auto elementCb = [&](const bool &) { return true; };

  Parser parser{Array{Value<bool>{elementCb}}};

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

TEST(Array, ArrayWithUnexpectedDouble) {
  std::string buf(R"([10.5])");

  auto elementCb = [&](const bool &) { return true; };

  Parser parser{Array{Value<bool>{elementCb}}};

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

TEST(Array, ArrayWithUnexpectedString) {
  std::string buf(R"(["value"])");

  auto elementCb = [&](const bool &) { return true; };

  Parser parser{Array{Value<bool>{elementCb}}};

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

TEST(Array, ArrayWithElementCallbackError) {
  std::string buf(R"([true, false])");

  auto elementCb = [&](const bool &) { return false; };

  Parser parser{Array{Value<bool>{elementCb}}};

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

TEST(Array, ArrayWithCallback) {
  std::string buf(R"([true, false])");
  std::vector<bool> values;
  bool callback_called = false;

  auto elementCb = [&](const bool &value) {
    values.push_back(value);
    return true;
  };

  auto arrayCb = [&](Array<Value<bool>> &) {
    callback_called = true;
    return true;
  };

  Parser parser{Array{Value<bool>{elementCb}, arrayCb}};

  ASSERT_NO_THROW(parser.parse(buf));
  ASSERT_NO_THROW(parser.finish());

  ASSERT_EQ(2, values.size());
  ASSERT_EQ(true, values[0]);
  ASSERT_EQ(false, values[1]);

  ASSERT_EQ(true, callback_called);

  ASSERT_TRUE(parser.parser().isSet());
}

TEST(Array, ArrayWithCallbackError) {
  std::string buf(R"([true, false])");

  auto elementCb = [&](const bool &) { return true; };

  Parser parser{Array{Value<bool>{elementCb}}};

  auto arrayCb = [&](decltype(parser)::ParserType &) { return false; };

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

TEST(Array, ArrayOfObjects) {
  std::string buf(
      R"([{"key": "value", "key2": 10}, {"key": "value2", "key2": 20}])");

  struct ObjectStruct {
    std::string member1;
    int64_t member2;
  };

  std::vector<ObjectStruct> values;

  using ObjectParser = Object<Value<std::string>, Value<int64_t>>;

  auto objectCb = [&](ObjectParser &parser) {
    values.push_back({parser.pop<0>(), parser.pop<1>()});
    return true;
  };

  Parser parser{Array{Object{std::tuple{Member{"key", Value<std::string>{}},
                                        Member{"key2", Value<int64_t>{}}},
                             objectCb}}};

  ASSERT_NO_THROW(parser.parse(buf));
  ASSERT_NO_THROW(parser.finish());

  ASSERT_EQ(2, values.size());
  ASSERT_EQ("value", values[0].member1);
  ASSERT_EQ(10, values[0].member2);
  ASSERT_EQ("value2", values[1].member1);
  ASSERT_EQ(20, values[1].member2);
}

TEST(Array, UnexpectedObject) {
  std::string buf(
      R"({"key": "value"})");

  Parser parser{Array{Object{std::tuple{Member{"key", Value<std::string>{}}}}}};

  try {
    parser.parse(buf);
    FAIL() << "No exception thrown";
  } catch (ParsingError &e) {
    ASSERT_FALSE(parser.parser().isSet());
    ASSERT_EQ("Unexpected token map start", e.sjparserError());

    ASSERT_EQ(
        R"(parse error: client cancelled parse via callback return value
                                       {"key": "value"}
                     (right here) ------^
)",
        e.parserError());
  } catch (...) {
    FAIL() << "Invalid exception thrown";
  }
}

TEST(Array, ArrayWithUnexpectedObject) {
  std::string buf(
      R"([{"key2": "value"}])");

  Parser parser{Array{Object{std::tuple{Member{"key", Value<std::string>{}}}}}};

  try {
    parser.parse(buf);
    FAIL() << "No exception thrown";
  } catch (ParsingError &e) {
    ASSERT_FALSE(parser.parser().isSet());
    ASSERT_EQ("Unexpected member key2", e.sjparserError());

    ASSERT_EQ(
        R"(parse error: client cancelled parse via callback return value
                                [{"key2": "value"}]
                     (right here) ------^
)",
        e.parserError());
  } catch (...) {
    FAIL() << "Invalid exception thrown";
  }
}

TEST(Array, ArrayOfSCustomObjects) {
  std::string buf(
      R"([{"key": "value", "key2": 10}, {"key": "value2", "key2": 20}])");

  struct ObjectStruct {
    std::string member1;
    int64_t member2;
  };

  std::vector<ObjectStruct> values;

  Parser parser{
      Array{SCustomObject{TypeHolder<ObjectStruct>{},
                          std::tuple{Member{"key", Value<std::string>{}},
                                     Member{"key2", Value<int64_t>{}}}}}};

  auto objectCb = [&](decltype(parser)::ParserType::ParserType &parser,
                      ObjectStruct &value) {
    value = {parser.pop<0>(), parser.pop<1>()};
    values.push_back(value);
    return true;
  };

  parser.parser().parser().setFinishCallback(objectCb);

  ASSERT_NO_THROW(parser.parse(buf));
  ASSERT_NO_THROW(parser.finish());

  ASSERT_EQ(2, values.size());
  ASSERT_EQ("value", values[0].member1);
  ASSERT_EQ(10, values[0].member2);
  ASSERT_EQ("value2", values[1].member1);
  ASSERT_EQ(20, values[1].member2);
}

TEST(Array, ArrayOfSAutoObjects) {
  std::string buf(
      R"([{"key": "value", "key2": 10}, {"key": "value2", "key2": 20}])");

  std::vector<std::tuple<std::string, int64_t>> values;

  auto objectCb = [&](const std::tuple<std::string, int64_t> &value) {
    values.push_back(value);
    return true;
  };

  Parser parser{
      Array{SAutoObject{std::tuple{Member{"key", Value<std::string>{}},
                                   Member{"key2", Value<int64_t>{}}},
                        objectCb}}};

  ASSERT_NO_THROW(parser.parse(buf));
  ASSERT_NO_THROW(parser.finish());

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

  Parser parser{
      Array{Union{TypeHolder<std::string>{}, "type",
                  std::tuple{Member{"str", Object{std::tuple{Member{
                                               "key", Value<std::string>{}}}}},
                             Member{"int", Object{std::tuple{Member{
                                               "key", Value<int64_t>{}}}}}}}}};

  auto unionCb = [&](decltype(parser)::ParserType::ParserType &parser) {
    switch (parser.currentMemberId()) {
      case 0:
        value_str = parser.get<0>().get<0>();
        break;
      case 1:
        value_int = parser.get<1>().get<0>();
        break;
      default:
        return false;
    }
    return true;
  };

  parser.parser().parser().setFinishCallback(unionCb);

  ASSERT_NO_THROW(parser.parse(buf));
  ASSERT_NO_THROW(parser.finish());

  ASSERT_EQ("value", value_str);
  ASSERT_EQ(10, value_int);
}

TEST(Array, ArrayOfStandaloneSUnions) {
  std::string buf(
      R"([{"type": "str", "key": "value"}, {"type": "int", "key": 10}])");

  std::string value_str = "";
  int64_t value_int = 0;

  Parser parser{
      Array{SUnion{TypeHolder<std::string>{}, "type",
                   std::tuple{Member{"str", SAutoObject{std::tuple{Member{
                                                "key", Value<std::string>{}}}}},
                              Member{"int", SAutoObject{std::tuple{Member{
                                                "key", Value<int64_t>{}}}}}}}}};

  auto unionCb = [&](const std::variant<std::tuple<std::string>,
                                        std::tuple<int64_t>> &value) {
    if (value.index() == 0) {
      value_str = std::get<0>(std::get<0>(value));
    } else {
      value_int = std::get<0>(std::get<1>(value));
    }
    return true;
  };

  parser.parser().parser().setFinishCallback(unionCb);

  ASSERT_NO_THROW(parser.parse(buf));
  ASSERT_NO_THROW(parser.finish());

  ASSERT_EQ("value", value_str);
  ASSERT_EQ(10, value_int);
}

TEST(Array, ArrayOfEmbeddedUnions) {
  std::string buf(
      R"([{"id": 1, "type": "str", "key": "value"},
          {"id": 2, "type": "int", "key": 10}])");

  std::vector<int64_t> ids;
  std::string value_str = "";
  int64_t value_int = 0;

  Parser parser{Array{Object{std::tuple{
      Member{"id", Value<int64_t>{}},
      Member{
          "type",
          Union{TypeHolder<std::string>{},
                std::tuple{Member{"str", Object{std::tuple{Member{
                                             "key", Value<std::string>{}}}}},
                           Member{"int", Object{std::tuple{Member{
                                             "key", Value<int64_t>{}}}}}}}}}}}};

  auto objectCb = [&](decltype(parser)::ParserType::ParserType &parser) {
    ids.push_back(parser.get<0>());

    switch (parser.get<1>().currentMemberId()) {
      case 0:
        value_str = parser.get<1>().get<0>().get<0>();
        break;
      case 1:
        value_int = parser.get<1>().get<1>().get<0>();
        break;
      default:
        return false;
    }
    return true;
  };

  parser.parser().parser().setFinishCallback(objectCb);

  ASSERT_NO_THROW(parser.parse(buf));
  ASSERT_NO_THROW(parser.finish());

  ASSERT_EQ(2, ids.size());
  ASSERT_EQ(1, ids[0]);
  ASSERT_EQ(2, ids[1]);
  ASSERT_EQ("value", value_str);
  ASSERT_EQ(10, value_int);
}

TEST(Array, ArrayOfEmbeddedSUnions) {
  std::string buf(
      R"([{"id": 1, "type": "str", "key": "value"},
          {"id": 2, "type": "int", "key": 10}])");

  std::vector<int64_t> ids;
  std::string value_str = "";
  int64_t value_int = 0;

  Parser parser{Array{Object{std::tuple{
      Member{"id", Value<int64_t>{}},
      Member{"type",
             SUnion{TypeHolder<std::string>{},
                    std::tuple{
                        Member{"str", SAutoObject{std::tuple{Member{
                                          "key", Value<std::string>{}}}}},
                        Member{"int", SAutoObject{std::tuple{Member{
                                          "key", Value<int64_t>{}}}}}}}}}}}};

  auto objectCb = [&](decltype(parser)::ParserType::ParserType &parser) {
    ids.push_back(parser.get<0>());

    auto value = parser.get<1>();

    if (value.index() == 0) {
      value_str = std::get<0>(std::get<0>(value));
    } else {
      value_int = std::get<0>(std::get<1>(value));
    }
    return true;
  };

  parser.parser().parser().setFinishCallback(objectCb);

  ASSERT_NO_THROW(parser.parse(buf));
  ASSERT_NO_THROW(parser.finish());

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

  auto innerArrayCb = [&](Array<Value<bool>> &) {
    values.push_back(tmp_values);
    tmp_values.clear();
    return true;
  };

  Parser parser{Array{Array{Value<bool>{elementCb}, innerArrayCb}}};

  ASSERT_NO_THROW(parser.parse(buf));
  ASSERT_NO_THROW(parser.finish());

  ASSERT_EQ(2, values.size());
  ASSERT_EQ(2, values[0].size());
  ASSERT_EQ(true, values[0][0]);
  ASSERT_EQ(true, values[0][1]);

  ASSERT_EQ(2, values[1].size());
  ASSERT_EQ(false, values[1][0]);
  ASSERT_EQ(false, values[1][1]);
}

TEST(Array, ArrayOfMaps) {
  std::string buf(R"([{"1": 10, "2": 20}, {"1": 30, "2": 40}])");
  std::vector<std::map<std::string, int64_t>> values;
  std::map<std::string, int64_t> tmp_values;

  Parser parser{Array{Map{Value<int64_t>{}}}};

  auto mapElementCb =
      [&](const std::string &key,
          decltype(parser)::ParserType::ParserType::ParserType &parser) {
        tmp_values[key] = parser.get();
        return true;
      };

  auto mapFinishCb = [&](decltype(parser)::ParserType::ParserType &) {
    values.push_back(tmp_values);
    tmp_values.clear();
    return true;
  };

  parser.parser().parser().setElementCallback(mapElementCb);

  parser.parser().parser().setFinishCallback(mapFinishCb);

  ASSERT_NO_THROW(parser.parse(buf));
  ASSERT_NO_THROW(parser.finish());

  ASSERT_EQ(2, values.size());
  ASSERT_EQ(2, values[0].size());
  ASSERT_EQ(10, values[0]["1"]);
  ASSERT_EQ(20, values[0]["2"]);

  ASSERT_EQ(2, values[1].size());
  ASSERT_EQ(30, values[1]["1"]);
  ASSERT_EQ(40, values[1]["2"]);
}

TEST(Array, ArrayOfSArrays) {
  std::string buf(R"([[true, true], [false, false]])");
  std::vector<std::vector<bool>> values;

  auto innerArrayCb = [&](const std::vector<bool> &value) {
    values.push_back(value);
    return true;
  };

  Parser parser{Array{SArray{Value<bool>{}, innerArrayCb}}};

  ASSERT_NO_THROW(parser.parse(buf));
  ASSERT_NO_THROW(parser.finish());

  ASSERT_EQ(2, values.size());
  ASSERT_EQ(2, values[0].size());
  ASSERT_EQ(true, values[0][0]);
  ASSERT_EQ(true, values[0][1]);

  ASSERT_EQ(2, values[1].size());
  ASSERT_EQ(false, values[1][0]);
  ASSERT_EQ(false, values[1][1]);
}

TEST(Array, ArrayWithParserReference) {
  std::string buf(R"([[13, 15, 16]])");

  SArray sarray{Value<int64_t>{}};

  Parser parser{Array{sarray}};

  ASSERT_NO_THROW(parser.parse(buf));
  ASSERT_NO_THROW(parser.finish());

  ASSERT_EQ(3, sarray.get().size());
  ASSERT_EQ(13, sarray.get()[0]);
  ASSERT_EQ(15, sarray.get()[1]);
  ASSERT_EQ(16, sarray.get()[2]);

  ASSERT_EQ(&(parser.parser().parser()), &sarray);
}

// Just check if the constructor compiles
TEST(Array, ArrayWithArrayReference) {
  Array array{Value<int64_t>{}};

  Parser parser{Array{array}};

  ASSERT_EQ(&(parser.parser().parser()), &array);
}
