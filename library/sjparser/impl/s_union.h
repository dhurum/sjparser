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
SUnion<TypeMemberT, Ts...>::SUnion(
    TypeHolder<TypeMemberT> type,
    std::tuple<Member<TypeMemberT, Ts>...> members, CallbackT on_finish)
    : Union<TypeMemberT, Ts...>(type, std::move(members)),
      _on_finish(std::move(on_finish)) {}

template <typename TypeMemberT, typename... Ts>
template <typename CallbackT>
SUnion<TypeMemberT, Ts...>::SUnion(
    TypeHolder<TypeMemberT> type, std::string type_member,
    std::tuple<Member<TypeMemberT, Ts>...> members, CallbackT on_finish)
    : Union<TypeMemberT, Ts...>(type, std::move(type_member),
                                std::move(members)),
      _on_finish(std::move(on_finish)) {}

template <typename TypeMemberT, typename... Ts>
SUnion<TypeMemberT, Ts...>::SUnion(SUnion &&other) noexcept
    : Union<TypeMemberT, Ts...>(std::move(other)),
      _value{Type{}},
      _on_finish(std::move(other._on_finish)) {}

template <typename TypeMemberT, typename... Ts>
void SUnion<TypeMemberT, Ts...>::setFinishCallback(Callback on_finish) {
  _on_finish = on_finish;
}

template <typename TypeMemberT, typename... Ts>
const typename SUnion<TypeMemberT, Ts...>::Type &
SUnion<TypeMemberT, Ts...>::get() const {
  checkSet();
  return _value;
}

template <typename TypeMemberT, typename... Ts>
typename SUnion<TypeMemberT, Ts...>::Type &&SUnion<TypeMemberT, Ts...>::pop() {
  checkSet();
  TokenParser::_set = false;
  return std::move(_value);
}

template <typename TypeMemberT, typename... Ts>
void SUnion<TypeMemberT, Ts...>::finish() {
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

template <typename TypeMemberT, typename... Ts>
void SUnion<TypeMemberT, Ts...>::reset() {
  Union<TypeMemberT, Ts...>::reset();
  _value = Type{};
}

template <typename TypeMemberT, typename... Ts>
template <size_t n, typename T, typename... TDs>
SUnion<TypeMemberT, Ts...>::ValueSetter<n, T, TDs...>::ValueSetter(
    Type &value, SUnion<TypeMemberT, Ts...> &parser)
    : ValueSetter<n + 1, TDs...>(value, parser) {
  if (parser.currentMemberId() != n) {
    return;
  }

  auto &member = parser._member_parsers.template get<n>();

  if (member.parser.isSet()) {
    value = member.parser.pop();
  } else if (member.optional) {
    if (!member.default_value.present) {
      std::stringstream error;
      error << "Optional member #" << n << " does not have a default value";
      throw std::runtime_error(error.str());
    }
    value = member.default_value.value;
  } else {
    std::stringstream error;
    error << "Mandatory member #" << n << " is not present";
    throw std::runtime_error(error.str());
  }
}
}  // namespace SJParser
