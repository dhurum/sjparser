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
#include "type_holder.h"

namespace SJParser {

/** @brief %Object parser, that stores parsed value of type
 * @ref SCustomObject_TypeT "TypeT"
 *
 * Stored value is set from a finish callback.
 *
 * @tparam TypeT Stored value type. It must have a default constructor. If you
 * want to include this parser into SArray, a move constructor of this type will
 * be used if possible.
 * @anchor SCustomObject_TypeT
 *
 * @tparam Ts A list of member parsers types.
 */

template <typename TypeT, typename... Ts>
class SCustomObject : public Object<Ts...> {
 public:
#ifdef DOXYGEN_ONLY
  /** @brief %Member parser type.
   *
   * Resolves to n-th member parser type
   *
   * @tparam n %Member index
   */
  template <size_t n> struct ParserType {
    /** n-th member parser type */
    using ParserType = NthTypes<n, TDs...>::ParserType;
  };
#endif

  /** Stored value type */
  using Type = TypeT;

  /** Finish callback type. */
  using Callback = std::function<bool(SCustomObject<Type, Ts...> &, Type &)>;

  /** @brief Constructor.
   *
   * @param [in] type Stored value type, please use a TypeHolder wrapper for it.
   *
   * @param [in] members std::tuple of Member structures, describing object
   * members.
   *
   * @param [in] on_finish (optional) Callback, that will be called after the
   * object is parsed.
   * The callback will be called with a reference to the parser and a reference
   * to the stored value as arguments. If the callback returns false, parsing
   * will be stopped with an error.
   */
  template <typename CallbackT = std::nullptr_t>
  SCustomObject(TypeHolder<Type> type,
                std::tuple<Member<std::string, Ts>...> members,
                CallbackT on_finish = nullptr,
                std::enable_if_t<std::is_constructible_v<Callback, CallbackT>>
                    * /*unused*/
                = 0);

  /** @brief Constructor.
   *
   * @param [in] type Stored value type, please use a TypeHolder wrapper for it.
   *
   * @param [in] members std::tuple of Member structures, describing object
   * members.
   *
   * @param [in] options Additional options for the parser.
   *
   * @param [in] on_finish (optional) Callback, that will be called after the
   * object is parsed.
   * The callback will be called with a reference to the parser and a reference
   * to the stored value as arguments. If the callback returns false, parsing
   * will be stopped with an error.
   */
  template <typename CallbackT = std::nullptr_t>
  SCustomObject(TypeHolder<Type> type,
                std::tuple<Member<std::string, Ts>...> members,
                ObjectOptions options, CallbackT on_finish = nullptr);

  /** Move constructor. */
  SCustomObject(SCustomObject &&other) noexcept;

  /** @brief Finish callback setter.
   *
   * @param [in] on_finish Callback, that will be called after the
   * object is parsed.
   * The callback will be called with a reference to the parser and a reference
   * to the stored value as arguments. If the callback returns false, parsing
   * will be stopped with an error.
   */
  void setFinishCallback(Callback on_finish);

#ifdef DOXYGEN_ONLY
  /** @brief Check if the parser has a value.
   *
   * @return True if the parser has some value stored or false otherwise.
   */
  bool isSet();
#endif

#ifdef DOXYGEN_ONLY
  /** @brief Universal member getter.
   *
   * @tparam n Index of the parser's member.
   *
   * @return If the n-th member parser stores value (is a Value, SAutoObject,
   * SCustomObject, SUnion or SArray), then the method returns a const reference
   * to the n-th member parser value. Otherwise, returns a reference to the n-th
   * member parser.
   *
   * @throw std::runtime_error thrown if the member parser value is unset (no
   * value was parsed or #pop was called for the member parser).
   */
  template <size_t n> auto &get();

  /** @brief Member parser getter.
   *
   * @tparam n Index of the parser's member.
   *
   * @return Reference to n-th member parser.
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
  /** @brief Get the member parsed value and unset the member parser.
   *
   * Moves the n-th member parsed value out of the member parser.
   *
   * @tparam n Index of the parser's member.
   *
   * @return Rvalue reference to n-th member parser value.
   *
   * @throw std::runtime_error thrown if the member parser value is unset (no
   * value was parsed or #pop was called for the member parser).
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
  Callback _on_finish;
};
}  // namespace SJParser

#include "impl/s_custom_object.h"
