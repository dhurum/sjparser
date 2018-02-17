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

template <typename NameType, typename... Ts>
KeyValueParser<NameType, Ts...>::KeyValueParser(
    std::tuple<Member<NameType, Ts>...> members, ObjectOptions options)
    : _member_parsers(_parsers_array, _parsers_map, members),
      _options(options) {}

template <typename NameType, typename... Ts>
KeyValueParser<NameType, Ts...>::KeyValueParser(KeyValueParser &&other) noexcept
    : _member_parsers(_parsers_array, _parsers_map,
                      std::move(other._member_parsers)),
      _options(other._options) {}

template <typename NameType, typename... Ts>
void KeyValueParser<NameType, Ts...>::setDispatcher(
    Dispatcher *dispatcher) noexcept {
  TokenParser::setDispatcher(dispatcher);
  for (auto &member : _parsers_map) {
    member.second->setDispatcher(dispatcher);
  }

  _ignore_parser.setDispatcher(dispatcher);
}

template <typename NameType, typename... Ts>
void KeyValueParser<NameType, Ts...>::reset() {
  TokenParser::reset();

  for (auto &member : _parsers_map) {
    member.second->reset();
  }

  _ignore_parser.reset();
}

template <typename NameType, typename... Ts>
void KeyValueParser<NameType, Ts...>::on(MapStartT /*unused*/) {
  reset();
}

template <typename NameType, typename... Ts>
void KeyValueParser<NameType, Ts...>::on(MapEndT /*unused*/) {
  endParsing();
}

template <typename NameType, typename... Ts>
void KeyValueParser<NameType, Ts...>::onMember(TokenType<NameType> member) {
  if (auto parser_it = _parsers_map.find(member);
      parser_it != _parsers_map.end()) {
    _dispatcher->pushParser(parser_it->second);
    return;
  }

  if (_options.unknown_member == Reaction::Error) {
    std::stringstream error;
    error << "Unexpected member " << member;
    throw std::runtime_error(error.str());
  }

  _dispatcher->pushParser(&_ignore_parser);
}

template <typename NameType, typename... Ts>
template <size_t n>
auto &KeyValueParser<NameType, Ts...>::get() {
  const auto parser =
      reinterpret_cast<typename NthTypes<n, Ts...>::ParserType *>(
          _parsers_array[n]);

  if constexpr (NthTypes<n, Ts...>::has_value_type) {
    return parser->get();
  } else {  // NOLINT
    return *parser;
  }
}

template <typename NameType, typename... Ts>
template <size_t n>
typename KeyValueParser<NameType, Ts...>::template NthTypes<n,
                                                            Ts...>::ParserType &
KeyValueParser<NameType, Ts...>::parser() {
  return *reinterpret_cast<typename NthTypes<n, Ts...>::ParserType *>(
      _parsers_array[n]);
}

template <typename NameType, typename... Ts>
template <size_t n>
typename KeyValueParser<NameType, Ts...>::template NthTypes<
    n, Ts...>::template ValueType<>
    &&KeyValueParser<NameType, Ts...>::pop() {
  return reinterpret_cast<typename NthTypes<n, Ts...>::ParserType *>(
             _parsers_array[n])
      ->pop();
}

template <typename NameType, typename... Ts>
template <size_t n, typename T, typename... TDs>
KeyValueParser<NameType, Ts...>::MemberParser<n, T, TDs...>::MemberParser(
    std::array<TokenParser *, sizeof...(Ts)> &parsers_array,
    std::unordered_map<InternalNameType, TokenParser *> &parsers_map,
    std::tuple<Member<NameType, Ts>...> &members)
    : MemberParser<n + 1, TDs...>(parsers_array, parsers_map, members),
      parser(std::forward<T>(std::get<n>(members).parser)),
      name(std::move(std::get<n>(members).name)) {
  parsers_array[n] = &parser;

  auto[_, inserted] = parsers_map.insert({name, &parser});
  std::ignore = _;
  if (!inserted) {
    std::stringstream error;
    error << "Member " << name << " appears more, than once";
    throw std::runtime_error(error.str());
  }
}

template <typename NameType, typename... Ts>
template <size_t n, typename T, typename... TDs>
KeyValueParser<NameType, Ts...>::MemberParser<n, T, TDs...>::MemberParser(
    std::array<TokenParser *, sizeof...(Ts)> &parsers_array,
    std::unordered_map<InternalNameType, TokenParser *> &parsers_map,
    MemberParser &&other)
    : MemberParser<n + 1, TDs...>(parsers_array, parsers_map, std::move(other)),
      parser(std::forward<T>(other.parser)),
      name(std::move(other.name)) {
  parsers_array[n] = &parser;
  parsers_map[name] = &parser;
}
}  // namespace SJParser
