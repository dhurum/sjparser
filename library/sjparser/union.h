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

#include "internals/key_value_parser.h"

#include <functional>

namespace SJParser {

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
  /** @cond INTERNAL Internal typedef */
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
}  // namespace SJParser

#include "impl/union.h"
