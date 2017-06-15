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

#include "sjparser_internals.h"

#include <yajl/yajl_parse.h>

namespace SJParser {

void TokenParser::setDispatcher(Dispatcher *dispatcher) noexcept {
  _dispatcher = dispatcher;
}

void TokenParser::reset() noexcept {
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

void ArrayParser::reset() noexcept {
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
  _parser->on(value);
  childParsed();
}

void ArrayParser::on(const int64_t &value) {
  _parser->on(value);
  childParsed();
}

void ArrayParser::on(const double &value) {
  _parser->on(value);
  childParsed();
}

void ArrayParser::on(const std::string &value) {
  _parser->on(value);
  childParsed();
}

void ArrayParser::on(const MapStartT /*unused*/) {
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

static int yajl_null(void *ctx) {
  auto parser = reinterpret_cast<ParserImpl *>(ctx);
  return parser->on(NullT{});
}

static int yajl_boolean(void *ctx, int value) {
  auto parser = reinterpret_cast<ParserImpl *>(ctx);
  return parser->on(static_cast<bool>(value));
}

static int yajl_integer(void *ctx, long long value) {
  auto parser = reinterpret_cast<ParserImpl *>(ctx);
  return parser->on(static_cast<int64_t>(value));
}

static int yajl_double(void *ctx, double value) {
  auto parser = reinterpret_cast<ParserImpl *>(ctx);
  return parser->on(value);
}

static int yajl_string(void *ctx, const unsigned char *value, size_t len) {
  auto parser = reinterpret_cast<ParserImpl *>(ctx);
  return static_cast<int>(
      parser->on(std::string(reinterpret_cast<const char *>(value), len)));
}

static int yajl_start_map(void *ctx) {
  auto parser = reinterpret_cast<ParserImpl *>(ctx);
  return parser->on(MapStartT{});
}

static int yajl_map_key(void *ctx, const unsigned char *value, size_t len) {
  auto parser = reinterpret_cast<ParserImpl *>(ctx);
  return static_cast<int>(parser->on(
      MapKeyT{std::string(reinterpret_cast<const char *>(value), len)}));
}

static int yajl_end_map(void *ctx) {
  auto parser = reinterpret_cast<ParserImpl *>(ctx);
  return parser->on(MapEndT{});
}

static int yajl_start_array(void *ctx) {
  auto parser = reinterpret_cast<ParserImpl *>(ctx);
  return parser->on(ArrayStartT{});
}

static int yajl_end_array(void *ctx) {
  auto parser = reinterpret_cast<ParserImpl *>(ctx);
  return parser->on(ArrayEndT{});
}

static const yajl_callbacks parser_yajl_callbacks{
    yajl_null,    yajl_boolean,     yajl_integer,   yajl_double,
    nullptr,      yajl_string,      yajl_start_map, yajl_map_key,
    yajl_end_map, yajl_start_array, yajl_end_array};

class YajlInfo {
 public:
  YajlInfo(ParserImpl *parser);
  ~YajlInfo();
  void reset();
  yajl_handle handle();

 private:
  void unset();

  yajl_handle _handle = nullptr;
  ParserImpl *_parser;
};

YajlInfo::YajlInfo(ParserImpl *parser) : _parser(parser) {
  reset();
}

YajlInfo::~YajlInfo() {
  unset();
}

void YajlInfo::reset() {
  unset();
  _handle = yajl_alloc(&parser_yajl_callbacks, nullptr, _parser);
}

yajl_handle YajlInfo::handle() {
  return _handle;
}

void YajlInfo::unset() {
  if (_handle) {
    yajl_free(_handle);
    _handle = nullptr;
  }
}

ParserImpl::ParserImpl(TokenParser *parser)
    : _dispatcher(parser), _yajl_info(std::make_unique<YajlInfo>(this)) {}

ParserImpl::~ParserImpl() = default;

bool ParserImpl::parse(const char *data, size_t len) {
  _data = reinterpret_cast<const unsigned char *>(data);
  _len = len;

  bool ret = yajl_parse(_yajl_info->handle(), _data, _len) == yajl_status_ok;

  if (!ret) {
    getYajlError();
  }

  return ret;
}

bool ParserImpl::finish() {
  bool ret = yajl_complete_parse(_yajl_info->handle()) == yajl_status_ok;

  if (!ret) {
    getYajlError();
  } else if (!_dispatcher.emptyParsersStack()) {
    ret = false;
    _sjparser_error = "Dispatcher parsers stack is not empty in the end";
  }

  _dispatcher.reset();
  _yajl_info->reset();

  return ret;
}

void ParserImpl::getYajlError() {
  auto err = yajl_get_error(_yajl_info->handle(), 1, _data, _len);
  _yajl_error = reinterpret_cast<char *>(err);
  yajl_free_error(_yajl_info->handle(), err);
}

std::string ParserImpl::getError(bool verbose) {
  if (!_sjparser_error.empty() && !verbose) {
    return _sjparser_error;
  }

  return _yajl_error + _sjparser_error + "\n";
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
