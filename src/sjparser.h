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

/*
 * Simple value parser.
 * T can be:
 *   - std::string
 *   - int64_t
 *   - bool
 *   - double
 */
template <typename T> class Value : public TokenParser {
 public:
  using Type = T;
  using Args = std::function<bool(const Type &)>;

  // After value is parsed, on_finish callback is called.
  Value(const Args &on_finish) : _on_finish(on_finish) {}
  // Method to check if value is set, returns boolean.
  using TokenParser::isSet;
  // Returns a reference to parsed value.  If value is unset, throws
  // std::runtime_error.
  const Type &get();
  template <typename U = Type>
  // Returns an rvalue reference to value for std::string and reference for
  // other
  // types. After call of this method value is unset.
  typename std::enable_if<std::is_same<U, std::string>::value, U>::type &&pop();
  template <typename U = Type>
  const typename std::enable_if<!std::is_same<U, std::string>::value, U>::type &
  pop();

 private:
  Type _value;
  Args _on_finish;

  virtual bool on(const Type &value) override;
  virtual bool finish() override;
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

/*
 * Object parser.
 * Ts is list of any entity parsers.
 */

template <typename... Ts> class Object : public ObjectParser {
 public:
  using ChildArgs = std::tuple<FieldArg<Ts>...>;
  struct Args {
    Args(const ChildArgs &args,
         const std::function<bool(Object<Ts...> &)> &on_finish = nullptr);

    ChildArgs args;
    std::function<bool(Object<Ts...> &)> on_finish;
  };

  /*
   * Constructor receives a structure.
   * It's first element is a tuple of object fields arguments,
   * and second element is a callback that will be called after the object is
   * parsed.
   * If you do not specify the callback - you can provide only a tuple of object
   * fields arguments to constructor.
   * Field argument is a structure where a first element is a string with field
   * name, and a second field is an argument for respective parser.
   * If you do not want to provide an argument to a field parser - you can
   * provide only name.
   * For example: {"field1", {"field2", ...}, "field3"}
   * Callback is called with a reference to this parser as an argument.
   */
  Object(const Args &args);
  Object(const Object &) = delete;

  template <size_t n, typename T, typename... TDs> struct NthType {
    using type = typename NthType<n - 1, TDs...>::type;
  };

  template <typename T, typename... TDs> struct NthType<0, T, TDs...> {
    using type = T;
  };

  // Returns reference to a parser of n-th field.
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

/*
 * Object parser, that can store result.
 * T is type of result value (structure or object). It must have default
 * constructor. If you want to store it in SArray it should have move
 * constructor, to avoid unnecessary copy operations.
 * Ts is list of any parsers.
 */

template <typename T, typename... Ts> class SObject : public Object<Ts...> {
 public:
  using ChildArgs = typename Object<Ts...>::ChildArgs;
  using Type = T;

  struct Args {
    Args(const ChildArgs &args,
         const std::function<bool(SObject<Type, Ts...> &, Type &)> &on_finish);

    ChildArgs args;
    std::function<bool(SObject<Type, Ts...> &, Type &)> on_finish;
  };

  /*
   * Constructor receives a structure.
   * It's first element is a tuple of object fields arguments,
   * and second element is a callback that will be called after the object is
   * parsed.
   * If you do not specify the callback - you can provide only tuple of object
   * fields arguments to constructor.
   * Field argument is a structure where a first element is a string with field
   * name, and a second field is an argument for respective parser.
   * If you do not want to provide an argument to a field parser - you can
   * provide only name.
   * For example: {"field1", {"field2", ...}, "field3"}
   * Callback is called with a reference to this parser as an argument and a
   * reference to internal value. You should set it in this callback.
   */
  SObject(const Args &args);
  SObject(const SObject &) = delete;

  // Method to check if value is set, returns boolean.
  using TokenParser::isSet;
  // Returns reference to a parser of n-th field.
  using Object<Ts...>::get;
  // Returns reference to parsed value. If value is unset, throws
  // std::runtime_error.
  Type &get();
  // Returns an rvalue reference to value. After call of this method value is
  // unset.
  Type &&pop();

 private:
  T _value;
  std::function<bool(SObject<T, Ts...> &, T &)> _on_finish;
  using TokenParser::_set;
  using TokenParser::checkSet;

  virtual bool finish() override;
  virtual void reset() override;
};

/*
 * Array parser.
 * T is any parser.
 */

template <typename T> class Array : public ArrayParser {
 public:
  using ChildArgs = typename T::Args;

  struct Args {
    Args(const ChildArgs &args,
         const std::function<bool()> &on_finish = nullptr);

    ChildArgs args;
    std::function<bool()> on_finish;
  };

  /*
   * Constructor receives a structure.
   * It's first element is an argument of T's constructor,
   * and second element is a callback that will be called after the array is
   * parsed.
   * If you do not specify the callback - you can provide only T's argument to
   * constructor.
   * Callback is called without arguments.
   */
  Array(const Args &args);
  Array(const Array &) = delete;

 protected:
  T _parser;
};

/*
 * Array parser, that stores a vector of T's values.
 * T is any parser.
 */

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

  /*
   * Constructor receives a structure.
   * It's first element is an argument of T's constructor,
   * and second element is a callback that will be called after the array is
   * parsed.
   * If you do not specify the callback - you can provide only T's argument to
   * constructor.
   * If you do not specify T's arguments - you can specify callback only.
   * Callback is called with a reference to this parser as argument.
   */
  SArray(const Args &args);
  SArray(const SArray &) = delete;

  // Method to check if vector of values is set, returns boolean.
  using TokenParser::isSet;
  // Returns reference to vecor of values. If vector is unset, throws
  // std::runtime_error.
  Type &get();
  // Returns an rvalue reference to vector of values. After call of this method
  // value is unset.
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

/*
 * Main parser.
 * T is one of the entities parsers (value, object, array).
 */

template <typename T> class Parser {
 public:
  /*
   * Constructor receives same arguments as T's constructor.
   */
  Parser(const typename T::Args &args = {});
  template <typename U = T> Parser(const typename U::ChildArgs &args);
  template <typename U = T> Parser(const typename U::CallbackType &callback);
  // Parse a piece of json. Returns false in case of error.
  bool parse(const std::string &data);
  // Parse a piece of json. Returns false in case of error.
  bool parse(const char *data, size_t len);
  // Finish parsing. Returns false in case of error.
  bool finish();
  // Returns parser error.
  std::string getError();
  // Returns reference to root entity parser.
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
  return _impl->parse(data.data(), data.size());
}

template <typename T> bool Parser<T>::parse(const char *data, size_t len) {
  return _impl->parse(data, len);
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
