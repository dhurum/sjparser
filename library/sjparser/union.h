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
#include "type_holder.h"

#include <functional>

namespace SJParser {

/** @brief %Union of @ref Object "Objects" parser.
 *
 * Parses an object from @ref Union_Ts "Ts" list based on a value of the type
 * member.
 *
 * You can use it standalone (in this case the first member of an object must
 * be a type member) or embedded in an object (in this case object members after
 * the type member will be parsed by one of union's object parsers).
 *
 * Union type is defined by arguments passed to the constructor.
 *
 * Empty standalone union will be parsed and marked as unset.
 *
 * If union type was parsed, then the corresponding object is mandatory
 * unless it's member has Presence::Optional set.
 * For absent member with default value Union::get<> will return the default
 * value and isSet will return false.
 *
 * @tparam TypeMemberT A type of the type member. Can be int64_t, bool, double
 * or std::string.
 *
 * @tparam Ts A list of object parsers.
 * @anchor Union_Ts
 */

template <typename TypeMemberT, typename... Ts>
class Union : public KeyValueParser<TypeMemberT, Ts...> {
 protected:
  /** @cond INTERNAL Internal typedef */
  using KVParser = KeyValueParser<TypeMemberT, Ts...>;
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
  using Callback = std::function<bool(Union<TypeMemberT, Ts...> &)>;

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
  Union(TypeHolder<TypeMemberT> type,
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
  Union(TypeHolder<TypeMemberT> type, std::string type_member,
        std::tuple<Member<TypeMemberT, Ts>...> members,
        CallbackT on_finish = nullptr);

  /** Move constructor. */
  Union(Union &&other) noexcept;

  /** @brief Finish callback setter.
   *
   * @param [in] on_finish Callback, that will be called after the
   * union is parsed.
   * The callback will be called with a reference to the parser as an argument.
   * If the callback returns false, parsing will be stopped with an error.
   */
  void setFinishCallback(Callback on_finish);

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
   * @return True if the parser parsed something or false otherwise.
   */
  bool isSet();

  /** @brief Check if the parsed union was empy (null).
   *
   * @return True if the parsed union was empty (null) or false otherwise.
   */
  bool isEmpty();
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

 protected:
  using TokenParser::checkSet;

  void reset() override;

 private:
  void setupIdsMap();

  using KVParser::on;

  void on(TokenType<TypeMemberT> value) override;
  void on(MapStartT /*unused*/) override;
  void on(MapKeyT key) override;

  void childParsed() override;
  void finish() override;

  template <size_t, typename...> struct MemberChecker {
    MemberChecker(Union<TypeMemberT, Ts...> & /*parser*/) {}
  };

  template <size_t n, typename T, typename... TDs>
  struct MemberChecker<n, T, TDs...> : private MemberChecker<n + 1, TDs...> {
    MemberChecker(Union<TypeMemberT, Ts...> &parser);
  };

  std::string _type_member;
  Callback _on_finish;
  std::unordered_map<TokenParser *, size_t> _members_ids_map;
  size_t _current_member_id;
};

/****************************** Implementations *******************************/

template <typename TypeMemberT, typename... Ts>
template <typename CallbackT>
Union<TypeMemberT, Ts...>::Union(TypeHolder<TypeMemberT> /*type*/,
                                 std::tuple<Member<TypeMemberT, Ts>...> members,
                                 CallbackT on_finish)
    : KVParser(std::move(members), {}),
      _on_finish(std::move(on_finish)),
      _current_member_id(0) {
  static_assert(std::is_constructible_v<Callback, CallbackT>,
                "Invalid callback type");
  setupIdsMap();
}

template <typename TypeMemberT, typename... Ts>
template <typename CallbackT>
Union<TypeMemberT, Ts...>::Union(TypeHolder<TypeMemberT> /*type*/,
                                 std::string type_member,
                                 std::tuple<Member<TypeMemberT, Ts>...> members,
                                 CallbackT on_finish)
    : KVParser(std::move(members), {}),
      _type_member(std::move(type_member)),
      _on_finish(std::move(on_finish)),
      _current_member_id(0) {
  static_assert(std::is_constructible_v<Callback, CallbackT>,
                "Invalid callback type");
  setupIdsMap();
}

template <typename TypeMemberT, typename... Ts>
Union<TypeMemberT, Ts...>::Union(Union &&other) noexcept
    : KVParser(std::move(other)),
      _type_member(std::move(other._type_member)),
      _on_finish(std::move(other._on_finish)),
      _current_member_id(0) {
  setupIdsMap();
}

template <typename TypeMemberT, typename... Ts>
void Union<TypeMemberT, Ts...>::setupIdsMap() {
  _members_ids_map.clear();
  for (size_t i = 0; i < KVParser::_parsers_array.size(); ++i) {
    _members_ids_map[KVParser::_parsers_array[i]] = i;
  }
}

template <typename TypeMemberT, typename... Ts>
void Union<TypeMemberT, Ts...>::setFinishCallback(Callback on_finish) {
  _on_finish = on_finish;
}

template <typename TypeMemberT, typename... Ts>
size_t Union<TypeMemberT, Ts...>::currentMemberId() {
  checkSet();
  return _current_member_id;
}

template <typename TypeMemberT, typename... Ts>
void Union<TypeMemberT, Ts...>::reset() {
  _current_member_id = 0;
  KVParser::reset();
}

template <typename TypeMemberT, typename... Ts>
void Union<TypeMemberT, Ts...>::on(TokenType<TypeMemberT> value) {
  reset();
  KVParser::onMember(value);
  _current_member_id = _members_ids_map[KVParser::_parsers_map[value]];
}

template <typename TypeMemberT, typename... Ts>
void Union<TypeMemberT, Ts...>::on(MapStartT /*unused*/) {
  if (_type_member.empty()) {
    throw std::runtime_error(
        "Union with an empty type member can't parse this");
  }
  reset();
}

template <typename TypeMemberT, typename... Ts>
void Union<TypeMemberT, Ts...>::on(MapKeyT key) {
  if (_type_member.empty()) {
    throw std::runtime_error(
        "Union with an empty type member can't parse this");
  }
  if (key.key != _type_member) {
    std::stringstream err;
    err << "Unexpected member " << key.key;
    throw std::runtime_error(err.str());
  }
}

template <typename TypeMemberT, typename... Ts>
void Union<TypeMemberT, Ts...>::childParsed() {
  KVParser::endParsing();
  if (_type_member.empty()) {
    // The union embedded into an object must propagate the end event to the
    // parent.
    KVParser::_dispatcher->on(MapEndT());
  }
}

template <typename TypeMemberT, typename... Ts>
void Union<TypeMemberT, Ts...>::finish() {
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

template <typename TypeMemberT, typename... Ts>
template <size_t n, typename T, typename... TDs>
Union<TypeMemberT, Ts...>::MemberChecker<n, T, TDs...>::MemberChecker(
    Union<TypeMemberT, Ts...> &parser)
    : MemberChecker<n + 1, TDs...>(parser) {
  if (parser.currentMemberId() != n) {
    return;
  }

  auto &member = parser._member_parsers.template get<n>();

  if (!member.parser.isSet() && !member.optional) {
    std::stringstream error;
    error << "Mandatory member #" << n << " is not present";
    throw std::runtime_error(error.str());
  }
}

}  // namespace SJParser
