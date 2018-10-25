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

#include "internals/dispatcher.h"
#include "internals/token_parser.h"

#include <functional>

namespace SJParser {

/** @brief %Map parser.
 *
 * Parses an object where each member value has type @ref Map_T "T", and member
 * name represents map key.
 *
 * @tparam T Element's value parser type.
 * @anchor Map_T
 */

template <typename T> class Map : public TokenParser {
 public:
  /** Element's value parser type. */
  using ParserType = std::decay_t<T>;

  /** Element callback type. */
  using ElementCallback =
      std::function<bool(const std::string &, ParserType &)>;

  /** Finish callback type. */
  using Callback = std::function<bool(Map<T> &)>;

  /** @brief Constructor.
   *
   * @param [in] parser %Parser for map elements values, can be an lvalue
   * reference or an rvalue.
   *
   * @param [in] on_finish (optional) Callback, that will be called after the
   * map is parsed.
   * The callback will be called with a reference to the parser as an argument.
   * If the callback returns false, parsing will be stopped with an error.
   */
  template <typename CallbackT = std::nullptr_t>
  Map(T &&parser, CallbackT on_finish = nullptr,
      std::enable_if_t<std::is_constructible_v<Callback, CallbackT>>
          * /*unused*/
      = 0);

  /** @brief Constructor.
   *
   * @param [in] parser %Parser for map elements values, can be an lvalue
   * reference or an rvalue.
   *
   * @param [in] on_element Callback, that will be called after an element of
   * the map is parsed.
   * The callback will be called with a map key and a reference to the parser as
   * arguments.
   * If the callback returns false, parsing will be stopped with an error.
   *
   * @param [in] on_finish (optional) Callback, that will be called after the
   * array is parsed.
   * The callback will be called with a reference to the parser as an argument.
   * If the callback returns false, parsing will be stopped with an error.
   */
  template <typename ElementCallbackT, typename CallbackT = std::nullptr_t>
  Map(T &&parser, ElementCallbackT on_element, CallbackT on_finish = nullptr);

  /** Move constructor. */
  Map(Map &&other) noexcept;

  /** @brief Element callback setter.
   *
   * @param [in] on_element Callback, that will be called after an element of
   * the map is parsed.
   * The callback will be called with a map key and a reference to the parser as
   * arguments.
   * If the callback returns false, parsing will be stopped with an error.
   * */
  void setElementCallback(ElementCallback on_element);

  /** @brief Finish callback setter.
   *
   * @param [in] on_finish Callback, that will be called after the
   * map is parsed.
   * The callback will be called with a reference to the parser as an argument.
   * If the callback returns false, parsing will be stopped with an error.
   */
  void setFinishCallback(Callback on_finish);

  /** @brief Elements value parser getter.
   *
   * @return Reference to the elements parser.
   */
  T &parser();

  /** @cond INTERNAL Internal */
  void setDispatcher(Dispatcher *dispatcher) noexcept override;
  /** @endcond */

 protected:
  /** @cond INTERNAL Elements parser. */
  T _parser;
  std::string _current_key;
  /** @endcond */

 private:
  void on(MapStartT /*unused*/) override;
  void on(MapKeyT key) override;
  void on(MapEndT /*unused*/) override;

  void childParsed() override;
  void finish() override;

  ElementCallback _on_element;
  Callback _on_finish;
};

template <typename T> Map(Map<T> &&)->Map<Map<T>>;

template <typename T> Map(Map<T> &)->Map<Map<T> &>;

template <typename T> Map(T &&)->Map<T>;
}  // namespace SJParser

#include "impl/map.h"
