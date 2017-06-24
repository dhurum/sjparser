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

#include "internals.h"

namespace SJParser {

void TokenParser::setDispatcher(Dispatcher *dispatcher) noexcept {
  _dispatcher = dispatcher;
}

void TokenParser::reset() {
  _set = false;
}

void TokenParser::endParsing() {
  _set = true;
  finish();

  if (_dispatcher) {
    _dispatcher->popParser();
  }
}

void TokenParser::on(NullT /*unused*/) {
  reset();

  if (_dispatcher) {
    _dispatcher->popParser();
  }
}

void TokenParser::on(const bool & /*value*/) {
  unexpectedToken("boolean");
}

void TokenParser::on(const int64_t & /*value*/) {
  unexpectedToken("integer");
}

void TokenParser::on(const double & /*value*/) {
  unexpectedToken("double");
}

void TokenParser::on(const std::string & /*value*/) {
  unexpectedToken("string");
}

void TokenParser::on(MapStartT /*unused*/) {
  unexpectedToken("map start");
}

void TokenParser::on(const MapKeyT & /*key*/) {
  unexpectedToken("map key");
}

void TokenParser::on(MapEndT /*unused*/) {
  unexpectedToken("map end");
}

void TokenParser::on(ArrayStartT /*unused*/) {
  unexpectedToken("array start");
}

void TokenParser::on(ArrayEndT /*unused*/) {
  unexpectedToken("array end");
}

void TokenParser::childParsed() {}

void ArrayParser::reset() {
  TokenParser::reset();

  _parser->reset();
}

void ArrayParser::on(NullT /*unused*/) {
  if (!_started) {
    TokenParser::on(NullT{});
    return;
  }
  _parser->on(NullT{});
}

void ArrayParser::on(const bool &value) {
  if (!_started) {
    unexpectedToken("boolean");
  }
  _parser->on(value);
  childParsed();
}

void ArrayParser::on(const int64_t &value) {
  if (!_started) {
    unexpectedToken("integer");
  }
  _parser->on(value);
  childParsed();
}

void ArrayParser::on(const double &value) {
  if (!_started) {
    unexpectedToken("double");
  }
  _parser->on(value);
  childParsed();
}

void ArrayParser::on(const std::string &value) {
  if (!_started) {
    unexpectedToken("string");
  }
  _parser->on(value);
  childParsed();
}

void ArrayParser::on(const MapStartT /*unused*/) {
  if (!_started) {
    unexpectedToken("map start");
  }
  _parser->setDispatcher(_dispatcher);
  _dispatcher->pushParser(_parser);
  _parser->on(MapStartT{});
}

void ArrayParser::on(const ArrayStartT /*unused*/) {
  if (!_started) {
    reset();
    _started = true;
    return;
  }

  _parser->setDispatcher(_dispatcher);
  _dispatcher->pushParser(_parser);
  _parser->on(ArrayStartT{});
}

void ArrayParser::on(const ArrayEndT /*unused*/) {
  _started = false;
  endParsing();
}

Dispatcher::Dispatcher(TokenParser *parser) {
  _root_parser = parser;
  _parsers.push_back(parser);
  parser->setDispatcher(this);
}

void Dispatcher::pushParser(TokenParser *parser) {
  _parsers.push_back(parser);
}

void Dispatcher::popParser() {
  if (_parsers.empty()) {
    throw std::runtime_error("Can not pop parser, parsers stack is empty");
  }
  _parsers.pop_back();

  if (!_parsers.empty()) {
    _parsers.back()->childParsed();
  }
}

bool Dispatcher::emptyParsersStack() {
  return _parsers.empty();
}

void Dispatcher::reset() {
  _parsers.clear();
  _parsers.push_back(_root_parser);
}
}  // namespace SJParser

namespace std {

size_t hash<SJParser::FieldName>::operator()(
    const SJParser::FieldName &key) const {
  return hash<std::string>()(key.str());
}
basic_ostream<char> &operator<<(basic_ostream<char> &stream,
                                const SJParser::FieldName &name) {
  return stream << name.str();
}
}  // namespace std
