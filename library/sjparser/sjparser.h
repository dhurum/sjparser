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

#include "internals.h"
#include "yajl_parser.h"

#include <array>
#include <functional>
#include <memory>
#include <string>
#include <tuple>
#include <type_traits>
#include <unordered_map>
#include <variant>
#include <vector>

namespace SJParser {

/** @brief Plain value parser.
 *
 * @tparam T JSON value type, can be std::string, int64_t, bool or double
 */

template <typename T> class Value : public TokenParser {
 public:
  /** Underlying type, that can be obtained from this parser with #get or #pop.
   */
  using Type = T;

  /** Constructor argument type. */
  using Args = std::function<bool(const Type &)>;

  /** @brief Constructor.
   *
   * @param [in] on_finish (optional) Callback, that will be called after a
   * value is parsed.
   *
   * The callback will be called with a const reference to a parsed value as an
   * argument.
   *
   * If the callback returns false, parsing will be stopped with an error.
   */
  Value(const Args &on_finish = nullptr);

#ifdef DOXYGEN_ONLY
  /** @brief Check if the parser has a value.
   *
   * @return True if the parser has some value stored or false otherwise.
   */
  bool isSet();
#endif

  /** @brief Parsed value getter.
   *
   * @return Const reference to a parsed value.
   *
   * @throw std::runtime_error Thrown if the value is unset (no value was
   * parsed or #pop was called).
   */
  const Type &get() const;

  /** @brief Get the parsed value and unset the parser.
   *
   * Moves the parsed value out of the parser.
   *
   * @return Rvalue reference to the parsed value.
   *
   * @throw std::runtime_error thrown if the value is unset (no value was
   * parsed or #pop was called).
   */
  Type &&pop();

 private:
  void on(TokenType<Type> value) override;
  void finish() override;

  Type _value;
  Args _on_finish;
};

/** @brief %Object parser.
 *
 * @tparam Ts A list of field parsers types.
 */

template <typename... Ts>
class Object : public KeyValueParser<FieldName, Ts...> {
 protected:
  /** @cond Internal typedef */
  using KVParser = KeyValueParser<FieldName, Ts...>;
  /** @endcond */

 public:
  /** A std::tuple with arguments for field parsers. */
  using ChildArgs = typename KVParser::ChildArgs;

  /** Type alias for the parser options. */
  using Options = ObjectOptions;

  /** @brief Struct with arguments for the Object @ref Object() "constructor".
   */
  struct Args {
    /** @param [in] args Sets #args.
     *
     * @param [in] options Sets #options.
     *
     * @param [in] on_finish (optional) Sets #on_finish.
     */
    Args(const ChildArgs &args, const Options &options,
         const std::function<bool(Object<Ts...> &)> &on_finish = nullptr);

    /** @param [in] args Sets #args.
     *
     * @param [in] on_finish (optional) Sets #on_finish.
     *
     * @note If you are passing a single field name (without arguments)
     * to the constructor, you must pass it without surrounding {}:
     * @code {.cpp} Object<...>::Args args("field"); @endcode
     * However, if you initialize Args, you can pass the extra {}:
     * @code {.cpp} Object<...>::Args args = {"field"}; @endcode
     * @code {.cpp} Object<...>::Args args = {{"field"}}; @endcode
     */
    Args(const ChildArgs &args,
         const std::function<bool(Object<Ts...> &)> &on_finish = nullptr);

    /** A std::tuple with field arguments.
     *
     * A field argument is a structure where the first element is a string
     * with field's name and the second element is arguments to that field's
     * parser. If you do not want to provide any arguments to the field's
     * parser, you can pass only the field's name:
     * @code {.cpp} {"field1", {"field2", ...}, "field3"} @endcode
     * In this example field1 and field3 parsers do not receive any agruments
     * while field2 does receive something.
     */
    ChildArgs args;

    /** #SJParser::ObjectOptions struct with parser options. */
    Options options;

    /** Callback, that will be called after an object is parsed.
     *
     * The callback will be called with a reference to the parser as an
     * argument.
     *
     * If the callback returns false, parsing will be stopped with an error.
     */
    std::function<bool(Object<Ts...> &)> on_finish;
  };

  /** @brief Object constructor.
   *
   * @param [in] args Args stucture.
   * If you do not specify @ref Args::on_finish "on_finish" callback,
   * you can pass a @ref Args::args "tuple of field arguments" directly
   * into the constructor.
   *
   * @note If you are passing a single field name (without arguments) directly
   * to the constructor, you must pass it without surrounding {}:
   * @code {.cpp} Object<...> object("field"); @endcode
   */
  Object(const Args &args);
  Object(const Object &) = delete;

#ifdef DOXYGEN_ONLY
  /** @brief Universal field getter.
   *
   * @tparam n Index of the parser's field.
   *
   * @return If the n-th field parser stores value (is a Value, SObject or
   * SArray), then the method returns a const reference to the n-th field parser
   * value. Otherwise, returns a reference to the n-th field parser.
   *
   * @throw std::runtime_error thrown if the field parser value is unset (no
   * value was parsed or #pop was called for the field parser).
   */
  template <size_t n>
  const typename NthTypes<n, Ts...>::template ValueType<> &get();

  /** @brief Universal field getter.
   *
   * @tparam n Index of the parser's field.
   *
   * @return If the n-th field parser stores value (is a Value, SObject or
   * SArray), then the method returns a const reference to the n-th field parser
   * value. Otherwise, returns a reference to the n-th field parser.
   */
  template <size_t n> typename NthTypes<n, Ts...>::ParserType &get();

  /** @brief Field parser getter.
   *
   * @tparam n Index of the parser's field.
   *
   * @return Reference to n-th field parser.
   */
  template <size_t n> typename NthTypes<n, Ts...>::ParserType &parser();

  /** @brief Get the field parsed value and unset the field parser.
   *
   * Moves the n-th field parsed value out of the field parser.
   *
   * @tparam n Index of the parser's field.
   *
   * @return Rvalue reference to n-th field parser value.
   *
   * @throw std::runtime_error thrown if the field parser value is unset (no
   * value was parsed or #pop was called for the field parser).
   */
  template <size_t n> typename NthTypes<n, Ts...>::template ValueType<> &&pop();
#endif

 private:
  using KVParser::on;
  void on(MapKeyT key) override;

  void finish() override;

  std::function<bool(Object<Ts...> &)> _on_finish;
};

/** @brief %Object parser, that stores parsed value of type
 * @ref SCustomObject_T "T"
 *
 * Stored value is set from a @ref Args::on_finish "callback".
 *
 * Instead of using this type directly you should use an SObject, it will
 * automatically dispatch SCustomObject and SAutoObject based on the template
 * parameters.
 *
 * @tparam T Stored value type. It must have a default constructor. If you want
 * to include this parser into SArray, a move constructor of this type will be
 * used if possible.
 * @anchor SCustomObject_T
 *
 * @tparam Ts A list of field parsers types.
 */

template <typename T, typename... Ts>
class SCustomObject : public Object<Ts...> {
 public:
  /** A std::tuple with arguments for field parsers. */
  using ChildArgs = typename Object<Ts...>::ChildArgs;

  /** Stored value type */
  using Type = T;

  /** Type alias for the parser options. */
  using Options = typename Object<Ts...>::Options;

  /** @brief Struct with arguments for the SCustomObject
   * @ref SCustomObject() "constructor".
   */
  struct Args {
    /** @param [in] args Sets #args.
     *
     * @param [in] options Sets #options.
     *
     * @param[in] on_finish Sets #on_finish.
     */
    Args(const ChildArgs &args, const Options &options,
         const std::function<bool(SCustomObject<Type, Ts...> &, Type &)>
             &on_finish);
    /** @param [in] args Sets #args.
     *
     * @param[in] on_finish Sets #on_finish.
     */
    Args(const ChildArgs &args,
         const std::function<bool(SCustomObject<Type, Ts...> &, Type &)>
             &on_finish);

    /** A std::tuple with field arguments.
     *
     * A field argument is a structure where the first element is a string
     * with field's name and the second element is arguments to that field's
     * parser. If you do not want to provide any arguments to the field's
     * parser, you can pass only the field's name:
     * @code {.cpp} {"field1", {"field2", ...}, "field3"} @endcode
     * In this example field1 and field3 parsers do not receive any agruments
     * while field2 does receive something.
     */
    ChildArgs args;

    /** #SJParser::ObjectOptions struct with parser options. */
    Options options;

    /** Callback, that will be called after an object is parsed.
     *
     * The callback will be called with a reference to the parser as a first
     * argument and a reference to the stored value as a second argument. You
     * must set the stored value in this callback.
     *
     * If the callback returns false, parsing will be stopped with an error.
     */
    std::function<bool(SCustomObject<Type, Ts...> &, Type &)> on_finish;
  };

  /** @brief SCustomObject constructor.
   *
   * @param [in] args Args stucture.
   */
  SCustomObject(const Args &args);
  SCustomObject(const SCustomObject &) = delete;

#ifdef DOXYGEN_ONLY
  /** @brief Check if the parser has a value.
   *
   * @return True if the parser has some value stored or false otherwise.
   */
  bool isSet();
#endif

#ifdef DOXYGEN_ONLY
  /** @brief Universal field getter.
   *
   * @tparam n Index of the parser's field.
   *
   * @return If the n-th field parser stores value (is a Value, SObject or
   * SArray), then the method returns a const reference to the n-th field parser
   * value. Otherwise, returns a reference to the n-th field parser.
   *
   * @throw std::runtime_error thrown if the field parser value is unset (no
   * value was parsed or #pop was called for the field parser).
   */
  template <size_t n>
  const typename NthTypes<n, Ts...>::template ValueType<> &get();

  /** @brief Universal field getter.
   *
   * @tparam n Index of the parser's field.
   *
   * @return If the n-th field parser stores value (is a Value, SObject or
   * SArray), then the method returns a const reference to the n-th field parser
   * value. Otherwise, returns a reference to the n-th field parser.
   */
  template <size_t n> typename NthTypes<n, Ts...>::ParserType &get();

  /** @brief Field parser getter.
   *
   * @tparam n Index of the parser's field.
   *
   * @return Reference to n-th field parser.
   */
  template <size_t n> typename NthTypes<n, Ts...>::ParserType &parser();
#endif
  using Object<Ts...>::get;

  /** @brief Parsed value getter.
   *
   * @return Const reference to a parsed value.
   *
   * @throw std::runtime_error Thrown if the value is unset (no value was
   * parsed or #pop was called).
   */
  const Type &get() const;

#ifdef DOXYGEN_ONLY
  /** @brief Get the field parsed value and unset the field parser.
   *
   * Moves the n-th field parsed value out of the field parser.
   *
   * @tparam n Index of the parser's field.
   *
   * @return Rvalue reference to n-th field parser value.
   *
   * @throw std::runtime_error thrown if the field parser value is unset (no
   * value was parsed or #pop was called for the field parser).
   */
  template <size_t n> typename NthTypes<n, Ts...>::template ValueType<> &&pop();
#endif
  using Object<Ts...>::pop;

  /** @brief Get the parsed value and unset the parser.
   *
   * Moves the parsed value out of the parser.
   *
   * @note
   * @par
   * If you want your SCustomObject::Type to be movable, you need to provide
   * either a move assignment operator or a copy assignment operator, they are
   * used internally.
   * @par
   * If you want to use this parser in the SAutoObject, you need to provide both
   * a copy constructor and a copy assignment operator in the
   * SCustomObject::Type, they are used by std::tuple.
   *
   * @return Rvalue reference to the parsed value.
   *
   * @throw std::runtime_error Thrown if the value is unset (no value was
   * parsed or #pop was called).
   */
  Type &&pop();

 private:
  using TokenParser::_set;
  using TokenParser::checkSet;

  void finish() override;
  void reset() override;

  Type _value;
  std::function<bool(SCustomObject<T, Ts...> &, T &)> _on_finish;
};

/** @brief %Object parser, that stores the result in a tuple of field parsers
 * types.
 *
 * Instead of using this type directly you should use an SObject, it will
 * automatically dispatch SCustomObject and SAutoObject based on the template
 * parameters.
 *
 * @tparam Ts A list of field parsers types.
 */

template <typename... Ts> class SAutoObject : public Object<Ts...> {
 public:
  /** A std::tuple with arguments for field parsers. */
  using ChildArgs = typename Object<Ts...>::ChildArgs;

  /** Stored value type */
  using Type = std::tuple<typename Ts::Type...>;

  /** Type alias for the parser options. */
  using Options = typename Object<Ts...>::Options;

  /** @brief Struct with arguments for the SAutoObject
   * @ref SAutoObject() "constructor".
   */
  struct Args {
    /** @param [in] args Sets #args.
     *
     * @param[in] default_value Default for the parser value.
     *
     * @param [in] options Sets #options.
     *
     * @param[in] on_finish (optional) Sets #on_finish.
     */
    Args(const ChildArgs &args, const Type &default_value,
         const Options &options,
         const std::function<bool(const Type &)> &on_finish = nullptr);

    /** @param [in] args Sets #args.
     *
     * @param[in] default_value Default for the parser value.
     *
     * @param[in] on_finish (optional) Sets #on_finish.
     */
    Args(const ChildArgs &args, const Type &default_value,
         const std::function<bool(const Type &)> &on_finish = nullptr);

    /** @param [in] args Sets #args.
     *
     * @param [in] options Sets #options.
     *
     * @param[in] on_finish (optional) Sets #on_finish.
     */
    Args(const ChildArgs &args, const Options &options,
         const std::function<bool(const Type &)> &on_finish = nullptr);

    /** @param [in] args Sets #args.
     *
     * @param[in] on_finish (optional) Sets #on_finish.
     */
    Args(const ChildArgs &args,
         const std::function<bool(const Type &)> &on_finish = nullptr);

    /** A std::tuple with field arguments.
     *
     * A field argument is a structure where the first element is a string
     * with field's name and the second element is arguments to that field's
     * parser. If you do not want to provide any arguments to the field's
     * parser, you can pass only the field's name:
     * @code {.cpp} {"field1", {"field2", ...}, "field3"} @endcode
     * In this example field1 and field3 parsers do not receive any agruments
     * while field2 does receive something.
     */
    ChildArgs args;

    /** Default for the parser value. */
    Type default_value;

    /** @internal Internal flag, shows if default value was set. */
    bool allow_default_value = true;

    /** #SJParser::ObjectOptions struct with parser options. */
    Options options;

    /** Callback, that will be called after an object is parsed.
     *
     * The callback will be called with a reference to the stored value as an
     * argument.
     *
     * If the callback returns false, parsing will be stopped with an error.
     */
    std::function<bool(const Type &)> on_finish;
  };

  /** @brief SAutoObject constructor.
   *
   * @param [in] args Args stucture.
   * If you do not specify @ref Args::default_value "defult_value" default value
   * and @ref Args::on_finish "on_finish" callback, you can pass a
   * @ref Args::args "tuple of field arguments" directly into the constructor.
   *
   * @note If you are passing a single field name (without arguments), you must
   * pass it directly to the constructor, without surrounding {}:
   * @code {.cpp} SObject<...> object("field"); @endcode
   */
  SAutoObject(const Args &args);
  SAutoObject(const SAutoObject &) = delete;

#ifdef DOXYGEN_ONLY
  /** @brief Check if the parser has a value.
   *
   * @return True if the parser has some value stored or false otherwise.
   */
  bool isSet();
#endif

  /** @brief Parsed value getter.
   *
   * @return Const reference to a parsed value.
   *
   * @throw std::runtime_error Thrown if the value is unset (no value was
   * parsed or #pop was called).
   */
  const Type &get() const;

  /** @brief Get the parsed value and unset the parser.
   *
   * Moves the parsed value out of the parser.
   *
   * @note If you want to use SCustomObject inside this parser, you need to
   * provide both a copy constructor or move constructor and a copy assignment
   * operators in the SCustomObject::Type, they are used by parser.
   *
   * @return Rvalue reference to the parsed value.
   *
   * @throw std::runtime_error Thrown if the value is unset (no value was
   * parsed or #pop was called).
   */
  Type &&pop();

 private:
  using TokenParser::_set;
  using TokenParser::checkSet;

  void finish() override;
  void reset() override;

  // This is placed in the private section because the ValueSetter uses pop on
  // all fields, so they are always unset after parsing.
  using Object<Ts...>::get;
  using Object<Ts...>::pop;

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

/** @cond Internal class, needed for SObject */

template <bool auto_type, typename... Ts> struct SObjectDispatcher {
  using Type = SCustomObject<Ts...>;
};

template <typename... Ts> struct SObjectDispatcher<true, Ts...> {
  using Type = SAutoObject<Ts...>;
};

/** @endcond */

#ifdef DOXYGEN_ONLY
/** @brief SCustomObject and SAutoObject dispatcher
 *
 * Will point to SCustomObject or SAutoObject based on the template parameters.
 * You should use it instead of using those types directly.
 */

template <typename... Ts> class SObject;
#endif

template <typename T, typename... Ts>
using SObject = typename SObjectDispatcher<std::is_base_of_v<TokenParser, T>, T,
                                           Ts...>::Type;

/** @cond Internal class, needed for Union */

template <typename T> struct UnionFieldType { using Type = T; };

template <> struct UnionFieldType<std::string> { using Type = FieldName; };

/** @endcond */

/** @brief %Union of @ref Object "Objects" parser.
 *
 * Parses an object from @ref Union_Ts "Ts" list based on a value of the type
 * field value.
 *
 * You can use it stand-alone (in this case the first field of an object must
 * be a type field) or embedded in an object (in this case object fields after
 * the type field will be parsed by one of union's object parsers).
 *
 * Union type is defined by the Args, passed to the constructor.
 *
 * @tparam TypeFieldT A type of the type field. Can be int64_t, bool, double and
 * std::string.
 *
 * @tparam Ts A list of object parsers.
 * @anchor Union_Ts
 */

template <typename TypeFieldT, typename... Ts>
class Union
    : public KeyValueParser<typename UnionFieldType<TypeFieldT>::Type, Ts...> {
 protected:
  /** @cond Internal typedef */
  using KVParser =
      KeyValueParser<typename UnionFieldType<TypeFieldT>::Type, Ts...>;
  /** @endcond */

 public:
  /** A std::tuple with arguments for field parsers. */
  using ChildArgs = typename KVParser::ChildArgs;

  /** @brief Struct with arguments for the Union @ref Union() "constructor".
   */
  struct Args {
    /** @brief Embedded Union constructor arguments.
     *
     * In this mode the parser must be used as an Object field. That field value
     * would be used as a type field value by the Union.
     *
     * @param [in] args Sets #args.
     *
     * @param[in] on_finish (optional) Sets #on_finish.
     */
    Args(const ChildArgs &args,
         const std::function<bool(Union<TypeFieldT, Ts...> &)> &on_finish =
             nullptr);

    /** @brief Standalone Union constructor arguments.
     *
     * @param [in] type_field Type field name.
     *
     * @param [in] args Sets #args.
     *
     * @param[in] on_finish (optional) Sets #on_finish.
     */
    Args(const FieldName &type_field, const ChildArgs &args,
         const std::function<bool(Union<TypeFieldT, Ts...> &)> &on_finish =
             nullptr);

    /** Type field name */
    std::string type_field;

    /** A std::tuple with field arguments.
     *
     * A field argument is a structure where the first element is a string
     * with field's name and the second element is arguments to that field's
     * parser. If you do not want to provide any arguments to the field's
     * parser, you can pass only the field's name:
     * @code {.cpp} {"field1", {"field2", ...}, "field3"} @endcode
     * In this example field1 and field3 parsers do not receive any agruments
     * while field2 does receive something.
     */
    ChildArgs args;

    /** Callback, that will be called after a union object is parsed.
     *
     * The callback will be called with a reference to the parser as an
     * argument.
     *
     * If the callback returns false, parsing will be stopped with an error.
     */
    std::function<bool(Union<TypeFieldT, Ts...> &)> on_finish;
  };

  /** @brief Union constructor.
   *
   * @param [in] args Args stucture.
   * If you do not specify @ref Args::type_field "type_field" type field
   * and @ref Args::on_finish "on_finish" callback, you can pass a
   * @ref Args::args "tuple of field arguments" directly into the constructor.
   */
  Union(const Args &args);
  Union(const Union &) = delete;

  /** @brief Parsed object index getter.
   *
   * @return Index of a parsed object.
   *
   * @throw std::runtime_error Thrown if no members were parsed.
   */
  size_t currentMemberId();

#ifdef DOXYGEN_ONLY
  /** @brief Check if the parser has a value.
   *
   * @return True if the parser has some value stored or false otherwise.
   */
  bool isSet();
#endif

#ifdef DOXYGEN_ONLY
  /** @brief Universal field getter.
   *
   * @tparam n Index of the parser's field.
   *
   * @return If the n-th field parser stores value (is a Value, SObject or
   * SArray), then the method returns a const reference to the n-th field parser
   * value. Otherwise, returns a reference to the n-th field parser.
   *
   * @throw std::runtime_error thrown if the field parser value is unset (no
   * value was parsed or #pop was called for the field parser).
   */
  template <size_t n>
  const typename NthTypes<n, Ts...>::template ValueType<> &get();

  /** @brief Universal field getter.
   *
   * @tparam n Index of the parser's field.
   *
   * @return If the n-th field parser stores value (is a Value, SObject or
   * SArray), then the method returns a const reference to the n-th field parser
   * value. Otherwise, returns a reference to the n-th field parser.
   */
  template <size_t n> typename NthTypes<n, Ts...>::ParserType &get();

  /** @brief Field parser getter.
   *
   * @tparam n Index of the parser's field.
   *
   * @return Reference to n-th field parser.
   */
  template <size_t n> typename NthTypes<n, Ts...>::ParserType &parser();

  /** @brief Get the field parsed value and unset the field parser.
   *
   * Moves the n-th field parsed value out of the field parser.
   *
   * @tparam n Index of the parser's field.
   *
   * @return Rvalue reference to n-th field parser value.
   *
   * @throw std::runtime_error thrown if the field parser value is unset (no
   * value was parsed or #pop was called for the field parser).
   */
  template <size_t n> typename NthTypes<n, Ts...>::template ValueType<> &&pop();
#endif

 protected:
  using TokenParser::checkSet;

 private:
  using KVParser::on;

  void on(TokenType<TypeFieldT> value) override;
  void on(MapStartT /*unused*/) override;
  void on(MapKeyT key) override;

  void childParsed() override;
  void finish() override;

  std::string _type_field;
  std::function<bool(Union<TypeFieldT, Ts...> &)> _on_finish;
  std::unordered_map<TokenParser *, size_t> _fields_ids_map;
  size_t _current_member_id;
};

/** @brief %Union parser, that stores the result in a variant of field parser
 * types.
 *
 * Parses an object from @ref SUnion_Ts "Ts" list based on a value of the type
 * field value.
 *
 * You can use it stand-alone (in this case the first field of an object must
 * be a type field) or embedded in an object (in this case object fields after
 * the type field will be parsed by one of union's object parsers).
 *
 * SUnion type is defined by the Args, passed to the constructor.
 *
 * @tparam TypeFieldT A type of the type field. Can be int64_t, bool, double and
 * std::string.
 *
 * @tparam Ts A list of object parsers.
 * @anchor SUnion_Ts
 */

template <typename TypeFieldT, typename... Ts>
class SUnion : public Union<TypeFieldT, Ts...> {
 public:
  /** A std::tuple with arguments for field parsers. */
  using ChildArgs = typename Union<TypeFieldT, Ts...>::ChildArgs;

  /** Stored value type */
  using Type = std::variant<typename Ts::Type...>;

  /** @brief Struct with arguments for the SUnion @ref SUnion() "constructor".
   */
  struct Args {
    /** @brief Embedded SUnion constructor arguments.
     *
     * In this mode the parser must be used as an Object field. That field value
     * would be used as a type field value by the SUnion.
     *
     * @param [in] args Sets #args.
     *
     * @param[in] on_finish (optional) Sets #on_finish.
     */
    Args(const ChildArgs &args,
         const std::function<bool(const Type &)> &on_finish = nullptr);

    /** @brief Standalone SUnion constructor arguments.
     *
     * @param [in] type_field Type field name.
     *
     * @param [in] args Sets #args.
     *
     * @param[in] on_finish (optional) Sets #on_finish.
     */
    Args(const FieldName &type_field, const ChildArgs &args,
         const std::function<bool(const Type &)> &on_finish = nullptr);

    /** @brief Standalone SUnion constructor arguments.
     *
     * @param [in] type_field Type field name.
     *
     * @param [in] args Sets #args.
     *
     * @param[in] default_value Default for the parser value. Will be used in
     * case of empty union.
     *
     * @param[in] on_finish (optional) Sets #on_finish.
     */
    Args(const FieldName &type_field, const ChildArgs &args,
         const Type &default_value,
         const std::function<bool(const Type &)> &on_finish = nullptr);

    /** Type field name */
    std::string type_field;
    /** A std::tuple with field arguments.
     *
     * A field argument is a structure where the first element is a string
     * with field's name and the second element is arguments to that field's
     * parser. If you do not want to provide any arguments to the field's
     * parser, you can pass only the field's name:
     * @code {.cpp} {"field1", {"field2", ...}, "field3"} @endcode
     * In this example field1 and field3 parsers do not receive any agruments
     * while field2 does receive something.
     */
    ChildArgs args;

    /** Default for the parser value. */
    Type default_value;

    /** @internal Internal flag, shows if default value was set. */
    bool allow_default_value = true;

    /** Callback, that will be called after an sunion object is parsed.
     *
     * The callback will be called with a reference to the stored value as an
     * argument.
     *
     * If the callback returns false, parsing will be stopped with an error.
     */
    std::function<bool(const Type &)> on_finish;
  };

  /** @brief SUnion constructor.
   *
   * @param [in] args Args stucture.
   * If you do not specify @ref Args::type_field "type_field" type field
   * and @ref Args::on_finish "on_finish" callback, you can pass a
   * @ref Args::args "tuple of field arguments" directly into the constructor.
   */
  SUnion(const Args &args);
  SUnion(const SUnion &) = delete;

#ifdef DOXYGEN_ONLY
  /** @brief Check if the parser has a value.
   *
   * @return True if the parser has some value stored or false otherwise.
   */
  bool isSet();
#endif

  /** @brief Parsed value getter.
   *
   * @return Const reference to a parsed value.
   *
   * @throw std::runtime_error Thrown if the value is unset (no value was
   * parsed or #pop was called).
   */
  const Type &get() const;

  /** @brief Get the parsed value and unset the parser.
   *
   * Moves the parsed value out of the parser.
   *
   * @note If you want to use SCustomObject inside this parser, you need to
   * provide both a copy constructor or move constructor and a copy assignment
   * operators in the SCustomObject::Type, they are used by parser.
   *
   * @return Rvalue reference to the parsed value.
   *
   * @throw std::runtime_error Thrown if the value is unset (no value was
   * parsed or #pop was called).
   */
  Type &&pop();

 private:
  using TokenParser::_set;
  using TokenParser::checkSet;

  void finish() override;
  void reset() override;

  // This is placed in the private section because the ValueSetter uses pop on
  // all fields, so they are always unset after parsing.
  using Union<TypeFieldT, Ts...>::get;
  using Union<TypeFieldT, Ts...>::pop;
  using Union<TypeFieldT, Ts...>::currentMemberId;

  template <size_t, typename...> struct ValueSetter {
    ValueSetter(Type & /*value*/, SUnion<TypeFieldT, Ts...> & /*parser*/) {}
  };

  template <size_t n, typename T, typename... TDs>
  struct ValueSetter<n, T, TDs...> : private ValueSetter<n + 1, TDs...> {
    ValueSetter(Type &value, SUnion<TypeFieldT, Ts...> &parser);
  };

  Type _value;
  Type _default_value;
  bool _allow_default_value;
  std::function<bool(const Type &)> _on_finish;
};

/** @brief %Map parser.
 *
 * Parses an object where each field is of type @ref Map_T "T", and field
 * name represents map key.
 *
 * @tparam T Underlying parser type.
 * @anchor Map_T
 */

template <typename T> class Map : public TokenParser {
 public:
  /** Arguments for the underlying parser */
  using ChildArgs = typename T::Args;

  /** @cond Underlying parser type */
  using ParserType = T;
  /** @endcond */

  /** Child arguments for the underlying type */
  template <typename U = Map<T>>
  using GrandChildArgs = typename U::ParserType::ChildArgs;

  /** @brief Struct with arguments for the Map @ref Map() "constructor". */
  struct Args {
    /** @param [in] args (optional) Sets #args.
     *
     * @param[in] on_key (optional) Sets #on_key.
     *
     * @param[in] on_finish (optional) Sets #on_finish.
     */
    Args(const ChildArgs &args = {},
         const std::function<bool(const std::string &, T &)> &on_key = nullptr,
         const std::function<bool(Map<T> &)> &on_finish = nullptr);

    /** @param [in] args Sets #args.
     *
     * @param[in] on_finish (optional) Sets #on_finish.
     */
    Args(const ChildArgs &args, const std::function<bool(Map<T> &)> &on_finish);

    /** @param[in] on_key Sets #on_key.
     *
     * @param[in] on_finish (optional) Sets #on_finish.
     */
    Args(const std::function<bool(const std::string &, T &)> &on_key,
         const std::function<bool(Map<T> &)> &on_finish = nullptr);

    /** @param[in] on_finish Sets #on_finish.
     */
    Args(const std::function<bool(Map<T> &)> &on_finish);

    /** @param [in] args Sets #args.
     *
     * @param[in] on_key (optional) Sets #on_key.
     *
     * @param[in] on_finish (optional) Sets #on_finish.
     */
    template <typename U = Map<T>>
    Args(const GrandChildArgs<U> &args,
         const std::function<bool(const std::string &, T &)> &on_key = nullptr,
         const std::function<bool(Map<T> &)> &on_finish = nullptr);

    /** @param [in] args Sets #args.
     *
     * @param[in] on_finish Sets #on_finish.
     */
    template <typename U = Map<T>>
    Args(const GrandChildArgs<U> &args,
         const std::function<bool(Map<T> &)> &on_finish);

    /** Arguments for the underlying parser */
    ChildArgs args;

    /** Callback, that will be called after each key's value is parsed.
     *
     * The callback will be called with a field name and a reference to the
     * child parser as arguments.
     *
     * If the callback returns false, parsing will be stopped with an error.
     */
    std::function<bool(const std::string &, T &)> on_key;

    /** Callback, that will be called after an object is parsed.
     *
     * The callback will be called with a reference to the parser as an
     * argument.
     * This reference is mostly useless, it's main purpose it to help the
     * compiler.
     *
     * If the callback returns false, parsing will be stopped with an error.
     */
    std::function<bool(Map<T> &)> on_finish;
  };

  /** @brief Map constructor.
   *
   * @param [in] args Args stucture.
   * If you do not specify @ref Args::on_finish "on_finish" and
   * @ref Args::on_key "on_key" callbacks, you can pass a
   * @ref Args::args "underlying parser arguments" directly into the
   * constructor.
   */
  Map(const Args &args);
  Map(const Map &) = delete;

  /** @cond Internal */
  void setDispatcher(Dispatcher *dispatcher) noexcept override;
  /** @endcond */

 private:
  void on(MapStartT /*unused*/) override;
  void on(MapKeyT key) override;
  void on(MapEndT /*unused*/) override;

  void childParsed() override;
  void finish() override;

  T _parser;
  std::string _current_key;
  std::function<bool(const std::string &, T &)> _on_key;
  std::function<bool(Map<T> &)> _on_finish;
};

/** @brief %Array parser.
 *
 * @tparam T Underlying parser type.
 */

template <typename T> class Array : public ArrayParser {
 public:
  /** Arguments for the underlying parser */
  using ChildArgs = typename T::Args;

  /** @cond Underlying parser type */
  using ParserType = T;
  /** @endcond */

  /** Child arguments for the underlying type */
  template <typename U = Array<T>>
  using GrandChildArgs = typename U::ParserType::ChildArgs;

  /** @brief Struct with arguments for the Array @ref Array() "constructor". */
  struct Args {
    /** @param [in] args Sets #args.
     *
     * @param[in] on_finish (optional) Sets #on_finish.
     */
    Args(const ChildArgs &args,
         const std::function<bool(Array<T> &)> &on_finish = nullptr);

    /** @param [in] args Sets #args.
     *
     * @param[in] on_finish (optional) Sets #on_finish.
     */
    template <typename U = Array<T>>
    Args(const GrandChildArgs<U> &args,
         const std::function<bool(Array<T> &)> &on_finish = nullptr);

    /** Arguments for the underlying parser */
    ChildArgs args;

    /** Callback, that will be called after an object is parsed.
     *
     * The callback will be called with a reference to the parser as an
     * argument.
     * This reference is mostly useless, it's main purpose it to help the
     * compiler.
     *
     * If the callback returns false, parsing will be stopped with an error.
     */
    std::function<bool(Array<T> &)> on_finish;
  };

  /** @brief Array constructor.
   *
   * @param [in] args Args stucture.
   * If you do not specify @ref Args::on_finish "on_finish" callback, you can
   * pass a @ref Args::args "underlying parser arguments" directly into the
   * constructor.
   */
  Array(const Args &args);
  Array(const Array &) = delete;

 protected:
  /** @cond Internal */
  T _parser;
  /** @endcond */

 private:
  void finish() override;

  std::function<bool(Array<T> &)> _on_finish;
};

/** @brief %Array parser, that stores the result in an std::vector of
 * @ref SArray_T "T" values.
 *
 * @tparam T Underlying parser type.
 * @anchor SArray_T
 */

template <typename T> class SArray : public Array<T> {
 public:
  /** Arguments for the underlying parser */
  using ChildArgs = typename T::Args;

  /** Underlying parser value type */
  using EltType = typename T::Type;

  /** Stored value type */
  using Type = std::vector<EltType>;

  /** Finish callback type */
  using CallbackType = std::function<bool(const Type &)>;

  /** @cond Underlying parser type */
  using ParserType = T;
  /** @endcond */

  /** Child arguments for the underlying type */
  template <typename U = Array<T>>
  using GrandChildArgs = typename U::ParserType::ChildArgs;

  /** @brief Struct with arguments for the SArray @ref SArray() "constructor".
   */
  struct Args {
    /** @param [in] args Sets #args.
     *
     * @param[in] on_finish (optional) Sets #on_finish.
     */
    Args(const ChildArgs &args = {}, const CallbackType &on_finish = nullptr);

    /** @param [in] args Sets #args.
     *
     * @param[in] on_finish (optional) Sets #on_finish.
     */
    template <typename U = Array<T>>
    Args(const GrandChildArgs<U> &args,
         const CallbackType &on_finish = nullptr);

    /**@param[in] on_finish (optional) Sets #on_finish. */
    Args(const CallbackType &on_finish);

    /** Arguments for the underlying parser */
    ChildArgs args;
    /** Callback, that will be called after an object is parsed.
     *
     * The callback will be called with a reference to the parser as an
     * argument.
     * This reference is mostly useless, it's main purpose it to help the
     * compiler.
     *
     * If the callback returns false, parsing will be stopped with an error.
     */
    std::function<bool(const Type &)> on_finish;
  };

  /** @brief Array constructor.
   *
   * @param [in] args Args stucture.
   * If you do not specify @ref Args::on_finish "on_finish" callback, you can
   * pass a @ref Args::args "underlying parser arguments" directly into the
   * constructor.
   * If you do not specify @ref Args::args "underlying parser arguments", you
   * can pass a @ref Args::on_finish "on_finish" callback directly into the
   * constructor.
   */
  SArray(const Args &args);
  SArray(const SArray &) = delete;

#ifdef DOXYGEN_ONLY
  /** @brief Check if the parser has a value.
   *
   * @return True if the parser has some value stored or false otherwise.
   */
  bool isSet();
#endif

  /** @brief Parsed value getter.
   *
   * @return Const reference to a parsed value.
   *
   * @throw std::runtime_error Thrown if the value is unset (no value was
   * parsed or #pop was called).
   */
  const Type &get() const;

  /** @brief Get the parsed value and unset the parser.
   *
   * Moves the parsed value out of the parser.
   *
   * @return Rvalue reference to the parsed value.
   *
   * @throw std::runtime_error Thrown if the value is unset (no value was
   * parsed or #pop was called).
   */
  Type &&pop();

 private:
  using TokenParser::_set;
  using TokenParser::checkSet;

  void childParsed() override;
  void finish() override;
  void reset() override;

  std::vector<EltType> _values;
  std::function<bool(const Type &)> _on_finish;
};

/** @brief Main parser.
 *
 * @tparam T Root parser (Value, Object, SObject, Union, Array, SArray)
 * @anchor Parser_T
 *
 * @tparam Impl Underlying parser implementation, the default one is YajlParser,
 * a YAJL parser adapter. This class inherits from the implementation, so please
 * refer to it's documentation for API details.
 */

template <typename T, typename Impl = YajlParser> class Parser : public Impl {
 public:
  /** @brief Parser constructor.
   *
   * @param [in] args Takes same arguments as the @ref Parser_T "T" does.
   */
  Parser(const typename T::Args &args = {});

  /** @brief Parser constructor.
   *
   * @param [in] args Takes same arguments as the @ref Parser_T "T" does.
   */
  template <typename U = T> Parser(const typename U::ChildArgs &args);

  /** @brief Parser constructor.
   *
   * @param [in] args Takes same arguments as the @ref Parser_T "T" does.
   */
  template <typename U = T>
  Parser(const typename U::template GrandChildArgs<U> &args);

  /** @brief Parser constructor.
   *
   * @param [in] callback Takes same arguments as the @ref Parser_T "T" does.
   */
  template <typename U = T> Parser(const typename U::CallbackType &callback);

  /** @brief Root parser getter.
   *
   * @return A reference to the root parser (@ref Parser_T "T").
   */
  T &parser();

 private:
  T _parser;
};
}  // namespace SJParser

#include "sjparser_impl.h"
