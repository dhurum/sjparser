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
 * @tparam Ts A list of member parsers types.
 */

template <typename... Ts>
class Object : public KeyValueParser<std::string, Ts...> {
 protected:
  /** @cond INTERNAL Internal typedef */
  using KVParser = KeyValueParser<std::string, Ts...>;
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
    using ParserType = NthTypes<n, TDs...>::ParserType;
  };
#endif

  /** Finish callback type. */
  using Callback = std::function<bool(Object<Ts...> &)>;

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
  Object(std::tuple<Member<std::string, Ts>...> members,
         CallbackT on_finish = nullptr,
         std::enable_if_t<std::is_constructible_v<Callback, CallbackT>>
             * /*unused*/
         = 0);

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
  Object(std::tuple<Member<std::string, Ts>...> members, ObjectOptions options,
         CallbackT on_finish = nullptr);

  /** Move constructor. */
  Object(Object &&other) noexcept;

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
  bool isSet();

  /** @brief Check if the parsed object was empy (null).
   *
   * @return True if the parsed object was empty (null) or false otherwise.
   */
  bool isEmpty();

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
  template <size_t n> auto &get();

  /** @brief Member parser getter.
   *
   * @tparam n Index of the parser's member.
   *
   * @return Reference to n-th member parser.
   */
  template <size_t n> typename NthTypes<n, Ts...>::ParserType &parser();

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
  template <size_t n> typename NthTypes<n, Ts...>::template ValueType<> &&pop();
#endif

 protected:
  template <size_t, typename...> struct MemberChecker {
    MemberChecker(Object<Ts...> & /*parser*/) {}
  };

  template <size_t n, typename T, typename... TDs>
  struct MemberChecker<n, T, TDs...> : private MemberChecker<n + 1, TDs...> {
    MemberChecker(Object<Ts...> &parser);
  };

 private:
  using KVParser::on;
  void on(MapKeyT key) override;

  void finish() override;

  Callback _on_finish;
};

/****************************** Implementations *******************************/

template <typename... Ts>
template <typename CallbackT>
Object<Ts...>::Object(
    std::tuple<Member<std::string, Ts>...> members, CallbackT on_finish,
    std::enable_if_t<std::is_constructible_v<Callback, CallbackT>> * /*unused*/)
    : KVParser(std::move(members), {}), _on_finish(std::move(on_finish)) {}

template <typename... Ts>
template <typename CallbackT>
Object<Ts...>::Object(std::tuple<Member<std::string, Ts>...> members,
                      ObjectOptions options, CallbackT on_finish)
    : KVParser(std::move(members), options), _on_finish(std::move(on_finish)) {
  static_assert(std::is_constructible_v<Callback, CallbackT>,
                "Invalid callback type");
}

template <typename... Ts>
Object<Ts...>::Object(Object &&other) noexcept
    : KVParser(std::move(other)), _on_finish(std::move(other._on_finish)) {}

template <typename... Ts>
void Object<Ts...>::setFinishCallback(Callback on_finish) {
  _on_finish = on_finish;
}

template <typename... Ts> void Object<Ts...>::on(MapKeyT key) {
  KVParser::onMember(key.key);
}

template <typename... Ts> void Object<Ts...>::finish() {
  if (TokenParser::isEmpty()) {
    TokenParser::_set = false;
    return;
  }

  try {
    MemberChecker<0, Ts...>(*this);
  } catch (std::exception &e) {
    TokenParser::_set = false;
    throw;
  }

  if (_on_finish && !_on_finish(*this)) {
    throw std::runtime_error("Callback returned false");
  }
}

template <typename... Ts>
template <size_t n, typename T, typename... TDs>
Object<Ts...>::MemberChecker<n, T, TDs...>::MemberChecker(Object<Ts...> &parser)
    : MemberChecker<n + 1, TDs...>(parser) {
  auto &member = parser._member_parsers.template get<n>();

  if (!member.parser.isSet() && !member.optional) {
    throw std::runtime_error("Mandatory member " + member.name
                             + " is not present");
  }
}

}  // namespace SJParser
