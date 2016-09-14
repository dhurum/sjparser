#pragma once

#include "sjparser_impl.h"

#include <array>
#include <functional>
#include <memory>
#include <string>
#include <tuple>
#include <unordered_map>
#include <vector>

namespace SJParser {

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

template <typename... Ts> class Object : public ObjectBase {
 public:
  using FieldArgs = std::tuple<FieldArg<Ts>...>;
  struct Args {
    Args(const FieldArgs &args,
         const std::function<bool(Object<Ts...> &)> &on_finish);
    Args(const FieldArgs &args);

    FieldArgs args;
    std::function<bool(Object<Ts...> &)> on_finish;
  };

  Object(const Args &args);
  Object(const Object &) = delete;

  template <size_t n, typename T, typename... TDs> struct NthType {
    using type = typename NthType<n - 1, TDs...>::type;
  };

  template <typename T, typename... TDs> struct NthType<0, T, TDs...> {
    using type = T;
  };

  template <size_t n> typename NthType<n, Ts...>::type &get();

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
          const FieldArgs &args);

    T _field;
  };

  virtual bool finish() override;

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
         const std::function<bool(SObject<T, Ts...> &, T &)> &make_value,
         const std::function<bool(const T &)> &on_finish);
    Args(const FieldArgs &args,
         const std::function<bool(SObject<T, Ts...> &, T &)> &make_value);

    FieldArgs args;
    std::function<bool(SObject<T, Ts...> &, T &)> make_value;
    std::function<bool(const T &)> on_finish;
  };

  SObject(const Args &args);
  SObject(const SObject &) = delete;

  using Object<Ts...>::get;
  Type &get();

 private:
  T _value;
  std::function<bool(SObject<T, Ts...> &, T &)> _make_value;
  std::function<bool(const T &)> _on_finish;

  virtual bool finish() override;
  virtual void reset() override;
};

template <typename T> class Array : public ArrayBase {
 public:
  struct Args {
    using EltArgs = typename T::Args;
    Args(const EltArgs &args, const std::function<bool()> &on_finish);
    Args(const EltArgs &args);

    EltArgs args;
    std::function<bool()> on_finish;
  };

  Array(const Args &args);
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
    Args(const EltArgs &args,
         const std::function<bool(const Type &)> &on_finish);
    Args(const EltArgs &args);
    Args() {}

    EltArgs args;
    std::function<bool(const Type &)> on_finish;
  };

  SArray(const Args &args);
  SArray(const SArray &) = delete;

  Type &get();

 private:
  std::vector<EltType> _values;
  std::function<bool(const Type &)> _on_finish;

  virtual void childParsed() override;
  virtual bool finish() override;
  virtual void reset() override;
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

/*Definitions*/

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

template <typename T>
FieldArg<T>::FieldArg(const std::string &name, const Args &value)
    : name(name), value(value) {}

template <typename T>
FieldArg<T>::FieldArg(const std::string &name) : name(name) {}

template <typename T> FieldArg<T>::FieldArg(const char *name) : name(name) {}

template <typename... Ts>
Object<Ts...>::Args::Args(const FieldArgs &args,
                          const std::function<bool(Object<Ts...> &)> &on_finish)
    : args(args), on_finish(on_finish) {}

template <typename... Ts>
Object<Ts...>::Args::Args(const FieldArgs &args) : args(args) {}

template <typename... Ts>
Object<Ts...>::Object(const Args &args)
    : _fields(_fields_array, _fields_map, args.args),
      _on_finish(args.on_finish) {}

template <typename... Ts>
template <size_t n>
typename Object<Ts...>::template NthType<n, Ts...>::type &Object<Ts...>::get() {
  return *reinterpret_cast<typename NthType<n, Ts...>::type *>(
      _fields_array[n]);
}

template <typename... Ts>
template <size_t n, typename T, typename... TDs>
Object<Ts...>::Field<n, T, TDs...>::Field(
    std::array<Token *, sizeof...(Ts)> &fields_array,
    std::unordered_map<std::string, Token *> &fields_map, const FieldArgs &args)
    : Field<n + 1, TDs...>(fields_array, fields_map, args),
      _field(std::get<n>(args).value) {
  fields_array[n] = &_field;
  fields_map[std::get<n>(args).name] = &_field;
}

template <typename... Ts> bool Object<Ts...>::finish() {
  if (!_on_finish) {
    return true;
  }
  return _on_finish(*this);
}

template <typename T, typename... Ts>
SObject<T, Ts...>::Args::Args(
    const FieldArgs &args,
    const std::function<bool(SObject<T, Ts...> &, T &)> &make_value,
    const std::function<bool(const T &)> &on_finish)
    : args(args), make_value(make_value), on_finish(on_finish) {}

template <typename T, typename... Ts>
SObject<T, Ts...>::Args::Args(
    const FieldArgs &args,
    const std::function<bool(SObject<T, Ts...> &, T &)> &make_value)
    : args(args), make_value(make_value) {}

template <typename T, typename... Ts>
SObject<T, Ts...>::SObject(const Args &args)
    : Object<Ts...>(args.args),
      _make_value(args.make_value),
      _on_finish(args.on_finish) {}

template <typename T, typename... Ts>
typename SObject<T, Ts...>::Type &SObject<T, Ts...>::get() {
  return _value;
}

template <typename T, typename... Ts> bool SObject<T, Ts...>::finish() {
  if (!_make_value(*this, _value)) {
    return false;
  }
  if (!_on_finish) {
    return true;
  }
  return _on_finish(_value);
}

template <typename T, typename... Ts> void SObject<T, Ts...>::reset() {
  ObjectBase::reset();
  _value = {};
}

template <typename T>
Array<T>::Args::Args(const EltArgs &args,
                     const std::function<bool()> &on_finish)
    : args(args), on_finish(on_finish) {}

template <typename T> Array<T>::Args::Args(const EltArgs &args) : args(args) {}

template <typename T>
Array<T>::Array(const Args &args)
    : ArrayBase(args.on_finish), _parser(args.args) {
  ArrayBase::_parser = &_parser;
}

template <typename T>
SArray<T>::Args::Args(const EltArgs &args,
                      const std::function<bool(const Type &)> &on_finish)
    : args(args), on_finish(on_finish) {}

template <typename T> SArray<T>::Args::Args(const EltArgs &args) : args(args) {}

template <typename T>
SArray<T>::SArray(const Args &args)
    : Array<T>(args.args), _on_finish(args.on_finish) {}

template <typename T> typename SArray<T>::Type &SArray<T>::get() {
  return _values;
}

template <typename T> void SArray<T>::childParsed() {
  _values.push_back(Array<T>::_parser.get());
}

template <typename T> bool SArray<T>::finish() {
  if (!_on_finish) {
    return true;
  }
  return _on_finish(_values);
}

template <typename T> void SArray<T>::reset() {
  _values.clear();
}
}
