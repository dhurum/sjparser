#pragma once

#include <yajl/yajl_parse.h>
#include <array>
#include <functional>
#include <initializer_list>
#include <memory>
#include <stack>
#include <string>
#include <unordered_map>
#include <vector>

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

  virtual bool on(const bool & /*value*/) { return false; }
  virtual bool on(const int64_t & /*value*/) { return false; }
  virtual bool on(const double & /*value*/) { return false; }
  virtual bool on(const std::string & /*value*/) { return false; }
  virtual bool on(const MapStartT) { return false; }
  virtual bool on(const MapKeyT & /*key*/) { return false; }
  virtual bool on(const MapEndT) { return false; }
  virtual bool on(const ArrayStartT) { return false; }
  virtual bool on(const ArrayEndT) { return false; }

  virtual bool finish() { return true; }

  virtual ~TokenParser() = default;

 protected:
  Dispatcher *_dispatcher = nullptr;
  bool _set = false;
};

template <typename T> class ValueParser : public TokenParser {
 public:
  using Args = std::function<bool(const T &)>;

  ValueParser(const Args &on_finish) : _on_finish(on_finish) {}
  virtual bool on(const T &value) override;
  virtual bool finish() override;
  const T &get();
  const T &pop();

 private:
  T _value;
  Args _on_finish;
};

class ObjectParserBase : public TokenParser {
 public:
  virtual void setDispatcher(Dispatcher *dispatcher) override;
  virtual void reset() override;

  virtual bool on(const MapStartT) override;
  virtual bool on(const MapKeyT &key) override;
  virtual bool on(const MapEndT) override;

 protected:
  std::unordered_map<std::string, TokenParser *> _fields_map;
};

template <typename T> struct ObjectArg {
  using Args = typename T::Args;

  ObjectArg(const std::string &name, const Args &value)
      : name(name), value(value) {}
  ObjectArg(const std::string &name) : name(name) {}
  ObjectArg(const char *name) : name(name) {}

  std::string name;
  Args value = nullptr;
};

template <typename...> struct ObjectArgs {};

template <typename T, typename... Ts>
struct ObjectArgs<T, Ts...> : private ObjectArgs<Ts...> {
  ObjectArgs(ObjectArg<T> arg, ObjectArg<Ts>... ts)
      : ObjectArgs<Ts...>(ts...), _name(arg.name), _value(arg.value) {}

  template <size_t n, typename TD, typename... TDs> struct NthType {
    using type = typename NthType<n - 1, TDs...>::type;
  };

  template <typename TD, typename... TDs> struct NthType<0, TD, TDs...> {
    using type = typename TD::Args;
  };

  template <size_t n>
  typename std::enable_if<n == 0,
                          const typename NthType<n, T, Ts...>::type &>::type
  getValue() const {
    return _value;
  }

  template <size_t n>
  typename std::enable_if<n != 0,
                          const typename NthType<n, T, Ts...>::type &>::type
  getValue() const {
    const ObjectArgs<Ts...> &base = *this;
    return base.template getValue<n - 1>();
  }

  template <size_t n>
  typename std::enable_if<n == 0, const std::string &>::type getName() const {
    return _name;
  }

  template <size_t n>
  typename std::enable_if<n != 0, const std::string &>::type getName() const {
    const ObjectArgs<Ts...> &base = *this;
    return base.template getName<n - 1>();
  }

  std::string _name;
  typename T::Args _value;
};

template <typename... Ts> class ObjectParser : public ObjectParserBase {
 public:
  using FieldArgs = ObjectArgs<Ts...>;
  struct Args {
    Args(const FieldArgs &args,
         const std::function<bool(ObjectParser<Ts...> &)> &on_finish)
        : args(args), on_finish(on_finish) {}
    Args(const FieldArgs &args) : args(args) {}

    FieldArgs args;
    std::function<bool(ObjectParser<Ts...> &)> on_finish = nullptr;
  };

  ObjectParser(const Args &args)
      : _fields(_fields_array, _fields_map, args.args),
        _on_finish(args.on_finish) {}

  ObjectParser(const ObjectParser &) = delete;

  virtual bool finish() override {
    if (!_on_finish) {
      return true;
    }
    return _on_finish(*this);
  }

  template <size_t n, typename T, typename... TDs> struct NthType {
    using type = typename NthType<n - 1, TDs...>::type;
  };

  template <typename T, typename... TDs> struct NthType<0, T, TDs...> {
    using type = T;
  };

  template <size_t n> typename NthType<n, Ts...>::type &get() {
    return *reinterpret_cast<typename NthType<n, Ts...>::type *>(
        _fields_array[n]);
  }

 private:
  template <size_t, typename...> struct Field {
    Field(std::array<TokenParser *, sizeof...(Ts)> & /*fields_array*/,
          std::unordered_map<std::string, TokenParser *> & /*fields_map*/,
          const FieldArgs & /*args*/) {}
  };

  template <size_t n, typename T, typename... TDs>
  struct Field<n, T, TDs...> : private Field<n + 1, TDs...> {
    Field(std::array<TokenParser *, sizeof...(Ts)> &fields_array,
          std::unordered_map<std::string, TokenParser *> &fields_map,
          const FieldArgs &args)
        : Field<n + 1, TDs...>(fields_array, fields_map, args),
          _field(args.template getValue<n>()) {
      fields_array[n] = &_field;
      fields_map[args.template getName<n>()] = &_field;
    }

    T _field;
  };

  std::array<TokenParser *, sizeof...(Ts)> _fields_array;
  Field<0, Ts...> _fields;
  std::function<bool(ObjectParser<Ts...> &)> _on_finish;
};

class ArrayParserBase : public TokenParser {
 public:
  ArrayParserBase(std::function<bool()> on_finish) : _on_finish(on_finish) {}
  virtual void reset() override;

  virtual bool on(const bool &value) override;
  virtual bool on(const int64_t &value) override;
  virtual bool on(const double &value) override;
  virtual bool on(const std::string &value) override;
  virtual bool on(const MapStartT) override;
  virtual bool on(const ArrayStartT) override;
  virtual bool on(const ArrayEndT) override;

  virtual bool finish() override {
    if (!_on_finish) {
      return true;
    }
    return _on_finish();
  }

 protected:
  TokenParser *_parser;
  std::function<bool()> _on_finish;

 private:
  bool _started = false;
};

// TODO: maybe add some template parameter for internal vector
template <typename T> class ArrayParser : public ArrayParserBase {
 public:
  struct Args {
    using EltArgs = typename T::Args;
    Args(const EltArgs &args, const std::function<bool()> &on_finish)
        : args(args), on_finish(on_finish) {}
    Args(const EltArgs &args) : args(args) {}

    EltArgs args;
    std::function<bool()> on_finish = nullptr;
  };

  ArrayParser(const Args &args)
      : ArrayParserBase(args.on_finish), _parser(args.args) {
    ArrayParserBase::_parser = &_parser;
  }

 private:
  T _parser;
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

class ParserImpl {
 public:
  ParserImpl(TokenParser *parser);
  ~ParserImpl();
  bool parse(const std::string &data);
  bool finish();
  std::string getError();

 private:
  const yajl_callbacks _callbacks;
  yajl_handle _handle;
  Dispatcher _dispatcher;
};

template <typename T> class Parser {
 public:
  Parser(const typename T::Args &args = nullptr)
      : _parser(args), _impl(std::make_unique<ParserImpl>(&_parser)) {}
  bool parse(const std::string &data) { return _impl->parse(data); }
  bool finish() { return _impl->finish(); }
  std::string getError() { return _impl->getError(); }
  T &parser() { return _parser; }

 private:
  T _parser;
  std::unique_ptr<ParserImpl> _impl;
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
  if (!isSet()) {
    throw std::runtime_error("Can't get value, parser is unset");
  }
  return _value;
}

template <typename T> const T &ValueParser<T>::pop() {
  auto &ret = get();
  reset();
  return ret;
}
}
