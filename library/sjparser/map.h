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

/****************************** Implementations *******************************/

template <typename T>
template <typename CallbackT>
Map<T>::Map(
    T &&parser, CallbackT on_finish,
    std::enable_if_t<std::is_constructible_v<Callback, CallbackT>> * /*unused*/)
    : _parser(std::forward<T>(parser)), _on_finish(std::move(on_finish)) {
  static_assert(std::is_base_of_v<TokenParser, ParserType>,
                "Invalid parser used in Map");
}

template <typename T>
template <typename ElementCallbackT, typename CallbackT>
Map<T>::Map(T &&parser, ElementCallbackT on_element, CallbackT on_finish)
    : _parser(std::forward<T>(parser)),
      _on_element(std::move(on_element)),
      _on_finish(std::move(on_finish)) {
  static_assert(std::is_base_of_v<TokenParser, ParserType>,
                "Invalid parser used in Map");
  static_assert(std::is_constructible_v<ElementCallback, ElementCallbackT>,
                "Invalid element callback type");
  static_assert(std::is_constructible_v<Callback, CallbackT>,
                "Invalid callback type");
}

template <typename T>
Map<T>::Map(Map &&other) noexcept
    : _parser(std::forward<T>(other._parser)),
      _on_element(std::move(other._on_element)),
      _on_finish(std::move(other._on_finish)) {}

template <typename T>
void Map<T>::setElementCallback(ElementCallback on_element) {
  _on_element = on_element;
}

template <typename T> void Map<T>::setFinishCallback(Callback on_finish) {
  _on_finish = on_finish;
}

template <typename T> T &Map<T>::parser() {
  return _parser;
}

template <typename T>
void Map<T>::setDispatcher(Dispatcher *dispatcher) noexcept {
  TokenParser::setDispatcher(dispatcher);
  _parser.setDispatcher(dispatcher);
}

template <typename T> void Map<T>::on(MapStartT /*unused*/) {
  reset();
}

template <typename T> void Map<T>::on(MapKeyT key) {
  TokenParser::_empty = false;
  _dispatcher->pushParser(&_parser);
  _current_key = key.key;
}

template <typename T> void Map<T>::on(MapEndT /*unused*/) {
  endParsing();
}

template <typename T> void Map<T>::childParsed() {
  if (_on_element && !_on_element(_current_key, _parser)) {
    throw std::runtime_error("Element callback returned false");
  }
}

template <typename T> void Map<T>::finish() {
  if (_on_finish && !_on_finish(*this)) {
    throw std::runtime_error("Callback returned false");
  }
}

}  // namespace SJParser
