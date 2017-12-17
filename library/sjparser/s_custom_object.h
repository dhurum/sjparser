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

#include "object.h"

namespace SJParser {

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
}  // namespace SJParser

#include "impl/s_custom_object.h"
