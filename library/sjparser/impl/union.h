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

namespace SJParser {

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
