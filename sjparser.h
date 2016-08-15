#pragma once

#include <yajl/yajl_parse.h>
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
  virtual void setDispatcher(Dispatcher *dispatcher);
  bool isSet();
  void reset();
  bool endParsing();

  virtual bool on(const bool &value) { return false; }
  virtual bool on(const int64_t &value) { return false; }
  virtual bool on(const double &value) { return false; }
  virtual bool on(const std::string &value) { return false; }
  virtual bool on(const MapStartT) { return false; }
  virtual bool on(const MapKeyT &key) { return false; }
  virtual bool on(const MapEndT) { return false; }
  virtual bool on(const ArrayStartT) { return false; }
  virtual bool on(const ArrayEndT) { return false; }

  virtual bool finish() { return true; }

  virtual ~TokenParser() = default;

 protected:
  Dispatcher *_dispatcher = nullptr;
  bool _set = false;
};

class Dispatcher {
 public:
  Dispatcher(TokenParser *parser);
  void pushParser(TokenParser *parser);
  void popParser();

  template <typename T> bool on(const T &value);

 protected:
  std::stack<TokenParser *> _parsers;
  std::function<void()> _on_completion;
};

template <typename T> class ValueParser : public TokenParser {
 public:
  ValueParser(std::function<bool(const T &)> on_finish = nullptr)
      : _on_finish(on_finish) {}
  virtual bool on(const T &value) override;
  virtual bool finish() override;
  const T &get();

 private:
  T _value;
  std::function<bool(const T &)> _on_finish;
};

class ObjectParser : public TokenParser {
 public:
  virtual void setDispatcher(Dispatcher *dispatcher) override;

  virtual bool on(const MapStartT) override;
  virtual bool on(const MapKeyT &key) override;
  virtual bool on(const MapEndT) override;

  virtual ~ObjectParser() = default;

 protected:
  void addField(const std::string &name, TokenParser *parser);

 private:
  std::unordered_map<std::string, TokenParser *> _fields;
};

class ArrayParser : public TokenParser {
 public:
  virtual bool on(const bool &value) override;
  virtual bool on(const int64_t &value) override;
  virtual bool on(const double &value) override;
  virtual bool on(const std::string &value) override;
  virtual bool on(const MapStartT) override;
  virtual bool on(const ArrayStartT) override;
  virtual bool on(const ArrayEndT) override;

  virtual ~ArrayParser() = default;

 protected:
  void setElementsParser(TokenParser *parser);

 private:
  TokenParser *_parser;
  bool _started = false;
};

class Parser {
 public:
  Parser(std::shared_ptr<TokenParser> parser);
  ~Parser();
  bool parse(const std::string &data);
  bool finish();
  std::string getError();

 protected:
  const yajl_callbacks _callbacks;
  yajl_handle _handle;
  std::shared_ptr<TokenParser> _parser;
  Dispatcher _dispatcher;
};

template <typename T> bool ValueParser<T>::on(const T &value) {
  _value = value;
  _set = true;

  return endParsing();
}

template <typename T> bool ValueParser<T>::finish() {
  if (!_on_finish) {
    return true;
  }
  return _on_finish(_value);
}

template <typename T> const T &ValueParser<T>::get() {
  // TODO: exception
  return _value;
}
}
