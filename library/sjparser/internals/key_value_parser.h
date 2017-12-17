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

#include "dispatcher.h"
#include "ignore.h"
#include "sjparser/options.h"
#include "token_parser.h"

#include <sstream>
#include <string>
#include <string_view>
#include <unordered_map>

namespace SJParser {

// std::string can be constructed from {"x", "y"}, so this wrapper is used
// instead
class FieldName {
 public:
  FieldName(std::string str);
  FieldName(std::string_view str);
  FieldName(const char *str);
  operator const std::string &() const;
  bool operator==(const FieldName &other) const;
  const std::string &str() const;

 private:
  std::string _str;
};

template <typename TypeFieldT, typename... Ts>
class KeyValueParser : public TokenParser {
 public:
  template <typename T> struct FieldArgs {
    using Args = typename T::Args;

    FieldArgs(const TypeFieldT &field, const Args &value = {});
    template <typename U = T>
    FieldArgs(const TypeFieldT &field, const typename U::ChildArgs &value);
    template <typename U = TypeFieldT>
    FieldArgs(
        const char *field,
        typename std::enable_if_t<std::is_same_v<U, FieldName>> * /*unused*/
        = 0);
    template <typename U = TypeFieldT>
    FieldArgs(
        const std::string &field,
        typename std::enable_if_t<std::is_same_v<U, FieldName>> * /*unused*/
        = 0);

    TypeFieldT field;
    Args value;
  };
  using ChildArgs = std::tuple<FieldArgs<Ts>...>;

  using Options = ObjectOptions;

  KeyValueParser(const ChildArgs &args, const Options &options = {});

  void setDispatcher(Dispatcher *dispatcher) noexcept override;
  void reset() override;

  void on(MapStartT /*unused*/) override;
  void on(MapEndT /*unused*/) override;

  void onField(const TypeFieldT &field);

  template <size_t n, typename T, typename... TDs> struct NthTypes {
   private:
    using NextLevel = NthTypes<n - 1, TDs...>;

   public:
    using ParserType = typename NextLevel::ParserType;

    template <typename U = NextLevel>
    using ValueType = typename U::template ValueType<>;

    enum { has_value_type = NextLevel::has_value_type };
  };

  template <typename T, typename... TDs> struct NthTypes<0, T, TDs...> {
    using ParserType = T;

    template <typename U = T> using ValueType = typename U::Type;

   private:
    using HasValueType = uint8_t;
    using NoValueType = uint32_t;
    template <typename U>
    static HasValueType valueTypeTest(typename U::Type * /*unused*/);
    template <typename U> static NoValueType valueTypeTest(...);

   public:
    enum {
      has_value_type = sizeof(valueTypeTest<T>(nullptr)) == sizeof(HasValueType)
    };
  };

  // Returns NthTypes<n, Ts...>::template ValueType<> if it is available,
  // otherwise NthTypes<n, Ts...>::ParserType
  template <size_t n> auto &get();

  template <size_t n> typename NthTypes<n, Ts...>::ParserType &parser();

  template <size_t n> typename NthTypes<n, Ts...>::template ValueType<> &&pop();

 protected:
  template <size_t, typename Args, typename...> struct Field {
    Field(std::array<TokenParser *, sizeof...(Ts)> & /*fields_array*/,
          std::unordered_map<TypeFieldT, TokenParser *> & /*fields_map*/,
          const Args & /*args*/) {}
  };

  template <size_t n, typename Args, typename T, typename... TDs>
  struct Field<n, Args, T, TDs...> : private Field<n + 1, Args, TDs...> {
    Field(std::array<TokenParser *, sizeof...(Ts)> &fields_array,
          std::unordered_map<TypeFieldT, TokenParser *> &fields_map,
          const Args &args);

    T _field;
  };

  std::array<TokenParser *, sizeof...(Ts)> _fields_array;
  std::unordered_map<TypeFieldT, TokenParser *> _fields_map;
  Field<0, ChildArgs, Ts...> _fields;
  Ignore _ignore_parser;
  Options _options;
};
}  // namespace SJParser

#include "impl/key_value_parser.h"
