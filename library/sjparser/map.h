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
#include "internals/dispatcher.h"

#include <functional>

namespace SJParser {

/** @brief %Map parser.
 *
 * Parses an object where each field is of type @ref Map_T "T", and field
 * name represents map key.
 *
 * @tparam T Underlying parser type.
 * @anchor Map_T
 */

template <typename T> class Map : public TokenParser {
 public:
  /** Arguments for the underlying parser */
  using ChildArgs = typename T::Args;

  /** @cond INTERNAL Underlying parser type */
  using ParserType = T;
  /** @endcond */

  /** Child arguments for the underlying type */
  template <typename U = Map<T>>
  using GrandChildArgs = typename U::ParserType::ChildArgs;

  /** @brief Struct with arguments for the Map @ref Map() "constructor". */
  struct Args {
    /** @param [in] args (optional) Sets #args.
     *
     * @param[in] on_key (optional) Sets #on_key.
     *
     * @param[in] on_finish (optional) Sets #on_finish.
     */
    Args(const ChildArgs &args = {},
         const std::function<bool(const std::string &, T &)> &on_key = nullptr,
         const std::function<bool(Map<T> &)> &on_finish = nullptr);

    /** @param [in] args Sets #args.
     *
     * @param[in] on_finish (optional) Sets #on_finish.
     */
    Args(const ChildArgs &args, const std::function<bool(Map<T> &)> &on_finish);

    /** @param[in] on_key Sets #on_key.
     *
     * @param[in] on_finish (optional) Sets #on_finish.
     */
    Args(const std::function<bool(const std::string &, T &)> &on_key,
         const std::function<bool(Map<T> &)> &on_finish = nullptr);

    /** @param[in] on_finish Sets #on_finish.
     */
    Args(const std::function<bool(Map<T> &)> &on_finish);

    /** @param [in] args Sets #args.
     *
     * @param[in] on_key (optional) Sets #on_key.
     *
     * @param[in] on_finish (optional) Sets #on_finish.
     */
    template <typename U = Map<T>>
    Args(const GrandChildArgs<U> &args,
         const std::function<bool(const std::string &, T &)> &on_key = nullptr,
         const std::function<bool(Map<T> &)> &on_finish = nullptr);

    /** @param [in] args Sets #args.
     *
     * @param[in] on_finish Sets #on_finish.
     */
    template <typename U = Map<T>>
    Args(const GrandChildArgs<U> &args,
         const std::function<bool(Map<T> &)> &on_finish);

    /** Arguments for the underlying parser */
    ChildArgs args;

    /** Callback, that will be called after each key's value is parsed.
     *
     * The callback will be called with a field name and a reference to the
     * child parser as arguments.
     *
     * If the callback returns false, parsing will be stopped with an error.
     */
    std::function<bool(const std::string &, T &)> on_key;

    /** Callback, that will be called after an object is parsed.
     *
     * The callback will be called with a reference to the parser as an
     * argument.
     * This reference is mostly useless, it's main purpose it to help the
     * compiler.
     *
     * If the callback returns false, parsing will be stopped with an error.
     */
    std::function<bool(Map<T> &)> on_finish;
  };

  /** @brief Map constructor.
   *
   * @param [in] args Args stucture.
   * If you do not specify @ref Args::on_finish "on_finish" and
   * @ref Args::on_key "on_key" callbacks, you can pass a
   * @ref Args::args "underlying parser arguments" directly into the
   * constructor.
   */
  Map(const Args &args);
  Map(const Map &) = delete;

  /** @cond INTERNAL Internal */
  void setDispatcher(Dispatcher *dispatcher) noexcept override;
  /** @endcond */

 private:
  void on(MapStartT /*unused*/) override;
  void on(MapKeyT key) override;
  void on(MapEndT /*unused*/) override;

  void childParsed() override;
  void finish() override;

  T _parser;
  std::string _current_key;
  std::function<bool(const std::string &, T &)> _on_key;
  std::function<bool(Map<T> &)> _on_finish;
};
}  // namespace SJParser

#include "impl/map.h"
