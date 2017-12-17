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
  /** Arguments for the underlying parser */
  using ChildArgs = typename T::Args;

  /** Underlying parser value type */
  using EltType = typename T::Type;

  /** Stored value type */
  using Type = std::vector<EltType>;

  /** Finish callback type */
  using CallbackType = std::function<bool(const Type &)>;

  /** @cond INTERNAL Underlying parser type */
  using ParserType = T;
  /** @endcond */

  /** Child arguments for the underlying type */
  template <typename U = Array<T>>
  using GrandChildArgs = typename U::ParserType::ChildArgs;

  /** @brief Struct with arguments for the SArray @ref SArray() "constructor".
   */
  struct Args {
    /** @param [in] args Sets #args.
     *
     * @param[in] on_finish (optional) Sets #on_finish.
     */
    Args(const ChildArgs &args = {}, const CallbackType &on_finish = nullptr);

    /** @param [in] args Sets #args.
     *
     * @param[in] on_finish (optional) Sets #on_finish.
     */
    template <typename U = Array<T>>
    Args(const GrandChildArgs<U> &args,
         const CallbackType &on_finish = nullptr);

    /**@param[in] on_finish (optional) Sets #on_finish. */
    Args(const CallbackType &on_finish);

    /** Arguments for the underlying parser */
    ChildArgs args;
    /** Callback, that will be called after an object is parsed.
     *
     * The callback will be called with a reference to the parser as an
     * argument.
     * This reference is mostly useless, it's main purpose it to help the
     * compiler.
     *
     * If the callback returns false, parsing will be stopped with an error.
     */
    std::function<bool(const Type &)> on_finish;
  };

  /** @brief Array constructor.
   *
   * @param [in] args Args stucture.
   * If you do not specify @ref Args::on_finish "on_finish" callback, you can
   * pass a @ref Args::args "underlying parser arguments" directly into the
   * constructor.
   * If you do not specify @ref Args::args "underlying parser arguments", you
   * can pass a @ref Args::on_finish "on_finish" callback directly into the
   * constructor.
   */
  SArray(const Args &args);
  SArray(const SArray &) = delete;

#ifdef DOXYGEN_ONLY
  /** @brief Check if the parser has a value.
   *
   * @return True if the parser has some value stored or false otherwise.
   */
  bool isSet();
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
  using TokenParser::_set;
  using TokenParser::checkSet;

  void childParsed() override;
  void finish() override;
  void reset() override;

  std::vector<EltType> _values;
  std::function<bool(const Type &)> _on_finish;
};
}  // namespace SJParser

#include "impl/s_array.h"
