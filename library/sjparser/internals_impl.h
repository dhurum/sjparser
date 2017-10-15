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

bool TokenParser::isSet() const noexcept {
  return _set;
}

void TokenParser::checkSet() const {
  if (!isSet()) {
    throw std::runtime_error("Can't get value, parser is unset");
  }
}

void TokenParser::unexpectedToken(const std::string &type) {
  throw std::runtime_error("Unexpected token " + type);
}

template <typename FieldIDType, typename... Ts>
template <typename T>
KeyValueParser<FieldIDType, Ts...>::FieldArgs<T>::FieldArgs(
    const FieldIDType &field, const Args &value)
    : field(field), value(value) {}

template <typename FieldIDType, typename... Ts>
template <typename T>
template <typename U>
KeyValueParser<FieldIDType, Ts...>::FieldArgs<T>::FieldArgs(
    const FieldIDType &field, const typename U::ChildArgs &value)
    : field(field), value(value) {}

template <typename FieldIDType, typename... Ts>
template <typename T>
KeyValueParser<FieldIDType, Ts...>::FieldArgs<T>::FieldArgs(
    const FieldIDType &field)
    : field(field) {}

template <typename FieldIDType, typename... Ts>
template <typename T>
template <typename U>
KeyValueParser<FieldIDType, Ts...>::FieldArgs<T>::FieldArgs(
    const char *field,
    typename std::enable_if_t<std::is_same_v<U, FieldName>> * /*unused*/)
    : field(field) {}

template <typename FieldIDType, typename... Ts>
template <typename T>
template <typename U>
KeyValueParser<FieldIDType, Ts...>::FieldArgs<T>::FieldArgs(
    const std::string &field,
    typename std::enable_if_t<std::is_same_v<U, FieldName>> * /*unused*/)
    : field(field) {}

template <typename FieldIDType, typename... Ts>
KeyValueParser<FieldIDType, Ts...>::KeyValueParser(const ChildArgs &args)
    : _fields(_fields_array, _fields_map, args) {}

template <typename FieldIDType, typename... Ts>
void KeyValueParser<FieldIDType, Ts...>::setDispatcher(
    Dispatcher *dispatcher) noexcept {
  TokenParser::setDispatcher(dispatcher);
  for (auto &field : _fields_map) {
    field.second->setDispatcher(dispatcher);
  }
}

template <typename FieldIDType, typename... Ts>
void KeyValueParser<FieldIDType, Ts...>::reset() {
  TokenParser::reset();

  for (auto &field : _fields_map) {
    field.second->reset();
  }
}

template <typename FieldIDType, typename... Ts>
void KeyValueParser<FieldIDType, Ts...>::on(MapStartT /*unused*/) {
  reset();
}

template <typename FieldIDType, typename... Ts>
void KeyValueParser<FieldIDType, Ts...>::on(MapEndT /*unused*/) {
  endParsing();
}

template <typename FieldIDType, typename... Ts>
void KeyValueParser<FieldIDType, Ts...>::onField(const FieldIDType &field) {
  try {
    auto &parser = _fields_map.at(field);
    _dispatcher->pushParser(parser);
  } catch (...) {
    std::stringstream error;
    error << "Unexpected field " << field;
    throw std::runtime_error(error.str());
  }
}

template <typename FieldIDType, typename... Ts>
template <size_t n>
auto &KeyValueParser<FieldIDType, Ts...>::get() {
  const auto parser =
      reinterpret_cast<typename NthTypes<n, Ts...>::ParserType *>(
          _fields_array[n]);

  if constexpr (NthTypes<n, Ts...>::has_value_type) {
    return parser->get();
  } else {  // NOLINT
    return *parser;
  }
}

template <typename FieldIDType, typename... Ts>
template <size_t n>
typename KeyValueParser<FieldIDType,
                        Ts...>::template NthTypes<n, Ts...>::ParserType &
KeyValueParser<FieldIDType, Ts...>::parser() {
  return *reinterpret_cast<typename NthTypes<n, Ts...>::ParserType *>(
      _fields_array[n]);
}

template <typename FieldIDType, typename... Ts>
template <size_t n>
typename KeyValueParser<FieldIDType, Ts...>::template NthTypes<
    n, Ts...>::template ValueType<>
    &&KeyValueParser<FieldIDType, Ts...>::pop() {
  return reinterpret_cast<typename NthTypes<n, Ts...>::ParserType *>(
             _fields_array[n])
      ->pop();
}

template <typename FieldIDType, typename... Ts>
template <size_t n, typename Args, typename T, typename... TDs>
KeyValueParser<FieldIDType, Ts...>::Field<n, Args, T, TDs...>::Field(
    std::array<TokenParser *, sizeof...(Ts)> &fields_array,
    std::unordered_map<FieldIDType, TokenParser *> &fields_map,
    const Args &args)
    : Field<n + 1, Args, TDs...>(fields_array, fields_map, args),
      _field(std::get<n>(args).value) {
  fields_array[n] = &_field;
  fields_map[std::get<n>(args).field] = &_field;
}

template <typename T> void Dispatcher::on(const T &value) {
  if (_parsers.empty()) {
    throw std::runtime_error("Parsers stack is empty");
  }
  _parsers.back()->on(value);
}

std::basic_ostream<char> &operator<<(std::basic_ostream<char> &stream,
                                     const FieldName &name);
}  // namespace SJParser

namespace std {
template <> struct hash<SJParser::FieldName> {
  std::size_t operator()(const SJParser::FieldName &key) const {
    return hash<std::string>()(key.str());
  }
};
}  // namespace std
