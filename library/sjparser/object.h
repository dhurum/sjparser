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

/** @brief %Object parser.
 *
 * All object members are mandatory, except for members with Presence::Optional.
 * For absent members with default value Object::get<> will return the default
 * value and isSet will return false.
 *
 * Unknown members will cause parsing error unless ObjectOptions is passed to
 * the constructor with unknown_member set to Reaction::Ignore.
 *
 * Empty object will be parsed and marked as unset.
 *
 * @tparam ParserTs A list of member parsers types.
 */

template <typename... ParserTs>
class Object : public KeyValueParser<std::string, ParserTs...> {
 protected:
  /** @cond INTERNAL Internal typedef */
  using KVParser = KeyValueParser<std::string, ParserTs...>;
  /** @endcond */

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
    using ParserType = NthTypes<n, ParserTDs...>::ParserType;
  };
#endif

  /** Finish callback type. */
  using Callback = std::function<bool(Object<ParserTs...> &)>;

  /** @brief Constructor.
   *
   * @param [in] members std::tuple of Member structures, describing object
   * members.
   *
   * @param [in] on_finish (optional) Callback, that will be called after the
   * object is parsed.
   * The callback will be called with a reference to the parser as an argument.
   * If the callback returns false, parsing will be stopped with an error.
   */
  template <typename CallbackT = std::nullptr_t>
  explicit Object(std::tuple<Member<std::string, ParserTs>...> members,
                  CallbackT on_finish = nullptr,
                  std::enable_if_t<std::is_constructible_v<Callback, CallbackT>>
                      * /*unused*/
                  = nullptr);

  /** @brief Constructor.
   *
   * @param [in] members std::tuple of Member structures, describing object
   * members.
   *
   * @param [in] options Additional options for the parser.
   *
   * @param [in] on_finish (optional) Callback, that will be called after the
   * object is parsed.
   * The callback will be called with a reference to the parser as an argument.
   * If the callback returns false, parsing will be stopped with an error.
   */
  template <typename CallbackT = std::nullptr_t>
  Object(std::tuple<Member<std::string, ParserTs>...> members,
         ObjectOptions options, CallbackT on_finish = nullptr);

  /** Move constructor. */
  Object(Object &&other) noexcept;

  /** Move assignment operator */
  Object<ParserTs...> &operator=(Object &&other) noexcept;

  /** @cond INTERNAL Boilerplate. */
  ~Object() override = default;
  Object(const Object &) = delete;
  Object &operator=(const Object &) = delete;
  /** @endcond */

  /** @brief Finish callback setter.
   *
   * @param [in] on_finish Callback, that will be called after the
   * object is parsed.
   * The callback will be called with a reference to the parser as an argument.
   * If the callback returns false, parsing will be stopped with an error.
   */
  void setFinishCallback(Callback on_finish);

#ifdef DOXYGEN_ONLY
  /** @brief Check if the parser has a value.
   *
   * @return True if the parser parsed something or false otherwise.
   */
  [[nodiscard]] bool isSet();

  /** @brief Check if the parsed object was empy (null).
   *
   * @return True if the parsed object was empty (null) or false otherwise.
   */
  [[nodiscard]] bool isEmpty();

  /** @brief Universal member getter.
   *
   * @tparam n Index of the parser's member.
   *
   * @return If the n-th member parser stores value (is a Value, SAutoObject,
   * SCustomObject, SUnion or SArray), then the method returns a const reference
   * to the n-th member parser value or a default value (if a default value is
   * set and the member is not present). Otherwise, returns a reference to the
   * n-th member parser.
   *
   * @throw std::runtime_error thrown if the member parser value is unset (no
   * value was parsed and no default value was specified or #pop was called for
   * the member parser).
   */
  [[nodiscard]] template <size_t n> auto &get();

  /** @brief Member parser getter.
   *
   * @tparam n Index of the parser's member.
   *
   * @return Reference to n-th member parser.
   */
  [[nodiscard]] template <size_t n>
  typename NthTypes<n, ParserTs...>::ParserType &parser();

  /** @brief Get the member parsed value and unset the member parser.
   *
   * Moves the n-th member parsed value out of the member parser.
   *
   * @tparam n Index of the parser's member.
   *
   * @return Rvalue reference to the n-th member parser value or a default value
   * (if a default value is set and the member is not present).
   *
   * @throw std::runtime_error thrown if the member parser value is unset (no
   * value was parsed or #pop was called for the member parser).
   */
  template <size_t n>
  typename NthTypes<n, ParserTs...>::template ValueType<> &&pop();
#endif

 protected:
  template <size_t, typename...> struct MemberChecker {
    explicit MemberChecker(Object<ParserTs...> & /*parser*/) {}
  };

  template <size_t n, typename ParserT, typename... ParserTDs>
  struct MemberChecker<n, ParserT, ParserTDs...>
      : private MemberChecker<n + 1, ParserTDs...> {
    explicit MemberChecker(Object<ParserTs...> &parser);
  };

 private:
  using KVParser::on;
  void on(MapKeyT key) override;

  void finish() override;

  Callback _on_finish;
};

}  // namespace SJParser

namespace std {

template <typename... ParserTs>
struct tuple_size<SJParser::Object<ParserTs...>>
    : std::integral_constant<std::size_t, sizeof...(ParserTs)> {};

template <std::size_t n, typename... ParserTs>
struct tuple_element<n, SJParser::Object<ParserTs...>> {
  using type =
      decltype(std::declval<SJParser::Object<ParserTs...>>().template get<n>());
};

}  // namespace std

namespace SJParser {

/****************************** Implementations *******************************/

template <typename... ParserTs>
template <typename CallbackT>
Object<ParserTs...>::Object(
    std::tuple<Member<std::string, ParserTs>...> members, CallbackT on_finish,
    std::enable_if_t<std::is_constructible_v<Callback, CallbackT>> * /*unused*/)
    : Object{std::move(members), ObjectOptions{}, std::move(on_finish)} {}

template <typename... ParserTs>
template <typename CallbackT>
Object<ParserTs...>::Object(
    std::tuple<Member<std::string, ParserTs>...> members, ObjectOptions options,
    CallbackT on_finish)
    : KVParser{std::move(members), options}, _on_finish{std::move(on_finish)} {
  static_assert(std::is_constructible_v<Callback, CallbackT>,
                "Invalid callback type");
}

template <typename... ParserTs>
Object<ParserTs...>::Object(Object &&other) noexcept
    : KVParser{std::move(other)}, _on_finish{std::move(other._on_finish)} {}

template <typename... ParserTs>
Object<ParserTs...> &Object<ParserTs...>::operator=(Object &&other) noexcept {
  KVParser::operator=(std::move(other));
  _on_finish = std::move(other._on_finish);

  return *this;
}

template <typename... ParserTs>
void Object<ParserTs...>::setFinishCallback(Callback on_finish) {
  _on_finish = on_finish;
}

template <typename... ParserTs> void Object<ParserTs...>::on(MapKeyT key) {
  KVParser::onMember(key.key);
}

template <typename... ParserTs> void Object<ParserTs...>::finish() {
  if (TokenParser::isEmpty()) {
    TokenParser::unset();
    return;
  }

  try {
    MemberChecker<0, ParserTs...>(*this);
  } catch (std::exception &e) {
    TokenParser::unset();
    throw;
  }

  if (_on_finish && !_on_finish(*this)) {
    throw std::runtime_error("Callback returned false");
  }
}

template <typename... ParserTs>
template <size_t n, typename ParserT, typename... ParserTDs>
Object<ParserTs...>::MemberChecker<n, ParserT, ParserTDs...>::MemberChecker(
    Object<ParserTs...> &parser)
    : MemberChecker<n + 1, ParserTDs...>{parser} {
  auto &member = parser.memberParsers().template get<n>();

  if (!member.parser.isSet() && !member.optional) {
    throw std::runtime_error("Mandatory member " + member.name
                             + " is not present");
  }
}

}  // namespace SJParser
