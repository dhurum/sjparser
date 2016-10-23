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
    ret = ret && _dispatcher->popParser();
  }
  return ret;
}

bool TokenParser::on(const bool & /*value*/) {
  return unexpectedToken("boolean");
}

bool TokenParser::on(const int64_t & /*value*/) {
  return unexpectedToken("integer");
}

bool TokenParser::on(const double & /*value*/) {
  return unexpectedToken("double");
}

bool TokenParser::on(const std::string & /*value*/) {
  return unexpectedToken("string");
}

bool TokenParser::on(const MapStartT) {
  return unexpectedToken("map start");
}

bool TokenParser::on(const MapKeyT & /*key*/) {
  return unexpectedToken("map key");
}

bool TokenParser::on(const MapEndT) {
  return unexpectedToken("map end");
}

bool TokenParser::on(const ArrayStartT) {
  return unexpectedToken("array start");
}

bool TokenParser::on(const ArrayEndT) {
  return unexpectedToken("array end");
}

bool TokenParser::childParsed() {
  return true;
}

void ArrayParser::reset() noexcept {
  TokenParser::reset();

  _parser->reset();
}

bool ArrayParser::on(const bool &value) {
  if (!_parser->on(value)) {
    return false;
  }
  return childParsed();
}

bool ArrayParser::on(const int64_t &value) {
  if (!_parser->on(value)) {
    return false;
  }
  return childParsed();
}

bool ArrayParser::on(const double &value) {
  if (!_parser->on(value)) {
    return false;
  }
  return childParsed();
}

bool ArrayParser::on(const std::string &value) {
  if (!_parser->on(value)) {
    return false;
  }
  return childParsed();
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
  _root_parser = parser;
  _parsers.push_back(parser);
  parser->setDispatcher(this);
}

void Dispatcher::pushParser(TokenParser *parser) {
  _parsers.push_back(parser);
}

bool Dispatcher::popParser() {
  _parsers.pop_back();

  if (!_parsers.empty()) {
    return _parsers.back()->childParsed();
  }
  return true;
}

void Dispatcher::reset() {
  _parsers.clear();
  _parsers.push_back(_root_parser);
  setError("");
}

template <typename T> bool Dispatcher::on(const T &value) {
  if (_parsers.empty()) {
    setError("Parsers stack is empty");
    return false;
  }
  return _parsers.back()->on(value);
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

class SJParser::YajlInfo {
 public:
  YajlInfo(Dispatcher *dispatcher);
  ~YajlInfo();
  void reset();
  yajl_handle handle();

 private:
  void unset();

  yajl_handle _handle = nullptr;
  Dispatcher *_dispatcher;
};

YajlInfo::YajlInfo(Dispatcher *dispatcher) : _dispatcher(dispatcher) {
  reset();
}

YajlInfo::~YajlInfo() {
  unset();
}

void YajlInfo::reset() {
  unset();
  _handle = yajl_alloc(&parser_yajl_callbacks, nullptr, _dispatcher);
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
    : _dispatcher(parser),
      _yajl_info(std::make_unique<YajlInfo>(&_dispatcher)) {}

ParserImpl::~ParserImpl() = default;

bool ParserImpl::parse(const char *data, size_t len) {
  _data = reinterpret_cast<const unsigned char *>(data);
  _len = len;

  return yajl_parse(_yajl_info->handle(), _data, _len) == yajl_status_ok;
}

bool ParserImpl::finish() {
  bool ret = yajl_complete_parse(_yajl_info->handle()) == yajl_status_ok;

  _dispatcher.reset();
  _yajl_info->reset();

  return ret;
}

std::string ParserImpl::getError(bool verbose) {
  auto internal_error = _dispatcher.getError();

  if (internal_error.size() && !verbose) {
    return internal_error;
  }

  auto err = yajl_get_error(_yajl_info->handle(), static_cast<int>(verbose),
                            _data, _len);
  std::string yajl_error = reinterpret_cast<char *>(err);

  yajl_free_error(_yajl_info->handle(), err);

  return yajl_error + internal_error + "\n";
}

FieldName::FieldName(const std::string &str) : _str(str) {}

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

namespace std {
size_t hash<SJParser::FieldName>::operator()(
    const SJParser::FieldName &key) const {
  return hash<std::string>()(key.str());
}
basic_ostream<char> &operator<<(basic_ostream<char> &stream,
                                const SJParser::FieldName &name) {
  return stream << name.str();
}
}
