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

#include "sjparser_internals.h"

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

  virtual void on(const Type &value) override;
  virtual void finish() override;
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
  using ChildArgs = typename KVParser::ChildArgs;
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
  Object(const Object &) = delete;

  // Returns reference to a parser of n-th field.
  // template <size_t n> X &get<n>();
  using KVParser::get;

 private:
  using KVParser::on;
  virtual void on(const MapKeyT &key) override;

  virtual void finish() override;

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

  virtual void finish() override;
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
         const std::function<bool(const Type &)> &on_finish = nullptr);
    Args(const ChildArgs &args,
         const std::function<bool(const Type &)> &on_finish = nullptr);

    ChildArgs args;
    Type default_value;
    bool allow_default_value = true;
    std::function<bool(const Type &)> on_finish;
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
  SAutoObject(const SAutoObject &) = delete;

  // Returns true if parser has some value stored and false otherwise
  // bool isSet();
  using TokenParser::isSet;

  // Returns reference to parsed value. If value is unset, throws
  // std::runtime_error.
  inline const Type &get() const;

  // Returns an rvalue reference to value. After call of this method value is
  // unset.
  inline Type &&pop();

 private:
  using TokenParser::_set;
  using TokenParser::checkSet;

  virtual void finish() override;
  virtual void reset() noexcept override;

  // This is placed in the private section because the ValueSetter uses pop on
  // all fields, so they are always unset after parsing.
  using Object<Ts...>::get;

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
  std::function<bool(const Type &)> _on_finish;
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
  using ChildArgs = typename KVParser::ChildArgs;
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
  using KVParser::on;

  virtual void on(const I &value) override;
  virtual void on(const MapStartT) override;
  virtual void on(const MapKeyT &key) override;

  virtual void childParsed() override;
  virtual void finish() override;

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
  using ParserType = T;
  template <typename U = Array<T>>
  using GrandChildArgs = typename U::ParserType::ChildArgs;

  struct Args {
    Args(const ChildArgs &args,
         const std::function<bool(Array<T> &)> &on_finish = nullptr);

    template <typename U = Array<T>>
    Args(const GrandChildArgs<U> &args,
         const std::function<bool(Array<T> &)> &on_finish = nullptr);

    ChildArgs args;
    std::function<bool(Array<T> &)> on_finish;
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

 private:
  std::function<bool(Array<T> &)> _on_finish;

  virtual void finish() override;
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
  using ParserType = T;
  template <typename U = Array<T>>
  using GrandChildArgs = typename U::ParserType::ChildArgs;

  struct Args {
    Args(const ChildArgs &args = {}, const CallbackType &on_finish = nullptr);

    template <typename U = Array<T>>
    Args(const GrandChildArgs<U> &args,
         const CallbackType &on_finish = nullptr);

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

  virtual void childParsed() override;
  virtual void finish() override;
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
  template <typename U = T>
  Parser(const typename U::template GrandChildArgs<U> &args);
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
}

#include "sjparser_impl.h"
