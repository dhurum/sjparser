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
