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

#pragma once

#include <functional>
#include <memory>
#include <stack>
#include <string>
#include <unordered_map>

namespace SJParser {

struct MapStartT {};
struct MapKeyT {
  const std::string &key;
};
struct MapEndT {};
struct ArrayStartT {};
struct ArrayEndT {};

class Dispatcher;

class TokenParser {
 public:
  virtual void setDispatcher(Dispatcher *dispatcher) noexcept;
  inline bool isSet() const noexcept;
  virtual void reset() noexcept;
  bool endParsing();
  virtual bool finish() = 0;

  virtual bool on(const bool & /*value*/);
  virtual bool on(const int64_t & /*value*/);
  virtual bool on(const double & /*value*/);
  virtual bool on(const std::string & /*value*/);
  virtual bool on(const MapStartT);
  virtual bool on(const MapKeyT & /*key*/);
  virtual bool on(const MapEndT);
  virtual bool on(const ArrayStartT);
  virtual bool on(const ArrayEndT);

  virtual void childParsed() {}

  virtual ~TokenParser() = default;

 protected:
  Dispatcher *_dispatcher = nullptr;
  bool _set = false;

  inline void checkSet() const;

  inline bool unexpectedToken(const std::string &type);
};

class ObjectParser : public TokenParser {
 public:
  virtual void setDispatcher(Dispatcher *dispatcher) noexcept override;
  virtual void reset() noexcept override;

  virtual bool on(const MapStartT) noexcept override;
  virtual bool on(const MapKeyT &key) noexcept override;
  virtual bool on(const MapEndT) override;

 protected:
  std::unordered_map<std::string, TokenParser *> _fields_map;
};

class ArrayParser : public TokenParser {
 public:
  ArrayParser(std::function<bool()> on_finish) : _on_finish(on_finish) {}
  virtual void reset() noexcept override;

  virtual bool on(const bool &value) override;
  virtual bool on(const int64_t &value) override;
  virtual bool on(const double &value) override;
  virtual bool on(const std::string &value) override;
  virtual bool on(const MapStartT) override;
  virtual bool on(const ArrayStartT) override;
  virtual bool on(const ArrayEndT) override;

  virtual bool finish() override;

 protected:
  TokenParser *_parser;
  std::function<bool()> _on_finish;

 private:
  bool _started = false;
};

class Dispatcher {
 public:
  Dispatcher(TokenParser *parser);
  void pushParser(TokenParser *parser);
  void popParser();

  template <typename T> bool on(const T &value);

  inline void setError(const std::string &error);
  inline std::string &getError() noexcept;

 protected:
  std::stack<TokenParser *> _parsers;
  std::function<void()> _on_completion;
  std::string _error;
};

struct YajlInfo;

class ParserImpl {
 public:
  ParserImpl(TokenParser *parser);
  ~ParserImpl();
  bool parse(const char *data, size_t len);
  bool finish();
  std::string getError(bool verbose);

 private:
  std::unique_ptr<YajlInfo> _yajl_info;
  Dispatcher _dispatcher;
  const unsigned char *_data;
  size_t _len;
};

/******************************** Definitions ********************************/

bool TokenParser::isSet() const noexcept {
  return _set;
}

void TokenParser::checkSet() const {
  if (!isSet()) {
    throw std::runtime_error("Can't get value, parser is unset");
  }
}

bool TokenParser::unexpectedToken(const std::string &type) {
  _dispatcher->setError("Unexpected token " + type);
  return false;
}

void Dispatcher::setError(const std::string &error) {
  _error = error;
}

std::string &Dispatcher::getError() noexcept {
  return _error;
}
}
