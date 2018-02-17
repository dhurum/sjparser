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

template <typename TypeT, typename... Ts>
template <typename CallbackT>
SCustomObject<TypeT, Ts...>::SCustomObject(
    TypeHolder<TypeT> /*type*/, std::tuple<Member<std::string, Ts>...> members,
    CallbackT on_finish,
    std::enable_if_t<std::is_constructible_v<Callback, CallbackT>> * /*unused*/)
    : Object<Ts...>(std::move(members), {}), _on_finish(std::move(on_finish)) {}

template <typename TypeT, typename... Ts>
template <typename CallbackT>
SCustomObject<TypeT, Ts...>::SCustomObject(
    TypeHolder<TypeT> /*type*/, std::tuple<Member<std::string, Ts>...> members,
    ObjectOptions options, CallbackT on_finish)
    : Object<Ts...>(std::move(members), options),
      _on_finish(std::move(on_finish)) {
  static_assert(std::is_constructible_v<Callback, CallbackT>,
                "Invalid callback type");
}

template <typename TypeT, typename... Ts>
SCustomObject<TypeT, Ts...>::SCustomObject(SCustomObject &&other) noexcept
    : Object<Ts...>(std::move(other)),
      _on_finish(std::move(other._on_finish)) {}

template <typename TypeT, typename... Ts>
void SCustomObject<TypeT, Ts...>::setFinishCallback(Callback on_finish) {
  _on_finish = on_finish;
}

template <typename TypeT, typename... Ts>
const typename SCustomObject<TypeT, Ts...>::Type &
SCustomObject<TypeT, Ts...>::get() const {
  checkSet();
  return _value;
}

template <typename TypeT, typename... Ts>
typename SCustomObject<TypeT, Ts...>::Type &&
SCustomObject<TypeT, Ts...>::pop() {
  checkSet();
  _set = false;
  return std::move(_value);
}

template <typename TypeT, typename... Ts>
void SCustomObject<TypeT, Ts...>::finish() {
  if (!_on_finish(*this, _value)) {
    throw std::runtime_error("Callback returned false");
  }
}

template <typename TypeT, typename... Ts>
void SCustomObject<TypeT, Ts...>::reset() {
  Object<Ts...>::KVParser::reset();
  _value = Type();
}
}  // namespace SJParser
