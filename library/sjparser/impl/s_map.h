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

template <typename T>
template <typename CallbackT>
SMap<T>::SMap(
    T &&parser, CallbackT on_finish,
    std::enable_if_t<std::is_constructible_v<Callback, CallbackT>> * /*unused*/)
    : Map<T>(std::forward<T>(parser)), _on_finish(std::move(on_finish)) {
  static_assert(std::is_base_of_v<TokenParser, ParserType>,
                "Invalid parser used in Map");
}

template <typename T>
template <typename ElementCallbackT, typename CallbackT>
SMap<T>::SMap(T &&parser, ElementCallbackT on_element, CallbackT on_finish)
    : Map<T>(std::forward<T>(parser)),
      _on_element(std::move(on_element)),
      _on_finish(std::move(on_finish)) {
  static_assert(std::is_base_of_v<TokenParser, ParserType>,
                "Invalid parser used in Map");
  static_assert(std::is_constructible_v<Callback, CallbackT>,
                "Invalid callback type");
}

template <typename T>
SMap<T>::SMap(SMap &&other) noexcept
    : Map<T>(std::move(other)),
      _values{},
      _on_element(std::move(other._on_element)),
      _on_finish(std::move(other._on_finish)) {}

template <typename T>
void SMap<T>::setElementCallback(ElementCallback on_element) {
  _on_element = on_element;
}

template <typename T> void SMap<T>::setFinishCallback(Callback on_finish) {
  _on_finish = on_finish;
}

template <typename T> const typename SMap<T>::Type &SMap<T>::get() const {
  checkSet();
  return _values;
}

template <typename T> typename SMap<T>::Type &&SMap<T>::pop() {
  checkSet();
  TokenParser::_set = false;
  return std::move(_values);
}

template <typename T> void SMap<T>::childParsed() {
  if (_on_element && !_on_element(Map<T>::_current_key, Map<T>::_parser)) {
    throw std::runtime_error("Element callback returned false");
  }
  _values.insert(std::pair(Map<T>::_current_key, Map<T>::_parser.pop()));
}

template <typename T> void SMap<T>::finish() {
  if (_on_finish && !_on_finish(*this)) {
    throw std::runtime_error("Callback returned false");
  }
}

template <typename T> void SMap<T>::reset() {
  TokenParser::reset();
  _values = Type();
}
}  // namespace SJParser
