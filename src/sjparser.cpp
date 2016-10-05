/*******************************************************************************

Copyright (c) 2016 Denis Tikhomirov <dvtikhomirov@gmail.com>

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

#include "sjparser_impl.h"

#include <yajl/yajl_parse.h>

using namespace SJParser;

void TokenParser::setDispatcher(Dispatcher *dispatcher) noexcept {
  _dispatcher = dispatcher;
}

void TokenParser::reset() noexcept {
  _set = false;
}

bool TokenParser::endParsing() {
  _set = true;
  bool ret = finish();

  if (_dispatcher) {
    _dispatcher->popParser();
  }
  return ret;
}

void ObjectParser::setDispatcher(Dispatcher *dispatcher) noexcept {
  TokenParser::setDispatcher(dispatcher);
  for (auto &field : _fields_map) {
    field.second->setDispatcher(dispatcher);
  }
}

void ObjectParser::reset() noexcept {
  TokenParser::reset();

  for (auto &field : _fields_map) {
    field.second->reset();
  }
}

bool ObjectParser::on(const MapStartT /*unused*/) noexcept {
  reset();
  return true;
}

bool ObjectParser::on(const MapKeyT &key) noexcept {
  try {
    auto &parser = _fields_map.at(key.key);
    _dispatcher->pushParser(parser);
  } catch (...) {
    return false;
  }
  return true;
}

bool ObjectParser::on(const MapEndT /*unused*/) {
  return endParsing();
}

void ArrayParser::reset() noexcept {
  TokenParser::reset();

  _parser->reset();
}

bool ArrayParser::on(const bool &value) {
  if (!_parser->on(value)) {
    return false;
  }
  childParsed();
  return true;
}

bool ArrayParser::on(const int64_t &value) {
  if (!_parser->on(value)) {
    return false;
  }
  childParsed();
  return true;
}

bool ArrayParser::on(const double &value) {
  if (!_parser->on(value)) {
    return false;
  }
  childParsed();
  return true;
}

bool ArrayParser::on(const std::string &value) {
  if (!_parser->on(value)) {
    return false;
  }
  childParsed();
  return true;
}

bool ArrayParser::on(const MapStartT /*unused*/) {
  _parser->setDispatcher(_dispatcher);
  _dispatcher->pushParser(_parser);
  return _parser->on(MapStartT{});
}

bool ArrayParser::on(const ArrayStartT /*unused*/) {
  if (!_started) {
    reset();
    _started = true;
    return true;
  }

  _parser->setDispatcher(_dispatcher);
  _dispatcher->pushParser(_parser);
  return _parser->on(ArrayStartT{});
}

bool ArrayParser::on(const ArrayEndT /*unused*/) {
  _started = false;
  return endParsing();
}

bool ArrayParser::finish() {
  if (!_on_finish) {
    return true;
  }
  return _on_finish();
}

Dispatcher::Dispatcher(TokenParser *parser) {
  _parsers.push(parser);
  parser->setDispatcher(this);
}

void Dispatcher::pushParser(TokenParser *parser) {
  _parsers.push(parser);
}

void Dispatcher::popParser() {
  _parsers.pop();

  if (!_parsers.empty()) {
    _parsers.top()->childParsed();
  }
}

template <typename T> bool Dispatcher::on(const T &value) {
  if (_parsers.empty()) {
    return false;
  }
  return _parsers.top()->on(value);
}

static int yajl_boolean(void *ctx, int value) {
  auto dispatcher = reinterpret_cast<Dispatcher *>(ctx);
  return static_cast<int>(dispatcher->on(static_cast<bool>(value)));
}

static int yajl_integer(void *ctx, long long value) {
  auto dispatcher = reinterpret_cast<Dispatcher *>(ctx);
  return static_cast<int>(dispatcher->on(static_cast<int64_t>(value)));
}

static int yajl_double(void *ctx, double value) {
  auto dispatcher = reinterpret_cast<Dispatcher *>(ctx);
  return static_cast<int>(dispatcher->on(value));
}

static int yajl_string(void *ctx, const unsigned char *value, size_t len) {
  auto dispatcher = reinterpret_cast<Dispatcher *>(ctx);
  return static_cast<int>(
      dispatcher->on(std::string(reinterpret_cast<const char *>(value), len)));
}

static int yajl_start_map(void *ctx) {
  auto dispatcher = reinterpret_cast<Dispatcher *>(ctx);
  return static_cast<int>(dispatcher->on(MapStartT{}));
}

static int yajl_map_key(void *ctx, const unsigned char *value, size_t len) {
  auto dispatcher = reinterpret_cast<Dispatcher *>(ctx);
  return static_cast<int>(dispatcher->on(
      MapKeyT{std::string(reinterpret_cast<const char *>(value), len)}));
}

static int yajl_end_map(void *ctx) {
  auto dispatcher = reinterpret_cast<Dispatcher *>(ctx);
  return static_cast<int>(dispatcher->on(MapEndT{}));
}

static int yajl_start_array(void *ctx) {
  auto dispatcher = reinterpret_cast<Dispatcher *>(ctx);
  return static_cast<int>(dispatcher->on(ArrayStartT{}));
}

static int yajl_end_array(void *ctx) {
  auto dispatcher = reinterpret_cast<Dispatcher *>(ctx);
  return static_cast<int>(dispatcher->on(ArrayEndT{}));
}

static const yajl_callbacks parser_yajl_callbacks{
    nullptr,      yajl_boolean,     yajl_integer,   yajl_double,
    nullptr,      yajl_string,      yajl_start_map, yajl_map_key,
    yajl_end_map, yajl_start_array, yajl_end_array};

struct SJParser::YajlInfo {
  yajl_handle handle;
};

ParserImpl::ParserImpl(TokenParser *parser)
    : _yajl_info(std::make_unique<YajlInfo>()), _dispatcher(parser) {
  _yajl_info->handle =
      yajl_alloc(&parser_yajl_callbacks, nullptr, &_dispatcher);
}

ParserImpl::~ParserImpl() {
  yajl_free(_yajl_info->handle);
}

bool ParserImpl::parse(const char *data, size_t len) {
  return yajl_parse(_yajl_info->handle,
                    reinterpret_cast<const unsigned char *>(data), len)
         == yajl_status_ok;
}

bool ParserImpl::finish() {
  return yajl_complete_parse(_yajl_info->handle) == yajl_status_ok;
}

std::string ParserImpl::getError() {
  auto err = yajl_get_error(_yajl_info->handle, 0, nullptr, 0);
  std::string error_str = reinterpret_cast<char *>(err);

  yajl_free_error(_yajl_info->handle, err);
  return error_str;
}
