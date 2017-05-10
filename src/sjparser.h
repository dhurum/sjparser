/*******************************************************************************

Copyright (c) 2016-2017 Denis Tikhomirov <dvtikhomirov@gmail.com>

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
  Value(const Args &on_finish);

  // Returns true if parser has some value stored and false otherwise
  // bool isSet();
  using TokenParser::isSet;

  // Returns a reference to parsed value.  If value is unset, throws
  // std::runtime_error.
  inline const Type &get() const;

  // Returns an rvalue reference to value for std::string and reference for
  // other
  // types. After call of this method value is unset.
  template <typename U = Type>
  inline
      typename std::enable_if<std::is_same<U, std::string>::value, U>::type &&
      pop();
  template <typename U = Type>
  inline const typename std::enable_if<!std::is_same<U, std::string>::value,
                                       U>::type &
  pop();

 private:
  Type _value;
  Args _on_finish;

  virtual bool on(const Type &value) override;
  virtual bool finish() override;
};

/*
 * Object parser.
 * Ts is a list of any entity parsers.
 */

template <typename... Ts>
class Object : public KeyValueParser<FieldName, Ts...> {
 protected:
  using KVParser = KeyValueParser<FieldName, Ts...>;

 public:
  using ChildArgs = std::tuple<typename KVParser::template FieldArgs<Ts>...>;
  struct Args {
    Args(const ChildArgs &args,
         const std::function<bool(Object<Ts...> &)> &on_finish = nullptr);

    ChildArgs args;
    std::function<bool(Object<Ts...> &)> on_finish;
  };

  /*
   * Constructor receives a structure.
   * It's first element is a tuple of object fields arguments structs,
   * and second element is a callback that will be called after the object is
   * parsed.
   * If you do not specify the callback - you can provide only a tuple of object
   * fields arguments to constructor.
   * Field argument is a structure where a first element is a string with field
   * name, and a second field is an argument for respective parser.
   * If you do not want to provide an argument to a field parser - you can
   * provide only name.
   * For example: {"field1", {"field2", ...}, "field3"}
   * If you have only one field without arguments, you need to pass it without
   * brackets.
   * Callback is called with a reference to this parser as an argument.
   */
  Object(const Args &args);
  Object(const ChildArgs &args);
  Object(const Object &) = delete;

  // Returns reference to a parser of n-th field.
  // template <size_t n> X &get<n>();
  using KVParser::get;

 private:
  using KVParser::on;
  virtual bool on(const MapKeyT &key) override;

  virtual bool finish() override;

  typename KVParser::template Field<0, ChildArgs, Ts...> _fields;
  std::function<bool(Object<Ts...> &)> _on_finish;
};

/*
 * Object parser, that stores the result in a predefined type.
 * T is a type of result value (structure or object). It must have a default
 * constructor. If you want to store it in SArray it should have a move
 * constructor, to avoid unnecessary copy operations.
 * Ts is a list of any parsers.
 *
 * Instead of using this type directly you should use an SObject, it will
 * automatically dispatch SCustomObject and SAutoObject based on the template
 * parameters.
 */

template <typename T, typename... Ts>
class SCustomObject : public Object<Ts...> {
 public:
  using ChildArgs = typename Object<Ts...>::ChildArgs;
  using Type = T;

  struct Args {
    Args(const ChildArgs &args,
         const std::function<bool(SCustomObject<Type, Ts...> &, Type &)>
             &on_finish);

    ChildArgs args;
    std::function<bool(SCustomObject<Type, Ts...> &, Type &)> on_finish;
  };

  /*
   * Constructor receives a structure.
   * It's first element is a tuple of object fields arguments structs,
   * and second element is a callback that will be called after the object is
   * parsed.
   * Field argument is a structure where a first element is a string with field
   * name, and a second field is an argument for respective parser.
   * If you do not want to provide an argument to a field parser - you can
   * provide only name.
   * For example: {"field1", {"field2", ...}, "field3"}
   * Callback is called with a reference to this parser as an argument and a
   * reference to internal value. You must set it in this callback.
   */
  SCustomObject(const Args &args);
  SCustomObject(const SCustomObject &) = delete;

  // Returns true if parser has some value stored and false otherwise
  // bool isSet();
  using TokenParser::isSet;

  // Returns reference to a parser of n-th field.
  // template <size_t n> X &get<n>();
  using Object<Ts...>::get;

  // Returns reference to parsed value. If value is unset, throws
  // std::runtime_error.
  inline const Type &get() const;

  // Returns an rvalue reference to value. After call of this method value is
  // unset.
  inline Type &&pop();

 private:
  T _value;
  using TokenParser::_set;
  using TokenParser::checkSet;

  virtual bool finish() override;
  virtual void reset() noexcept override;

  std::function<bool(SCustomObject<T, Ts...> &, T &)> _on_finish;
};

/*
 * Object parser, that stores the result in a tuple of field parsers types.
 * Ts is a list of any parsers.
 *
 * Instead of using this type directly you should use an SObject, it will
 * automatically dispatch SCustomObject and SAutoObject based on the template
 * parameters.
 */

template <typename... Ts> class SAutoObject : public Object<Ts...> {
 public:
  using ChildArgs = typename Object<Ts...>::ChildArgs;
  using Type = std::tuple<typename Ts::Type...>;

  struct Args {
    Args(const ChildArgs &args, const Type &default_value,
         const std::function<bool(Type &)> &on_finish = nullptr);
    Args(const ChildArgs &args,
         const std::function<bool(Type &)> &on_finish = nullptr);

    ChildArgs args;
    Type default_value;
    bool allow_default_value = true;
    std::function<bool(Type &)> on_finish;
  };

  /*
   * Constructor receives a structure.
   * It's first element is a tuple of object fields arguments structs,
   * second element is a default value of the stored tuple, and the third
   * element is a callback that will be called after the object is
   * parsed.
   * Second and third srtucture elements are optional.
   * If you do not specify default value for a tuple, all object fields are
   * considered mandatory. Otherwise if some expected field is not present, the
   * default value will be used for it.
   * Field argument is a structure where a first element is a string with field
   * name, and a second field is an argument for respective parser.
   * If you do not want to provide an argument to a field parser - you can
   * provide only name.
   * For example: {"field1", {"field2", ...}, "field3"}
   * Callback is called with a reference to the internal value as an argument.
   */
  SAutoObject(const Args &args);
  SAutoObject(const ChildArgs &args);
  SAutoObject(const SAutoObject &) = delete;

  // Returns true if parser has some value stored and false otherwise
  // bool isSet();
  using TokenParser::isSet;

  // Returns reference to a parser of n-th field.
  // template <size_t n> X &get<n>();
  using Object<Ts...>::get;

  // Returns reference to parsed value. If value is unset, throws
  // std::runtime_error.
  inline const Type &get() const;

  // Returns an rvalue reference to value. After call of this method value is
  // unset.
  inline Type &&pop();

 private:
  using TokenParser::_set;
  using TokenParser::checkSet;

  virtual bool finish() override;
  virtual void reset() noexcept override;

  template <size_t, typename...> struct ValueSetter {
    ValueSetter(Type & /*value*/, SAutoObject<Ts...> & /*parser*/) {}
  };

  template <size_t n, typename T, typename... TDs>
  struct ValueSetter<n, T, TDs...> : private ValueSetter<n + 1, TDs...> {
    ValueSetter(Type &value, SAutoObject<Ts...> &parser);
  };

  Type _value;
  Type _default_value;
  bool _allow_default_value;
  std::function<bool(Type &)> _on_finish;
};

template <bool auto_type, typename... Ts> struct SObjectDispatcher {};

template <typename... Ts> struct SObjectDispatcher<false, Ts...> {
  using Type = SCustomObject<Ts...>;
};

template <typename... Ts> struct SObjectDispatcher<true, Ts...> {
  using Type = SAutoObject<Ts...>;
};

/* Dispatcher type alias, will point to SCustomObject or SAutoObject based on
 * the template parameters. You should use it instead of using those types
 * directly.
 */

template <typename T, typename... Ts>
using SObject =
    typename SObjectDispatcher<std::is_base_of<TokenParser, T>::value, T,
                               Ts...>::Type;

template <typename T> struct UnionFieldType { using type = T; };

template <> struct UnionFieldType<std::string> { using type = FieldName; };

/*
 * Union of objects parser. It can be used to parse an object from a list,
 * based on key field value.
 * I is key field type, can be int64_t, bool, std::string.
 * Ts is a list of object parsers.
 * You can use it stand-alone (in this case first field of object must be a key
 * field) or embedded in an object (in this case object fields after key field
 * will be parsed by one of union's object parsers).
 */
template <typename I, typename... Ts>
class Union : public KeyValueParser<typename UnionFieldType<I>::type, Ts...> {
 protected:
  using KVParser = KeyValueParser<typename UnionFieldType<I>::type, Ts...>;

 public:
  using ChildArgs = std::tuple<typename KVParser::template FieldArgs<Ts>...>;
  struct Args {
    Args(const ChildArgs &args,
         const std::function<bool(Union<I, Ts...> &)> &on_finish = nullptr);
    Args(const FieldName &type_field, const ChildArgs &args,
         const std::function<bool(Union<I, Ts...> &)> &on_finish = nullptr);

    std::string type_field;
    ChildArgs args;
    std::function<bool(Union<I, Ts...> &)> on_finish;
  };

  /*
   * Constructor can receive two structures.
   *
   * In stand-alone mode it's first element is key field name,
   * second element is a tuple of union members arguments structs,
   * and third element is a callback that will be called after the union is
   * parsed.
   * If you do not specify the callback - you can provide only key field name
   * and tuple of union members arguments to constructor.
   *
   * In embedded mode first element is a tuple of union members arguments,
   * and second element is a callback that will be called after the union is
   * parsed.
   * If you do not specify the callback - you can provide only tuple of union
   * members arguments to constructor.
   *
   * Member argument is a structure where a first element is a string with key
   * field value, and a second field is an argument for respective parser.
   *
   * For example: {"key", {{"1", ...}, {"2", ...}}}
   * Callback is called with a reference to this parser as an argument.
   */
  Union(const Args &args);
  Union(const ChildArgs &args);
  Union(const Union &) = delete;

  // Returns id of parsed member. If no members were parsed, throws std::runtime
  // exception.
  size_t currentMemberId();

  // Returns true if parser has parsed something and false otherwise
  // bool isSet();
  using TokenParser::isSet;

  // Returns reference to a parser of n-th member.
  // template <size_t n> X &get<n>();
  using KVParser::get;

 private:
  using TokenParser::checkSet;

  virtual bool on(const I &value) override;
  virtual bool on(const MapStartT) noexcept override;
  virtual bool on(const MapKeyT &key) override;
  virtual bool on(const MapEndT) override;

  virtual bool childParsed() override;
  virtual bool finish() override;

  typename KVParser::template Field<0, ChildArgs, Ts...> _fields;
  std::string _type_field;
  std::function<bool(Union<I, Ts...> &)> _on_finish;
  std::unordered_map<TokenParser *, size_t> _fields_ids_map;
  size_t _current_member_id;
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

  // Returns true if parser has some value stored and false otherwise
  // bool isSet();
  using TokenParser::isSet;

  // Returns reference to vecor of values. If vector is unset, throws
  // std::runtime_error.
  inline const Type &get() const;

  // Returns an rvalue reference to vector of values. After call of this method
  // value is unset.
  inline Type &&pop();

 private:
  std::vector<EltType> _values;
  std::function<bool(const Type &)> _on_finish;
  using TokenParser::_set;
  using TokenParser::checkSet;

  virtual bool childParsed() override;
  virtual bool finish() override;
  virtual void reset() noexcept override;
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
  inline bool parse(const std::string &data);

  // Parse a piece of json. Returns false in case of error.
  inline bool parse(const char *data, size_t len);

  // Finish parsing. Returns false in case of error.
  inline bool finish();

  // Returns parser error.
  inline std::string getError(bool verbose = false);

  // Returns reference to root entity parser.
  inline T &parser();

 private:
  T _parser;
  std::unique_ptr<ParserImpl> _impl;
};

/******************************** Definitions ********************************/

template <typename T>
Value<T>::Value(const Args &on_finish) : _on_finish(on_finish) {}

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

template <typename T> const T &Value<T>::get() const {
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

template <typename... Ts>
Object<Ts...>::Args::Args(const ChildArgs &args,
                          const std::function<bool(Object<Ts...> &)> &on_finish)
    : args(args), on_finish(on_finish) {}

template <typename... Ts>
Object<Ts...>::Object(const Args &args)
    : _fields(KVParser::_fields_array, KVParser::_fields_map, args.args),
      _on_finish(args.on_finish) {}

template <typename... Ts>
Object<Ts...>::Object(const ChildArgs &args) : Object({args, nullptr}) {}

template <typename... Ts> bool Object<Ts...>::on(const MapKeyT &key) {
  return KVParser::onField(key.key);
}

template <typename... Ts> bool Object<Ts...>::finish() {
  if (!_on_finish) {
    return true;
  }
  return _on_finish(*this);
}

template <typename T, typename... Ts>
SCustomObject<T, Ts...>::Args::Args(
    const ChildArgs &args,
    const std::function<bool(SCustomObject<T, Ts...> &, T &)> &on_finish)
    : args(args), on_finish(on_finish) {}

template <typename T, typename... Ts>
SCustomObject<T, Ts...>::SCustomObject(const Args &args)
    : Object<Ts...>(args.args), _on_finish(args.on_finish) {}

template <typename T, typename... Ts>
const typename SCustomObject<T, Ts...>::Type &SCustomObject<T, Ts...>::get()
    const {
  checkSet();
  return _value;
}

template <typename T, typename... Ts>
typename SCustomObject<T, Ts...>::Type &&SCustomObject<T, Ts...>::pop() {
  checkSet();
  _set = false;
  return std::move(_value);
}

template <typename T, typename... Ts> bool SCustomObject<T, Ts...>::finish() {
  return _on_finish(*this, _value);
}

template <typename T, typename... Ts>
void SCustomObject<T, Ts...>::reset() noexcept {
  Object<Ts...>::KVParser::reset();
  _value = Type();
}

template <typename... Ts>
SAutoObject<Ts...>::Args::Args(const ChildArgs &args, const Type &default_value,
                               const std::function<bool(Type &)> &on_finish)
    : args(args), default_value(default_value), on_finish(on_finish) {}

template <typename... Ts>
SAutoObject<Ts...>::Args::Args(const ChildArgs &args,
                               const std::function<bool(Type &)> &on_finish)
    : args(args), allow_default_value(false), on_finish(on_finish) {}

template <typename... Ts>
SAutoObject<Ts...>::SAutoObject(const Args &args)
    : Object<Ts...>(args.args),
      _default_value(args.default_value),
      _allow_default_value(args.allow_default_value),
      _on_finish(args.on_finish) {}

template <typename... Ts>
SAutoObject<Ts...>::SAutoObject(const ChildArgs &args)
    : SAutoObject<Ts...>({args, nullptr}) {}

template <typename... Ts>
const typename SAutoObject<Ts...>::Type &SAutoObject<Ts...>::get() const {
  checkSet();
  return _value;
}

template <typename... Ts>
typename SAutoObject<Ts...>::Type &&SAutoObject<Ts...>::pop() {
  checkSet();
  _set = false;
  return std::move(_value);
}

template <typename... Ts> bool SAutoObject<Ts...>::finish() {
  ValueSetter<0, Ts...>(_value, *this);
  if (_on_finish) {
    return _on_finish(_value);
  }
  return true;
}

template <typename... Ts> void SAutoObject<Ts...>::reset() noexcept {
  Object<Ts...>::KVParser::reset();
  _value = _default_value;
}

template <typename... Ts>
template <size_t n, typename T, typename... TDs>
SAutoObject<Ts...>::ValueSetter<n, T, TDs...>::ValueSetter(
    Type &value, SAutoObject<Ts...> &parser)
    : ValueSetter<n + 1, TDs...>(value, parser) {
  auto &field_parser = parser.template get<n>();
  if (field_parser.isSet()) {
    std::get<n>(value) = field_parser.pop();
  } else if (!parser._allow_default_value) {
    throw std::runtime_error(
        "Not all fields are set in an storage object without a default value");
  }
}

template <typename I, typename... Ts>
Union<I, Ts...>::Args::Args(
    const ChildArgs &args,
    const std::function<bool(Union<I, Ts...> &)> &on_finish)
    : args(args), on_finish(on_finish) {}

template <typename I, typename... Ts>
Union<I, Ts...>::Args::Args(
    const FieldName &type_field, const ChildArgs &args,
    const std::function<bool(Union<I, Ts...> &)> &on_finish)
    : type_field(type_field), args(args), on_finish(on_finish) {}

template <typename I, typename... Ts>
Union<I, Ts...>::Union(const Args &args)
    : _fields(KVParser::_fields_array, KVParser::_fields_map, args.args),
      _type_field(args.type_field),
      _on_finish(args.on_finish) {
  for (size_t i = 0; i < KVParser::_fields_array.size(); ++i) {
    _fields_ids_map[KVParser::_fields_array[i]] = i;
  }
}

template <typename I, typename... Ts>
Union<I, Ts...>::Union(const ChildArgs &args) : Union({args, nullptr}) {}

template <typename I, typename... Ts>
size_t Union<I, Ts...>::currentMemberId() {
  checkSet();
  return _current_member_id;
}

template <typename I, typename... Ts> bool Union<I, Ts...>::on(const I &value) {
  KVParser::reset();
  if (KVParser::onField(value)) {
    _current_member_id = _fields_ids_map[KVParser::_fields_map[value]];
    return true;
  }
  return false;
}

template <typename I, typename... Ts>
bool Union<I, Ts...>::on(const MapStartT) noexcept {
  if (_type_field.empty()) {
    return false;
  }
  return true;
}

template <typename I, typename... Ts>
bool Union<I, Ts...>::on(const MapKeyT &key) {
  if (_type_field.empty()) {
    return false;
  }
  if (key.key != _type_field) {
    std::stringstream error;
    error << "Unexpected field " << key.key;
    KVParser::_dispatcher->setError(error.str());
    return false;
  }
  return true;
}

template <typename I, typename... Ts> bool Union<I, Ts...>::on(const MapEndT) {
  return false;
}

template <typename I, typename... Ts> bool Union<I, Ts...>::childParsed() {
  return KVParser::endParsing();
}

template <typename I, typename... Ts> bool Union<I, Ts...>::finish() {
  if (!_on_finish) {
    return true;
  }
  return _on_finish(*this);
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

template <typename T> const typename SArray<T>::Type &SArray<T>::get() const {
  checkSet();
  return _values;
}

template <typename T> typename SArray<T>::Type &&SArray<T>::pop() {
  checkSet();
  _set = false;
  return std::move(_values);
}

template <typename T> bool SArray<T>::childParsed() {
  _values.push_back(Array<T>::_parser.pop());
  return true;
}

template <typename T> bool SArray<T>::finish() {
  if (!_on_finish) {
    return true;
  }
  return _on_finish(_values);
}

template <typename T> void SArray<T>::reset() noexcept {
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

template <typename T> std::string Parser<T>::getError(bool verbose) {
  return _impl->getError(verbose);
}

template <typename T> T &Parser<T>::parser() {
  return _parser;
}
}
