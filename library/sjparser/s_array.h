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

#include "array.h"

#include <vector>

namespace SJParser {

/** @brief %Array parser, that stores the result in an std::vector of
 * @ref SArray_T "T" values.
 *
 * @tparam T Underlying parser type.
 * @anchor SArray_T
 */

template <typename T> class SArray : public Array<T> {
 public:
  /** Underlying parser type */
  using ParserType = std::decay_t<T>;

  /** Stored value type */
  using Type = std::vector<typename ParserType::Type>;

  /** Finish callback type */
  using Callback = std::function<bool(const Type &)>;

  /** @brief Constructor.
   *
   * @param [in] parser %Parser for array elements, can be an lvalue reference
   * or an rvalue. It must be one of the parsers that store values (Value,
   * SArray, SAutoObject, SCustomObject, SUnion).
   *
   * @param [in] on_finish (optional) Callback, that will be called after the
   * array is parsed.
   * The callback will be called with a reference to the parser as an argument.
   * If the callback returns false, parsing will be stopped with an error.
   */
  template <typename CallbackT = std::nullptr_t>
  SArray(T &&parser, CallbackT on_finish = nullptr);

  /** Move constructor. */
  SArray(SArray &&other) noexcept;

  /** @brief Finish callback setter.
   *
   * @param [in] on_finish Callback, that will be called after the
   * array is parsed.
   *
   * The callback will be called with a reference to the parser as an argument.
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

  /** @brief Check if the parsed array was empy (null).
   *
   * @return True if the parsed array was empty (null) or false otherwise.
   */
  bool isEmpty();

  T &parser();
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
   * @throw std::runtime_error Thrown if the value is unset (no value was
   * parsed or #pop was called).
   */
  Type &&pop();

 private:
  using TokenParser::checkSet;

  void childParsed() override;
  void finish() override;
  void reset() override;

  Type _values;
  Callback _on_finish;
};

template <template <typename> typename U, typename T>
SArray(U<T> &&)->SArray<U<T>>;

template <template <typename> typename U, typename T>
SArray(U<T> &)->SArray<U<T> &>;

template <typename T> SArray(T &&)->SArray<T>;
}  // namespace SJParser

#include "impl/s_array.h"
