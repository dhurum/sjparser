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
#include <tuple>

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

template <typename T> class Value : public Token {
 public:
  using Args = std::function<bool(const T &)>;
  using Type = T;

  Value(const Args &on_finish) : _on_finish(on_finish) {}
  virtual bool on(const T &value) override;
  virtual bool finish() override;
  const T &get();

 private:
  T _value;
  Args _on_finish;
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

  FieldArg(const std::string &name, const Args &value)
      : name(name), value(value) {}
  FieldArg(const std::string &name) : name(name) {}
  FieldArg(const char *name) : name(name) {}

  std::string name;
  Args value;
};

template <typename... Ts> class Object : public ObjectBase {
 public:
  using FieldArgs = std::tuple<FieldArg<Ts>...>;
  struct Args {
    Args(const FieldArgs &args,
         const std::function<bool(Object<Ts...> &)> &on_finish)
        : args(args), on_finish(on_finish) {}
    Args(const FieldArgs &args) : args(args) {}

    FieldArgs args;
    std::function<bool(Object<Ts...> &)> on_finish;
  };

  Object(const Args &args)
      : _fields(_fields_array, _fields_map, args.args),
        _on_finish(args.on_finish) {}

  Object(const Object &) = delete;

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
    Field(std::array<Token *, sizeof...(Ts)> & /*fields_array*/,
          std::unordered_map<std::string, Token *> & /*fields_map*/,
          const FieldArgs & /*args*/) {}
  };

  template <size_t n, typename T, typename... TDs>
  struct Field<n, T, TDs...> : private Field<n + 1, TDs...> {
    Field(std::array<Token *, sizeof...(Ts)> &fields_array,
          std::unordered_map<std::string, Token *> &fields_map,
          const FieldArgs &args)
        : Field<n + 1, TDs...>(fields_array, fields_map, args),
          _field(std::get<n>(args).value) {
      fields_array[n] = &_field;
      fields_map[std::get<n>(args).name] = &_field;
    }

    T _field;
  };

  std::array<Token *, sizeof...(Ts)> _fields_array;
  Field<0, Ts...> _fields;
  std::function<bool(Object<Ts...> &)> _on_finish;
};

template <typename T, typename... Ts> class SObject : public Object<Ts...> {
 public:
  using FieldArgs = typename Object<Ts...>::FieldArgs;
  using Type = T;

  struct Args {
    Args(const FieldArgs &args,
         const std::function<bool (SObject<T, Ts...> &, T&)> &make_value,
         const std::function<bool(const T&)> &on_finish)
        : args(args), make_value(make_value), on_finish(on_finish) {}
    Args(const FieldArgs &args,
         const std::function<bool (SObject<T, Ts...> &, T&)> &make_value)
        : args(args), make_value(make_value) {}

    FieldArgs args;
    std::function<bool (SObject<T, Ts...> &, T&)> make_value;
    std::function<bool(const T&)> on_finish;
  };

  SObject(const Args &args)
      : Object<Ts...>(args.args),
        _make_value(args.make_value), _on_finish(args.on_finish) {}

  SObject(const SObject &) = delete;

  using Object<Ts...>::get;

  Type &get() { 
    return _value;
  }

  virtual bool finish() override {
    if (!_make_value(*this, _value)) {
      return false;
    }
    if (!_on_finish) {
      return true;
    }
    return _on_finish(_value);
  }

  virtual void reset() override {
    ObjectBase::reset();
    _value = {};
  }

 private:
  T _value;
  std::function<bool (SObject<T, Ts...> &, T&)> _make_value;
  std::function<bool(const T&)> _on_finish;
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

template <typename T>
class Array : public ArrayBase {
 public:
  struct Args {
    using EltArgs = typename T::Args;
    Args(const EltArgs &args, const std::function<bool()> &on_finish)
        : args(args), on_finish(on_finish) {}
    Args(const EltArgs &args) : args(args) {}

    EltArgs args;
    std::function<bool()> on_finish;
  };

  Array(const Args &args) : ArrayBase(args.on_finish), _parser(args.args) {
    ArrayBase::_parser = &_parser;
  }

  Array(const Array &) = delete;

 protected:
  T _parser;
};

template <typename T> class SArray : public Array<T> {
 public:
  using EltType = typename T::Type;
  using Type = std::vector<EltType>;

  struct Args {
    using EltArgs = typename T::Args;
    Args(const EltArgs &args, const std::function<bool(const Type &)> &on_finish)
        : args(args), on_finish(on_finish) {}
    Args(const EltArgs &args) : args(args) {}
    Args() {}

    EltArgs args;
    std::function<bool(const Type &)> on_finish;
  };

  SArray(const Args &args)
      : Array<T>(args.args), _on_finish(args.on_finish) {}

  SArray(const SArray &) = delete;

  virtual void childParsed() override {
    _values.push_back(Array<T>::_parser.get());
  }

  Type &get() { return _values; }

  virtual bool finish() override {
    if (!_on_finish) {
      return true;
    }
    return _on_finish(_values);
  }

  virtual void reset() override {
    _values.clear();
  }

 private:
  std::vector<EltType> _values;
  std::function<bool(const Type &)> _on_finish;
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

template <typename T> class Parser {
 public:
  Parser(const typename T::Args &args = {})
      : _parser(args), _impl(std::make_unique<ParserImpl>(&_parser)) {}
  bool parse(const std::string &data) { return _impl->parse(data); }
  bool finish() { return _impl->finish(); }
  std::string getError() { return _impl->getError(); }
  T &parser() { return _parser; }

 private:
  T _parser;
  std::unique_ptr<ParserImpl> _impl;
};

template <typename T> bool Value<T>::on(const T &value) {
  _value = value;
  _set = true;

  return endParsing();
}

template <typename T> bool Value<T>::finish() {
  if (!_on_finish) {
    return true;
  }
  return _on_finish(_value);
}

template <typename T> const T &Value<T>::get() {
  if (!isSet()) {
    throw std::runtime_error("Can't get value, parser is unset");
  }
  return _value;
}
}
