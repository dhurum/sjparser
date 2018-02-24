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
SArray<T>::SArray(T &&parser, CallbackT on_finish)
    : Array<T>(std::forward<T>(parser)), _on_finish(std::move(on_finish)) {
  static_assert(std::is_base_of_v<TokenParser, ParserType>,
                "Invalid parser used in SArray");
  static_assert(std::is_constructible_v<Callback, CallbackT>,
                "Invalid callback type");
}

template <typename T>
SArray<T>::SArray(SArray &&other) noexcept
    : Array<T>(std::move(other)), _on_finish(std::move(other._on_finish)) {}

template <typename T> void SArray<T>::setFinishCallback(Callback on_finish) {
  _on_finish = on_finish;
}

template <typename T> const typename SArray<T>::Type &SArray<T>::get() const {
  checkSet();
  return _values;
}

template <typename T> typename SArray<T>::Type &&SArray<T>::pop() {
  checkSet();
  TokenParser::_set = false;
  return std::move(_values);
}

template <typename T> void SArray<T>::childParsed() {
  _values.push_back(Array<T>::_parser.pop());
}

template <typename T> void SArray<T>::finish() {
  if (_on_finish && !_on_finish(_values)) {
    throw std::runtime_error("Callback returned false");
  }
}

template <typename T> void SArray<T>::reset() {
  ArrayParser::reset();
  _values = Type();
}
}  // namespace SJParser
