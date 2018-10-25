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

#include "internals/array_parser.h"

#include <functional>

namespace SJParser {

/** @brief %Array parser.
 *
 * @tparam T Elements parser type.
 */

template <typename T> class Array : public ArrayParser {
 public:
  /** Elements parser type. */
  using ParserType = std::decay_t<T>;

  /** Finish callback type. */
  using Callback = std::function<bool(Array<T> &)>;

  /** @brief Constructor.
   *
   * @param [in] parser %Parser for array elements, can be an lvalue reference
   * or an rvalue.
   *
   * @param [in] on_finish (optional) Callback, that will be called after the
   * array is parsed.
   * The callback will be called with a reference to the parser as an argument.
   * If the callback returns false, parsing will be stopped with an error.
   */
  template <typename CallbackT = std::nullptr_t>
  Array(T &&parser, CallbackT on_finish = nullptr);

  /** Move constructor. */
  Array(Array &&other) noexcept;

  /** @brief Finish callback setter.
   *
   * @param [in] on_finish Callback, that will be called after the
   * array is parsed.
   * The callback will be called with a reference to the parser as an argument.
   * If the callback returns false, parsing will be stopped with an error.
   */
  void setFinishCallback(Callback on_finish);

#ifdef DOXYGEN_ONLY
  /** @brief Check if the parser has a value.
   *
   * @return True if the parser parsed something or false otherwise.
   */
  bool isSet();

  /** @brief Check if the parsed array was empy (null).
   *
   * @return True if the parsed array was empty (null) or false otherwise.
   */
  bool isEmpty();
#endif

  /** @brief Elements parser getter.
   *
   * @return Reference to the elements parser.
   */
  T &parser();

 protected:
  /** @cond INTERNAL Elements parser. */
  T _parser;
  /** @endcond */

 private:
  void finish() override;

  Callback _on_finish;
};

template <typename T> Array(Array<T> &&)->Array<Array<T>>;

template <typename T> Array(Array<T> &)->Array<Array<T> &>;

template <typename T> Array(T &&)->Array<T>;
}  // namespace SJParser

#include "impl/array.h"
