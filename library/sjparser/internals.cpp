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

void TokenParser::on(bool /*value*/) {
  unexpectedToken("boolean");
}  // LCOV_EXCL_LINE

void TokenParser::on(int64_t /*value*/) {
  unexpectedToken("integer");
}  // LCOV_EXCL_LINE

void TokenParser::on(double /*value*/) {
  unexpectedToken("double");
}  // LCOV_EXCL_LINE

void TokenParser::on(std::string_view /*value*/) {
  unexpectedToken("string");
}  // LCOV_EXCL_LINE

void TokenParser::on(MapStartT /*unused*/) {
  unexpectedToken("map start");
}  // LCOV_EXCL_LINE

void TokenParser::on(MapKeyT /*key*/) {
  unexpectedToken("map key");
}  // LCOV_EXCL_LINE

void TokenParser::on(MapEndT /*unused*/) {
  unexpectedToken("map end");
}  // LCOV_EXCL_LINE

void TokenParser::on(ArrayStartT /*unused*/) {
  unexpectedToken("array start");
}  // LCOV_EXCL_LINE

void TokenParser::on(ArrayEndT /*unused*/) {
  unexpectedToken("array end");
}  // LCOV_EXCL_LINE

void TokenParser::childParsed() {}

void ArrayParser::reset() {
  TokenParser::reset();

  _parser_ptr->reset();
}

void ArrayParser::on(NullT /*unused*/) {
  if (!_started) {
    TokenParser::on(NullT{});
    return;
  }
  _parser_ptr->on(NullT{});
}

void ArrayParser::on(bool value) {
  if (!_started) {
    unexpectedToken("boolean");
  }
  _parser_ptr->on(value);
  childParsed();
}

void ArrayParser::on(int64_t value) {
  if (!_started) {
    unexpectedToken("integer");
  }
  _parser_ptr->on(value);
  childParsed();
}

void ArrayParser::on(double value) {
  if (!_started) {
    unexpectedToken("double");
  }
  _parser_ptr->on(value);
  childParsed();
}

void ArrayParser::on(std::string_view value) {
  if (!_started) {
    unexpectedToken("string");
  }
  _parser_ptr->on(value);
  childParsed();
}

void ArrayParser::on(MapStartT /*unused*/) {
  if (!_started) {
    unexpectedToken("map start");
  }
  _parser_ptr->setDispatcher(_dispatcher);
  _dispatcher->pushParser(_parser_ptr);
  _parser_ptr->on(MapStartT{});
}

void ArrayParser::on(ArrayStartT /*unused*/) {
  if (!_started) {
    reset();
    _started = true;
    return;
  }

  _parser_ptr->setDispatcher(_dispatcher);
  _dispatcher->pushParser(_parser_ptr);
  _parser_ptr->on(ArrayStartT{});
}

void ArrayParser::on(ArrayEndT /*unused*/) {
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

void Ignore::reset() {
  TokenParser::reset();

  _structure.clear();
}

void Ignore::onValue() {
  if (_structure.empty()) {
    endParsing();
  }
}

void Ignore::on(bool /*value*/) {
  onValue();
}

void Ignore::on(int64_t /*value*/) {
  onValue();
}

void Ignore::on(double /*value*/) {
  onValue();
}

void Ignore::on(std::string_view /*value*/) {
  onValue();
}

void Ignore::on(MapStartT /*unused*/) {
  _structure.push_back(Structure::Object);
}

void Ignore::on(MapKeyT /*key*/) {
  if (_structure.empty() || (_structure.back() != Structure::Object)) {
    unexpectedToken("map key");
  }
}

void Ignore::on(MapEndT /*unused*/) {
  if (_structure.empty() || (_structure.back() != Structure::Object)) {
    unexpectedToken("map end");
  }
  _structure.pop_back();

  if (_structure.empty()) {
    endParsing();
  }
}

void Ignore::on(ArrayStartT /*unused*/) {
  _structure.push_back(Structure::Array);
}

void Ignore::on(ArrayEndT /*unused*/) {
  if (_structure.empty() || (_structure.back() != Structure::Array)) {
    unexpectedToken("array end");
  }
  _structure.pop_back();

  if (_structure.empty()) {
    endParsing();
  }
}

void Ignore::finish() {}

std::basic_ostream<char> &operator<<(std::basic_ostream<char> &stream,
                                     const FieldName &name) {
  return stream << name.str();
}

FieldName::FieldName(std::string str) : _str(std::move(str)) {}

FieldName::FieldName(std::string_view str) : _str(str) {}

FieldName::FieldName(const char *str) : _str(str) {}

FieldName::operator const std::string &() const {
  return _str;
}

bool FieldName::operator==(const FieldName &other) const {
  return _str == other._str;
}

const std::string &FieldName::str() const {
  return _str;
}
}  // namespace SJParser
