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

class Token {
 public:
  virtual void setDispatcher(Dispatcher *dispatcher);
  bool isSet();
  virtual void reset();
  bool endParsing();
  virtual bool finish() = 0;

  virtual bool on(const bool & /*value*/) { return false; }
  virtual bool on(const int64_t & /*value*/) { return false; }
  virtual bool on(const double & /*value*/) { return false; }
  virtual bool on(const std::string & /*value*/) { return false; }
  virtual bool on(const MapStartT) { return false; }
  virtual bool on(const MapKeyT & /*key*/) { return false; }
  virtual bool on(const MapEndT) { return false; }
  virtual bool on(const ArrayStartT) { return false; }
  virtual bool on(const ArrayEndT) { return false; }

  virtual void childParsed() {}

  virtual ~Token() = default;

 protected:
  Dispatcher *_dispatcher = nullptr;
  bool _set = false;
};

class ObjectBase : public Token {
 public:
  virtual void setDispatcher(Dispatcher *dispatcher) override;
  virtual void reset() override;

  virtual bool on(const MapStartT) override;
  virtual bool on(const MapKeyT &key) override;
  virtual bool on(const MapEndT) override;

 protected:
  std::unordered_map<std::string, Token *> _fields_map;
};

template <typename T> struct FieldArg {
  using Args = typename T::Args;

  FieldArg(const std::string &name, const Args &value);
  FieldArg(const std::string &name);
  FieldArg(const char *name);

  std::string name;
  Args value;
};

class ArrayBase : public Token {
 public:
  ArrayBase(std::function<bool()> on_finish) : _on_finish(on_finish) {}
  virtual void reset() override;

  virtual bool on(const bool &value) override;
  virtual bool on(const int64_t &value) override;
  virtual bool on(const double &value) override;
  virtual bool on(const std::string &value) override;
  virtual bool on(const MapStartT) override;
  virtual bool on(const ArrayStartT) override;
  virtual bool on(const ArrayEndT) override;

  virtual bool finish() override;

 protected:
  Token *_parser;
  std::function<bool()> _on_finish;

 private:
  bool _started = false;
};

class Dispatcher {
 public:
  Dispatcher(Token *parser);
  void pushParser(Token *parser);
  void popParser();

  template <typename T> bool on(const T &value);

 protected:
  std::stack<Token *> _parsers;
  std::function<void()> _on_completion;
};

class ParserImpl {
 public:
  ParserImpl(Token *parser);
  ~ParserImpl();
  bool parse(const std::string &data);
  bool finish();
  std::string getError();

 private:
  const yajl_callbacks _callbacks;
  yajl_handle _handle;
  Dispatcher _dispatcher;
};
}
