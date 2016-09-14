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

  virtual ~TokenParser() = default;

 protected:
  Dispatcher *_dispatcher = nullptr;
  bool _set = false;
};

class ObjectParser : public TokenParser {
 public:
  virtual void setDispatcher(Dispatcher *dispatcher) override;
  virtual void reset() override;

  virtual bool on(const MapStartT) override;
  virtual bool on(const MapKeyT &key) override;
  virtual bool on(const MapEndT) override;

 protected:
  std::unordered_map<std::string, TokenParser *> _fields_map;
};

class ArrayParser : public TokenParser {
 public:
  ArrayParser(std::function<bool()> on_finish) : _on_finish(on_finish) {}
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

 protected:
  std::stack<TokenParser *> _parsers;
  std::function<void()> _on_completion;
};

struct YajlInfo;

class ParserImpl {
 public:
  ParserImpl(TokenParser *parser);
  ~ParserImpl();
  bool parse(const std::string &data);
  bool finish();
  std::string getError();

 private:
  std::unique_ptr<YajlInfo> _yajl_info;
  Dispatcher _dispatcher;
};
}
