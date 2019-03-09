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

#include "map.h"

#include <map>

namespace SJParser {

/** @brief %Map parser, that stores the result in an std::map with std::string
 * as a key and @ref SMap_T "T" as a value.
 *
 * Parses an object where each member value has type @ref SMap_T "T", and member
 * name represents map key.
 *
 * @tparam T Element's value parser type.
 * @anchor SMap_T
 */

template <typename T> class SMap : public Map<T> {
 public:
  /** Element's value parser type. */
  using ParserType = std::decay_t<T>;

  /** Stored value type */
  using Type = std::map<std::string, typename ParserType::Type>;

  /** Element callback type. */
  using ElementCallback =
      std::function<bool(const std::string &, ParserType &)>;

  /** Finish callback type. */
  using Callback = std::function<bool(SMap<T> &)>;

  /** @brief Constructor.
   *
   * @param [in] parser %Parser for map elements values, can be an lvalue
   * reference or an rvalue. It must be one of the parsers that store values
   * (Value, SArray, SAutoObject, SCustomObject, SUnion, SMap).
   *
   * @param [in] on_finish (optional) Callback, that will be called after the
   * map is parsed.
   * The callback will be called with a reference to the parser as an argument.
   * If the callback returns false, parsing will be stopped with an error.
   */
  template <typename CallbackT = std::nullptr_t>
  SMap(T &&parser, CallbackT on_finish = nullptr,
       std::enable_if_t<std::is_constructible_v<Callback, CallbackT>>
           * /*unused*/
       = 0);

  /** @brief Constructor.
   *
   * @param [in] parser %Parser for map elements values, can be an lvalue
   * reference or an rvalue. It must be one of the parsers that store values
   * (Value, SArray, SAutoObject, SCustomObject, SUnion, SMap).
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
  SMap(T &&parser, ElementCallbackT on_element, CallbackT on_finish = nullptr);

  /** Move constructor. */
  SMap(SMap &&other) noexcept;

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

  /** @brief Elements value parser getter.
   *
   * @return Reference to the elements parser.
   */
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
  ElementCallback _on_element;
  Callback _on_finish;
};

template <template <typename> typename U, typename T> SMap(U<T> &&)->SMap<U<T>>;

template <template <typename> typename U, typename T>
SMap(U<T> &)->SMap<U<T> &>;

template <typename T> SMap(T &&)->SMap<T>;

/****************************** Implementations *******************************/

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
