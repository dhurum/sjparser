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

#include "yajl_parser.h"
#include "parsing_error.h"

namespace SJParser {

// clang-tidy thinks this is redundant, while the standart thinks otherwise
constexpr yajl_callbacks YajlParser::_parser_yajl_callbacks;  // NOLINT

YajlParser::YajlParser() {
  resetYajlHandle();
}

YajlParser::~YajlParser() {
  freeYajlHandle();
}

void YajlParser::setTokenParser(TokenParser *parser) {
  _dispatcher = std::make_unique<Dispatcher>(parser);
}

void YajlParser::resetYajlHandle() {
  freeYajlHandle();
  _yajl_handle = yajl_alloc(&_parser_yajl_callbacks, nullptr, this);
  if (!_yajl_handle) {
    throw std::runtime_error("Can not allocate YAJL handle");
  }
}

void YajlParser::freeYajlHandle() {
  if (!_yajl_handle) {
    return;
  }
  yajl_free(_yajl_handle);
  _yajl_handle = nullptr;
}

void YajlParser::parse(const std::string &data) {
  parse(data.data(), data.size());
}

void YajlParser::parse(const char *data, size_t len) {
  _data = reinterpret_cast<const unsigned char *>(data);
  _len = len;

  if (_reset_needed) {
    _dispatcher->reset();
    resetYajlHandle();
    _sjparser_error = "";

    _reset_needed = false;
  }

  if (yajl_parse(_yajl_handle, _data, _len) != yajl_status_ok) {
    _reset_needed = true;
    throwParsingError();
  }
}

void YajlParser::finish() {
  _reset_needed = true;

  if (yajl_complete_parse(_yajl_handle) != yajl_status_ok) {
    throwParsingError();
  } else {
    checkDispatcherStack();
  }
}

void YajlParser::checkDispatcherStack() {
  if (_dispatcher->emptyParsersStack()) {
    return;
  }

  throw ParsingError("Dispatcher parsers stack is not empty in the end");
}

void YajlParser::throwParsingError() {
  auto yajl_error_ptr = yajl_get_error(_yajl_handle, 1, _data, _len);
  std::string yajl_error;

  if (yajl_error_ptr) {
    yajl_error = reinterpret_cast<char *>(yajl_error_ptr);
    yajl_free_error(_yajl_handle, yajl_error_ptr);
  } else {
    yajl_error = "Unknown YAJL error\n";
  }
  throw ParsingError(_sjparser_error, yajl_error);
}

int YajlParser::yajlOnNull(void *ctx) {
  auto parser = reinterpret_cast<YajlParser *>(ctx);
  return parser->on(NullT{});
}

int YajlParser::yajlOnBool(void *ctx, int value) {
  auto parser = reinterpret_cast<YajlParser *>(ctx);
  return parser->on(static_cast<bool>(value));
}

int YajlParser::yajlOnInt(void *ctx, long long value) {
  auto parser = reinterpret_cast<YajlParser *>(ctx);
  return parser->on(static_cast<int64_t>(value));
}

int YajlParser::yajlOnDouble(void *ctx, double value) {
  auto parser = reinterpret_cast<YajlParser *>(ctx);
  return parser->on(value);
}

int YajlParser::yajlOnString(void *ctx, const unsigned char *value,
                             size_t len) {
  auto parser = reinterpret_cast<YajlParser *>(ctx);
  return static_cast<int>(
      parser->on(std::string(reinterpret_cast<const char *>(value), len)));
}

int YajlParser::yajlOnMapStart(void *ctx) {
  auto parser = reinterpret_cast<YajlParser *>(ctx);
  return parser->on(MapStartT{});
}

int YajlParser::yajlOnMapKey(void *ctx, const unsigned char *value,
                             size_t len) {
  auto parser = reinterpret_cast<YajlParser *>(ctx);
  return static_cast<int>(parser->on(
      MapKeyT{std::string(reinterpret_cast<const char *>(value), len)}));
}

int YajlParser::yajlOnMapEnd(void *ctx) {
  auto parser = reinterpret_cast<YajlParser *>(ctx);
  return parser->on(MapEndT{});
}

int YajlParser::yajlOnArrayStart(void *ctx) {
  auto parser = reinterpret_cast<YajlParser *>(ctx);
  return parser->on(ArrayStartT{});
}

int YajlParser::yajlOnArrayEnd(void *ctx) {
  auto parser = reinterpret_cast<YajlParser *>(ctx);
  return parser->on(ArrayEndT{});
}

FieldName::FieldName(std::string str) : _str(std::move(str)) {}

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