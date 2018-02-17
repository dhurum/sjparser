
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
