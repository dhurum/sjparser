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

#include "internals/token_parser.h"

#include <functional>

namespace SJParser {

/** @brief Plain value parser.
 *
 * @tparam T JSON value type, can be std::string, int64_t, bool or double */

template <typename T> class Value : public TokenParser {
 public:
  /** Underlying type, that can be obtained from this parser with #get or #pop.
   */
  using Type = T;

  /** Finish callback type. */
  using Callback = std::function<bool(const Type &)>;

  /** @brief Constructor.
   *
   * @param [in] on_finish (optional) Callback, that will be called after a
   * value is parsed.
   * The callback will be called with a const reference to a parsed value as an
   * argument.
   * If the callback returns false, parsing will be stopped with an error.
   */
  Value(Callback on_finish = nullptr);

  /** Move constructor. */
  Value(Value &&other) noexcept;

  /** @brief Finish callback setter.
   *
   * @param [in] on_finish Callback, that will be called after a
   * value is parsed.
   * The callback will be called with a const reference to a parsed value as an
   *
   * If the callback returns false, parsing will be stopped with an error.
   */
  void setFinishCallback(Callback on_finish);

#ifdef DOXYGEN_ONLY
  /** @brief Check if the parser has a value.
   *
   * @return True if the parser has some value stored or false otherwise.
   */
  bool isSet();

  /** @brief Check if the parsed value was empy (null).
   *
   * @return True if the parsed value was empty (null) or false otherwise.
   */
  bool isEmpty();
#endif

  /** @brief Parsed value getter.
   *
   * @return Const reference to a parsed value.
   *
   * @throw std::runtime_error Thrown if the value is unset (no value was
   * parsed or #pop was called).
   */
  const Type &get() const;

  /** @brief Get the parsed value and unset the parser.
   *
   * Moves the parsed value out of the parser.
   *
   * @return Rvalue reference to the parsed value.
   *
   * @throw std::runtime_error thrown if the value is unset (no value was
   * parsed or #pop was called).
   */
  Type &&pop();

 private:
  void on(TokenType<Type> value) override;
  void finish() override;

  Type _value;
  Callback _on_finish;
};

/****************************** Implementations *******************************/

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
  TokenParser::_empty = false;
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
