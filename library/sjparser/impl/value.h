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
Value<T>::Value(Callback on_finish) : _on_finish(std::move(on_finish)) {
  // Formatting disabled because of a bug in clang-format
  // clang-format off
  static_assert(
      std::is_same_v<T, int64_t>
      || std::is_same_v<T, bool>
      || std::is_same_v<T, double>
      || std::is_same_v<T, std::string>,
      "Invalid type used in Value, only int64_t, bool, double or std::string"
      " are allowed");
  // clang-format on
}

template <typename T>
Value<T>::Value(Value &&other) noexcept
    : _on_finish(std::move(other._on_finish)) {}

template <typename T> void Value<T>::setFinishCallback(Callback on_finish) {
  _on_finish = on_finish;
}

template <typename T> void Value<T>::on(TokenType<T> value) {
  _value = value;
  endParsing();
}

template <typename T> void Value<T>::finish() {
  if (_on_finish && !_on_finish(_value)) {
    throw std::runtime_error("Callback returned false");
  }
}

template <typename T> const T &Value<T>::get() const {
  checkSet();
  return _value;
}

template <typename T> T &&Value<T>::pop() {
  checkSet();
  _set = false;
  return std::move(_value);
}
}  // namespace SJParser
