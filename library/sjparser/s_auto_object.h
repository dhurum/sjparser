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

/** @brief %Object parser, that stores the result in an std::tuple of member
 * parsers types.
 *
 * All object members are mandatory, except for members with Presence::Optional
 * and default value set.
 *
 * Unknown members will cause parsing error unless ObjectOptions is passed to
 * the constructor with unknown_member set to Reaction::Ignore.
 *
 * Empty object will be parsed and marked as unset.
 *
 * @tparam Ts A list of member parsers types.
 */

template <typename... Ts> class SAutoObject : public Object<Ts...> {
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
  using Type = std::tuple<typename std::decay_t<Ts>::Type...>;

  /** Finish callback type. */
  using Callback = std::function<bool(const Type &)>;

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
  SAutoObject(std::tuple<Member<std::string, Ts>...> members,
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
  SAutoObject(std::tuple<Member<std::string, Ts>...> members,
              ObjectOptions options, CallbackT on_finish = nullptr);

  /** Move constructor. */
  SAutoObject(SAutoObject &&other) noexcept;

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
   * @return True if the parser has some value stored or false otherwise.
   */
  bool isSet();

  /** @brief Check if the parsed object was empy (null).
   *
   * @return True if the parsed object was empty (null) or false otherwise.
   */
  bool isEmpty();
#endif

  /** @brief Parsed value getter.
   *
   * @return Const reference to a parsed value or a default value (if a default
   * value is set and the member is not present).
   *
   * @throw std::runtime_error Thrown if the value is unset (no value was
   * parsed and no default value was specified or #pop was called).
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

#ifdef DOXYGEN_ONLY
  /** @brief Member parser getter.
   *
   * @tparam n Index of the parser's member.
   *
   * @return Reference to n-th member parser.
   */
  template <size_t n> typename NthTypes<n, Ts...>::ParserType &parser();
#endif

 private:
  using TokenParser::checkSet;

  void finish() override;
  void reset() override;

  // This is placed in the private section because the ValueSetter uses pop on
  // all members, so they are always unset after parsing.
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
  Callback _on_finish;
};

/****************************** Implementations *******************************/

template <typename... Ts>
template <typename CallbackT>
SAutoObject<Ts...>::SAutoObject(
    std::tuple<Member<std::string, Ts>...> members, CallbackT on_finish,
    std::enable_if_t<std::is_constructible_v<Callback, CallbackT>> * /*unused*/)
    : Object<Ts...>(std::move(members), {}), _on_finish(std::move(on_finish)) {}

template <typename... Ts>
template <typename CallbackT>
SAutoObject<Ts...>::SAutoObject(std::tuple<Member<std::string, Ts>...> members,
                                ObjectOptions options, CallbackT on_finish)
    : Object<Ts...>(std::move(members), options),
      _on_finish(std::move(on_finish)) {
  static_assert(std::is_constructible_v<Callback, CallbackT>,
                "Invalid callback type");
}

template <typename... Ts>
SAutoObject<Ts...>::SAutoObject(SAutoObject &&other) noexcept
    : Object<Ts...>(std::move(other)),
      _value{},
      _on_finish(std::move(other._on_finish)) {}

template <typename... Ts>
void SAutoObject<Ts...>::setFinishCallback(Callback on_finish) {
  _on_finish = on_finish;
}

template <typename... Ts>
const typename SAutoObject<Ts...>::Type &SAutoObject<Ts...>::get() const {
  checkSet();
  return _value;
}

template <typename... Ts>
typename SAutoObject<Ts...>::Type &&SAutoObject<Ts...>::pop() {
  checkSet();
  TokenParser::_set = false;
  return std::move(_value);
}

template <typename... Ts> void SAutoObject<Ts...>::finish() {
  if (TokenParser::isEmpty()) {
    TokenParser::_set = false;
    return;
  }

  try {
    ValueSetter<0, Ts...>(_value, *this);
  } catch (std::exception &e) {
    TokenParser::_set = false;
    throw std::runtime_error(std::string("Can not set value: ") + e.what());
  } catch (...) {
    TokenParser::_set = false;
    throw std::runtime_error("Can not set value: unknown exception");
  }

  if (_on_finish && !_on_finish(_value)) {
    throw std::runtime_error("Callback returned false");
  }
}

template <typename... Ts> void SAutoObject<Ts...>::reset() {
  Object<Ts...>::KVParser::reset();
  _value = {};
}

template <typename... Ts>
template <size_t n, typename T, typename... TDs>
SAutoObject<Ts...>::ValueSetter<n, T, TDs...>::ValueSetter(
    Type &value, SAutoObject<Ts...> &parser)
    : ValueSetter<n + 1, TDs...>(value, parser) {
  auto &member = parser._member_parsers.template get<n>();

  if (member.parser.isSet()) {
    std::get<n>(value) = member.parser.pop();
  } else if (member.optional) {
    if (!member.default_value.present) {
      throw std::runtime_error("Optional member " + member.name
                               + " does not have a default value");
    }
    std::get<n>(value) = member.default_value.value;
  } else {
    throw std::runtime_error("Mandatory member " + member.name
                             + " is not present");
  }
}

}  // namespace SJParser
