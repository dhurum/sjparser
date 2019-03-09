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

#include "default_value.h"
#include "dispatcher.h"
#include "ignore.h"
#include "sjparser/member.h"
#include "sjparser/options.h"
#include "token_parser.h"
#include "traits.h"

#include <sstream>
#include <string>
#include <string_view>
#include <unordered_map>

namespace SJParser {

template <typename NameType, typename... Ts>
class KeyValueParser : public TokenParser {
 public:
  using InternalNameType = TokenType<NameType>;

  KeyValueParser(std::tuple<Member<NameType, Ts>...> members,
                 ObjectOptions options = {});
  KeyValueParser(KeyValueParser &&other) noexcept;

  void setDispatcher(Dispatcher *dispatcher) noexcept override;
  void reset() override;

  void on(MapStartT /*unused*/) override;
  void on(MapEndT /*unused*/) override;

  void onMember(InternalNameType member);

  template <size_t n, typename T, typename... TDs> struct NthTypes {
   private:
    using NextLevel = NthTypes<n - 1, TDs...>;

   public:
    using ParserType = typename NextLevel::ParserType;

    template <typename U = NextLevel>
    using ValueType = typename U::template ValueType<>;

    static constexpr bool has_value_type = NextLevel::has_value_type;
  };

  template <typename T, typename... TDs> struct NthTypes<0, T, TDs...> {
    using ParserType = std::decay_t<T>;

    template <typename U = ParserType> using ValueType = typename U::Type;

    static constexpr bool has_value_type = IsStorageParser<T>;
  };

  template <size_t n>
  using ParserType = typename NthTypes<n, Ts...>::ParserType;

  // Returns NthTypes<n, Ts...>::template ValueType<> if it is available,
  // otherwise NthTypes<n, Ts...>::ParserType
  template <size_t n> auto &get();

  template <size_t n> typename NthTypes<n, Ts...>::ParserType &parser();

  template <size_t n> typename NthTypes<n, Ts...>::template ValueType<> pop();

 protected:
  template <size_t, typename...> struct MemberParser {
    MemberParser(
        std::array<TokenParser *, sizeof...(Ts)> & /*parsers_array*/,
        std::unordered_map<InternalNameType, TokenParser *> & /*parsers_map*/,
        std::tuple<Member<NameType, Ts>...> & /*members*/) {}

    MemberParser(
        std::array<TokenParser *, sizeof...(Ts)> & /*parsers_array*/,
        std::unordered_map<InternalNameType, TokenParser *> & /*parsers_map*/,
        MemberParser && /*other*/) {}
  };

  template <size_t n, typename T, typename... TDs>
  struct MemberParser<n, T, TDs...> : private MemberParser<n + 1, TDs...> {
    MemberParser(
        std::array<TokenParser *, sizeof...(Ts)> &parsers_array,
        std::unordered_map<InternalNameType, TokenParser *> &parsers_map,
        std::tuple<Member<NameType, Ts>...> &members);

    MemberParser(
        std::array<TokenParser *, sizeof...(Ts)> &parsers_array,
        std::unordered_map<InternalNameType, TokenParser *> &parsers_map,
        MemberParser &&other);

    template <size_t index> auto &get();

    T parser;
    NameType name;
    bool optional;
    DefaultValue<T, IsStorageParser<T>> default_value;
  };

  std::array<TokenParser *, sizeof...(Ts)> _parsers_array;
  std::unordered_map<InternalNameType, TokenParser *> _parsers_map;
  MemberParser<0, Ts...> _member_parsers;
  Ignore _ignore_parser;
  ObjectOptions _options;
};

/****************************** Implementations *******************************/

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
  TokenParser::_empty = false;

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
  auto &member = _member_parsers.template get<n>();

  if constexpr (NthTypes<n, Ts...>::has_value_type) {
    if (!member.parser.isSet() && member.default_value.present) {
      return static_cast<const decltype(member.default_value.value) &>(
          member.default_value.value);
    }
    return member.parser.get();
  } else {  // NOLINT
    return member.parser;
  }
}

template <typename NameType, typename... Ts>
template <size_t n>
typename KeyValueParser<NameType, Ts...>::template NthTypes<n,
                                                            Ts...>::ParserType &
KeyValueParser<NameType, Ts...>::parser() {
  return _member_parsers.template get<n>().parser;
}

template <typename NameType, typename... Ts>
template <size_t n>
typename KeyValueParser<NameType, Ts...>::template NthTypes<
    n, Ts...>::template ValueType<>
KeyValueParser<NameType, Ts...>::pop() {
  auto &member = _member_parsers.template get<n>();

  if (!member.parser.isSet() && member.default_value.present) {
    // This form is used to reduce the number of required methods for
    // SCustomObject stored type. Otherwise a copy constructor would be
    // necessary.
    decltype(member.default_value.value) value;
    value = member.default_value.value;
    return value;
  }
  return std::move(member.parser.pop());
}

template <typename NameType, typename... Ts>
template <size_t n, typename T, typename... TDs>
KeyValueParser<NameType, Ts...>::MemberParser<n, T, TDs...>::MemberParser(
    std::array<TokenParser *, sizeof...(Ts)> &parsers_array,
    std::unordered_map<InternalNameType, TokenParser *> &parsers_map,
    std::tuple<Member<NameType, Ts>...> &members)
    : MemberParser<n + 1, TDs...>(parsers_array, parsers_map, members),
      parser(std::forward<T>(std::get<n>(members).parser)),
      name(std::move(std::get<n>(members).name)),
      optional(std::get<n>(members).optional),
      default_value(std::move(std::get<n>(members).default_value)) {
  parsers_array[n] = &parser;

  auto [_, inserted] = parsers_map.insert({name, &parser});
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
      name(std::move(other.name)),
      optional(other.optional),
      default_value(std::move(other.default_value)) {
  parsers_array[n] = &parser;
  parsers_map[name] = &parser;
}

template <typename NameType, typename... Ts>
template <size_t n, typename T, typename... TDs>
template <size_t index>
auto &KeyValueParser<NameType, Ts...>::MemberParser<n, T, TDs...>::get() {
  if constexpr (index == n) {
    return *this;
  } else {  // NOLINT
    return MemberParser<n + 1, TDs...>::template get<index>();
  }
}

}  // namespace SJParser
