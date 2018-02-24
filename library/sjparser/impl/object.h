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
