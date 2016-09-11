#include "sjparser.h"

using namespace SJParser;

void TokenParser::setDispatcher(Dispatcher *dispatcher) {
  _dispatcher = dispatcher;
}

bool TokenParser::isSet() {
  return _set;
}

void TokenParser::reset() {
  _set = false;
}

bool TokenParser::endParsing() {
  if (_dispatcher) {
    _dispatcher->popParser();
  }
  return finish();
}

void ObjectParserBase::setDispatcher(Dispatcher *dispatcher) {
  TokenParser::setDispatcher(dispatcher);
  for (auto &field : _fields_map) {
    field.second->setDispatcher(dispatcher);
  }
}

void ObjectParserBase::reset() {
  for (auto &field : _fields_map) {
    field.second->reset();
  }
}

bool ObjectParserBase::on(const MapStartT) {
  reset();
  return true;
}

bool ObjectParserBase::on(const MapKeyT &key) {
  try {
    auto &parser = _fields_map.at(key.key);
    _dispatcher->pushParser(parser);
  } catch (...) {
    return false;
  }
  return true;
}

bool ObjectParserBase::on(const MapEndT) {
  return endParsing();
}

void ArrayParserBase::reset() {
  _parser->reset();
}

bool ArrayParserBase::on(const bool &value) {
  return _parser->on(value);
}

bool ArrayParserBase::on(const int64_t &value) {
  return _parser->on(value);
}

bool ArrayParserBase::on(const double &value) {
  return _parser->on(value);
}

bool ArrayParserBase::on(const std::string &value) {
  return _parser->on(value);
}

bool ArrayParserBase::on(const MapStartT) {
  _parser->setDispatcher(_dispatcher);
  _dispatcher->pushParser(_parser);
  return _parser->on(MapStartT{});
}

bool ArrayParserBase::on(const ArrayStartT) {
  if (!_started) {
    _started = true;
    return true;
  }

  _parser->setDispatcher(_dispatcher);
  _dispatcher->pushParser(_parser);
  return _parser->on(ArrayStartT{});
}

bool ArrayParserBase::on(const ArrayEndT) {
  _started = false;
  return endParsing();
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
}

template <typename T>
bool Dispatcher::on(const T &value) {
  if (_parsers.empty()) {
    return false;
  }
  return _parsers.top()->on(value);
}

static int yajl_boolean(void *ctx, int value) {
  auto dispatcher = reinterpret_cast<Dispatcher *>(ctx);
  return dispatcher->on(static_cast<bool>(value));
}

static int yajl_integer(void *ctx, long long value) {
  auto dispatcher = reinterpret_cast<Dispatcher *>(ctx);
  return dispatcher->on(static_cast<int64_t>(value));
}

static int yajl_double(void *ctx, double value) {
  auto dispatcher = reinterpret_cast<Dispatcher *>(ctx);
  return dispatcher->on(value);
}

static int yajl_string(void *ctx, const unsigned char *value, size_t len) {
  auto dispatcher = reinterpret_cast<Dispatcher *>(ctx);
  return dispatcher->on(
      std::string(reinterpret_cast<const char *>(value), len));
}

static int yajl_start_map(void *ctx) {
  auto dispatcher = reinterpret_cast<Dispatcher *>(ctx);
  return dispatcher->on(MapStartT{});
}

static int yajl_map_key(void *ctx, const unsigned char *value, size_t len) {
  auto dispatcher = reinterpret_cast<Dispatcher *>(ctx);
  return dispatcher->on(
      MapKeyT{std::string(reinterpret_cast<const char *>(value), len)});
}

static int yajl_end_map(void *ctx) {
  auto dispatcher = reinterpret_cast<Dispatcher *>(ctx);
  return dispatcher->on(MapEndT{});
}

static int yajl_start_array(void *ctx) {
  auto dispatcher = reinterpret_cast<Dispatcher *>(ctx);
  return dispatcher->on(ArrayStartT{});
}

static int yajl_end_array(void *ctx) {
  auto dispatcher = reinterpret_cast<Dispatcher *>(ctx);
  return dispatcher->on(ArrayEndT{});
}

ParserImpl::ParserImpl(TokenParser *parser)
    : _callbacks{nullptr,      yajl_boolean,     yajl_integer,   yajl_double,
                 nullptr,      yajl_string,      yajl_start_map, yajl_map_key,
                 yajl_end_map, yajl_start_array, yajl_end_array},
      _dispatcher(parser) {
  _handle = yajl_alloc(&_callbacks, nullptr, &_dispatcher);
}

ParserImpl::~ParserImpl() {
  yajl_free(_handle);
}

bool ParserImpl::parse(const std::string &data) {
  if (yajl_parse(_handle, reinterpret_cast<const unsigned char *>(data.data()),
                 data.size()) != yajl_status_ok) {
    return false;
  }
  return true;
}

bool ParserImpl::finish() {
  if (yajl_complete_parse(_handle) != yajl_status_ok) {
    return false;
  }
  return true;
}

std::string ParserImpl::getError() {
  auto err = yajl_get_error(_handle, 0, nullptr, 0);
  std::string error_str = reinterpret_cast<char *>(err);

  yajl_free_error(_handle, err);
  return error_str;
}
