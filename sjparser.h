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

#include "sjparser_impl.h"

#include <array>
#include <functional>
#include <memory>
#include <string>
#include <tuple>
#include <type_traits>
#include <unordered_map>
#include <vector>

namespace SJParser {

template <typename T> class Value : public TokenParser {
 public:
  using Args = std::function<bool(const T &)>;
  using Type = T;

  Value(const Args &on_finish) : _on_finish(on_finish) {}
  virtual bool on(const T &value) override;
  virtual bool finish() override;
  const T &get();
  template <typename U = T>
  typename std::enable_if<std::is_same<U, std::string>::value, U>::type &&pop();
  template <typename U = T>
  const typename std::enable_if<!std::is_same<U, std::string>::value, U>::type &
  pop();

 private:
  T _value;
  Args _on_finish;
};

template <typename T> struct FieldArg {
  using Args = typename T::Args;

  FieldArg(const std::string &name, const Args &value = {});
  template <typename U = T>
  FieldArg(const std::string &name, const typename U::ChildArgs &value);
  FieldArg(const char *name);

  std::string name;
  Args value;
};

template <typename... Ts> class Object : public ObjectParser {
 public:
  using ChildArgs = std::tuple<FieldArg<Ts>...>;
  struct Args {
    Args(const ChildArgs &args,
         const std::function<bool(Object<Ts...> &)> &on_finish = nullptr);

    ChildArgs args;
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
    Field(std::array<TokenParser *, sizeof...(Ts)> & /*fields_array*/,
          std::unordered_map<std::string, TokenParser *> & /*fields_map*/,
          const ChildArgs & /*args*/) {}
  };

  template <size_t n, typename T, typename... TDs>
  struct Field<n, T, TDs...> : private Field<n + 1, TDs...> {
    Field(std::array<TokenParser *, sizeof...(Ts)> &fields_array,
          std::unordered_map<std::string, TokenParser *> &fields_map,
          const ChildArgs &args);

    T _field;
  };

  virtual bool finish() override;

  std::array<TokenParser *, sizeof...(Ts)> _fields_array;
  Field<0, Ts...> _fields;
  std::function<bool(Object<Ts...> &)> _on_finish;
};

template <typename T, typename... Ts> class SObject : public Object<Ts...> {
 public:
  using ChildArgs = typename Object<Ts...>::ChildArgs;
  using Type = T;

  struct Args {
    Args(const ChildArgs &args,
         const std::function<bool(SObject<T, Ts...> &, T &)> &on_finish);

    ChildArgs args;
    std::function<bool(SObject<T, Ts...> &, T &)> on_finish;
  };

  SObject(const Args &args);
  SObject(const SObject &) = delete;

  using Object<Ts...>::get;
  Type &get();
  Type &&pop();

 private:
  T _value;
  std::function<bool(SObject<T, Ts...> &, T &)> _on_finish;
  using TokenParser::_set;
  using TokenParser::checkSet;

  virtual bool finish() override;
  virtual void reset() override;
};

template <typename T> class Array : public ArrayParser {
 public:
  using ChildArgs = typename T::Args;

  struct Args {
    Args(const ChildArgs &args,
         const std::function<bool()> &on_finish = nullptr);

    ChildArgs args;
    std::function<bool()> on_finish;
  };

  Array(const Args &args);
  Array(const Array &) = delete;

 protected:
  T _parser;
};

template <typename T> class SArray : public Array<T> {
 public:
  using ChildArgs = typename T::Args;
  using EltType = typename T::Type;
  using Type = std::vector<EltType>;
  using CallbackType = std::function<bool(const Type &)>;

  struct Args {
    Args(const ChildArgs &args = {}, const CallbackType &on_finish = nullptr);
    Args(const CallbackType &on_finish);

    ChildArgs args;
    std::function<bool(const Type &)> on_finish;
  };

  SArray(const Args &args);
  SArray(const SArray &) = delete;

  Type &get();
  Type &&pop();

 private:
  std::vector<EltType> _values;
  std::function<bool(const Type &)> _on_finish;
  using TokenParser::_set;
  using TokenParser::checkSet;

  virtual void childParsed() override;
  virtual bool finish() override;
  virtual void reset() override;
};

template <typename T> class Parser {
 public:
  Parser(const typename T::Args &args = {});
  template <typename U = T> Parser(const typename U::ChildArgs &args);
  template <typename U = T> Parser(const typename U::CallbackType &callback);
  bool parse(const std::string &data);
  bool finish();
  std::string getError();
  T &parser();

 private:
  T _parser;
  std::unique_ptr<ParserImpl> _impl;
};

/*Definitions*/

template <typename T> bool Value<T>::on(const T &value) {
  _value = value;

  return endParsing();
}

template <typename T> bool Value<T>::finish() {
  if (!_on_finish) {
    return true;
  }
  return _on_finish(_value);
}

template <typename T> const T &Value<T>::get() {
  checkSet();
  return _value;
}

template <typename T>
template <typename U>
typename std::enable_if<std::is_same<U, std::string>::value, U>::type &&
Value<T>::pop() {
  checkSet();
  _set = false;
  return std::move(_value);
}

template <typename T>
template <typename U>
const typename std::enable_if<!std::is_same<U, std::string>::value, U>::type &
Value<T>::pop() {
  checkSet();
  _set = false;
  return _value;
}

template <typename T>
FieldArg<T>::FieldArg(const std::string &name, const Args &value)
    : name(name), value(value) {}

template <typename T>
template <typename U>
FieldArg<T>::FieldArg(const std::string &name,
                      const typename U::ChildArgs &value)
    : name(name), value(value) {}

template <typename T> FieldArg<T>::FieldArg(const char *name) : name(name) {}

template <typename... Ts>
Object<Ts...>::Args::Args(const ChildArgs &args,
                          const std::function<bool(Object<Ts...> &)> &on_finish)
    : args(args), on_finish(on_finish) {}

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
    std::array<TokenParser *, sizeof...(Ts)> &fields_array,
    std::unordered_map<std::string, TokenParser *> &fields_map,
    const ChildArgs &args)
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
    const ChildArgs &args,
    const std::function<bool(SObject<T, Ts...> &, T &)> &on_finish)
    : args(args), on_finish(on_finish) {}

template <typename T, typename... Ts>
SObject<T, Ts...>::SObject(const Args &args)
    : Object<Ts...>(args.args), _on_finish(args.on_finish) {}

template <typename T, typename... Ts>
typename SObject<T, Ts...>::Type &SObject<T, Ts...>::get() {
  checkSet();
  return _value;
}

template <typename T, typename... Ts>
typename SObject<T, Ts...>::Type &&SObject<T, Ts...>::pop() {
  checkSet();
  _set = false;
  return std::move(_value);
}

template <typename T, typename... Ts> bool SObject<T, Ts...>::finish() {
  return _on_finish(*this, _value);
}

template <typename T, typename... Ts> void SObject<T, Ts...>::reset() {
  ObjectParser::reset();
  _value = Type();
}

template <typename T>
Array<T>::Args::Args(const ChildArgs &args,
                     const std::function<bool()> &on_finish)
    : args(args), on_finish(on_finish) {}

template <typename T>
Array<T>::Array(const Args &args)
    : ArrayParser(args.on_finish), _parser(args.args) {
  ArrayParser::_parser = &_parser;
}

template <typename T>
SArray<T>::Args::Args(const ChildArgs &args,
                      const std::function<bool(const Type &)> &on_finish)
    : args(args), on_finish(on_finish) {}

template <typename T>
SArray<T>::Args::Args(const std::function<bool(const Type &)> &on_finish)
    : on_finish(on_finish) {}

template <typename T>
SArray<T>::SArray(const Args &args)
    : Array<T>(args.args), _on_finish(args.on_finish) {}

template <typename T> typename SArray<T>::Type &SArray<T>::get() {
  checkSet();
  return _values;
}

template <typename T> typename SArray<T>::Type &&SArray<T>::pop() {
  checkSet();
  _set = false;
  return std::move(_values);
}

template <typename T> void SArray<T>::childParsed() {
  _values.push_back(Array<T>::_parser.pop());
}

template <typename T> bool SArray<T>::finish() {
  if (!_on_finish) {
    return true;
  }
  return _on_finish(_values);
}

template <typename T> void SArray<T>::reset() {
  _values = Type();
}

template <typename T>
Parser<T>::Parser(const typename T::Args &args)
    : _parser(args), _impl(std::make_unique<ParserImpl>(&_parser)) {}

template <typename T>
template <typename U>
Parser<T>::Parser(const typename U::ChildArgs &args)
    : _parser(args), _impl(std::make_unique<ParserImpl>(&_parser)) {}

template <typename T>
template <typename U>
Parser<T>::Parser(const typename U::CallbackType &callback)
    : _parser(callback), _impl(std::make_unique<ParserImpl>(&_parser)) {}

template <typename T> bool Parser<T>::parse(const std::string &data) {
  return _impl->parse(data);
}

template <typename T> bool Parser<T>::finish() {
  return _impl->finish();
}

template <typename T> std::string Parser<T>::getError() {
  return _impl->getError();
}

template <typename T> T &Parser<T>::parser() {
  return _parser;
}
}
