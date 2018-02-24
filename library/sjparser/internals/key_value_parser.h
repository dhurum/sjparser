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
}  // namespace SJParser

#include "impl/key_value_parser.h"
