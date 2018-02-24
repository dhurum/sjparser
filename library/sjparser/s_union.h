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

#include "union.h"

#include <functional>
#include <variant>

namespace SJParser {

/** @brief %Union of @ref Object "Objects" parser, that stores the result in an
 * std::variant of member parser types.
 *
 * Parses an object from @ref SUnion_Ts "Ts" list based on a value of the type
 * member.
 *
 * You can use it standalone (in this case the first member of an object must
 * be a type member) or embedded in an object (in this case object members after
 * the type member will be parsed by one of union's object parsers).
 *
 * SUnion type is defined by arguments passed to the constructor.
 *
 * Empty standalone union will be parsed and marked as unset.
 *
 * If union type was parsed, then the corresponding object is mandatory
 * unless it's member has Presence::Optional and default value set.
 *
 * @tparam TypeMemberT A type of the type member. Can be int64_t, bool, double
 * or std::string.
 *
 * @tparam Ts A list of object parsers.
 * @anchor SUnion_Ts
 */

template <typename TypeMemberT, typename... Ts>
class SUnion : public Union<TypeMemberT, Ts...> {
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
  using Type = std::variant<typename std::decay_t<Ts>::Type...>;

  /** Finish callback type. */
  using Callback = std::function<bool(const Type &)>;

  /** @brief Embedded mode constructor.
   *
   * This union must be used as a member of an object. In JSON that member's
   * content will represent this union's type. Latter members will be parsed by
   * one of this union's objects parsers.
   *
   * @param [in] type Type member's type, please use a TypeHolder wrapper for
   * it.
   *
   * @param [in] members std::tuple of Member structures, describing union
   * objects.
   *
   * @param [in] on_finish (optional) Callback, that will be called after the
   * union is parsed.
   * The callback will be called with a reference to the parser as an argument.
   * If the callback returns false, parsing will be stopped with an error.
   */
  template <typename CallbackT = std::nullptr_t>
  SUnion(TypeHolder<TypeMemberT> type,
         std::tuple<Member<TypeMemberT, Ts>...> members,
         CallbackT on_finish = nullptr);

  /** @brief Standalone mode constructor.
   *
   * This union will parse a whole JSON object. Object's first member's name
   * must be same as type_member, and latter members will be parsed by one of
   * this union's objects parsers.
   *
   * @param [in] type Type member's type, please use a TypeHolder wrapper for
   * it.
   *
   * @param [in] type_member Type member's name.
   *
   * @param [in] members std::tuple of Member structures, describing union
   * objects.
   *
   * @param [in] on_finish (optional) Callback, that will be called after the
   * union is parsed.
   * The callback will be called with a reference to the parser as an argument.
   * If the callback returns false, parsing will be stopped with an error.
   */
  template <typename CallbackT = std::nullptr_t>
  SUnion(TypeHolder<TypeMemberT> type, std::string type_member,
         std::tuple<Member<TypeMemberT, Ts>...> members,
         CallbackT on_finish = nullptr);

  /** Move constructor. */
  SUnion(SUnion &&other) noexcept;

  /** @brief Finish callback setter.
   *
   * @param [in] on_finish Callback, that will be called after the
   * union is parsed.
   * The callback will be called with a reference to the parser as an argument.
   * If the callback returns false, parsing will be stopped with an error.
   */
  void setFinishCallback(Callback on_finish);

#ifdef DOXYGEN_ONLY
  /** @brief Check if the parser has a value.
   *
   * @return True if the parser has some value stored or false otherwise.
   */
  bool isSet();

  /** @brief Check if the parsed union was empy (null).
   *
   * @return True if the parsed union was empty (null) or false otherwise.
   */
  bool isEmpty();
#endif

  /** @brief Parsed value getter.
   *
   * @return Const reference to a parsed value.
   *
   * @throw std::runtime_error Thrown if the value is unset (no value was
   * parsed or #pop was called).
   */
  const Type &get() const;

#ifdef DOXYGEN_ONLY
  /** @brief Member parser getter.
   *
   * @tparam n Index of the parser's member.
   *
   * @return Reference to n-th member parser.
   */
  template <size_t n> typename NthTypes<n, Ts...>::ParserType &parser();
#endif

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
  using TokenParser::checkSet;

  void finish() override;
  void reset() override;

  // This is placed in the private section because the ValueSetter uses pop on
  // all members, so they are always unset after parsing.
  using Union<TypeMemberT, Ts...>::get;
  using Union<TypeMemberT, Ts...>::pop;
  using Union<TypeMemberT, Ts...>::currentMemberId;

  template <size_t, typename...> struct ValueSetter {
    ValueSetter(Type & /*value*/, SUnion<TypeMemberT, Ts...> & /*parser*/) {}
  };

  template <size_t n, typename T, typename... TDs>
  struct ValueSetter<n, T, TDs...> : private ValueSetter<n + 1, TDs...> {
    ValueSetter(Type &value, SUnion<TypeMemberT, Ts...> &parser);
  };

  Type _value;
  Callback _on_finish;
};
}  // namespace SJParser

#include "impl/s_union.h"
