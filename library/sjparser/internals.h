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

#include <deque>
#include <functional>
#include <list>
#include <memory>
#include <sstream>
#include <string>
#include <string_view>
#include <unordered_map>
#include "options.h"

namespace SJParser {

struct NullT {};
struct MapStartT {};
struct MapKeyT {
  std::string_view key;
};
struct MapEndT {};
struct ArrayStartT {};
struct ArrayEndT {};

template <typename T> struct TokenTypeResolver { using Type = T; };

template <> struct TokenTypeResolver<std::string> {
  using Type = std::string_view;
};

template <typename T> using TokenType = typename TokenTypeResolver<T>::Type;

class Dispatcher;

class TokenParser {
 public:
  virtual void setDispatcher(Dispatcher *dispatcher) noexcept;
  inline bool isSet() const noexcept;
  virtual void reset();
  void endParsing();
  virtual void finish() = 0;

  virtual void on(NullT /*unused*/);
  virtual void on(bool /*value*/);
  virtual void on(int64_t /*value*/);
  virtual void on(double /*value*/);
  virtual void on(std::string_view /*value*/);
  virtual void on(MapStartT /*unused*/);
  virtual void on(MapKeyT /*key*/);
  virtual void on(MapEndT /*unused*/);
  virtual void on(ArrayStartT /*unused*/);
  virtual void on(ArrayEndT /*unused*/);

  virtual void childParsed();

  virtual ~TokenParser() = default;

 protected:
  Dispatcher *_dispatcher = nullptr;
  bool _set = false;

  inline void checkSet() const;

  inline void unexpectedToken(const std::string &type);
};

class Ignore : public TokenParser {
 public:
  void reset() override;

  // Protected because we need to inherit from this class in order to unit test
  // it
 protected:
  enum class Structure { Object, Array };
  std::list<Structure> _structure;

  void onValue();
  void on(bool /*value*/) override;
  void on(int64_t /*value*/) override;
  void on(double /*value*/) override;
  void on(std::string_view /*value*/) override;
  void on(MapStartT /*unused*/) override;
  void on(MapKeyT /*key*/) override;
  void on(MapEndT /*unused*/) override;
  void on(ArrayStartT /*unused*/) override;
  void on(ArrayEndT /*unused*/) override;
  void finish() override;
};

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

class ArrayParser : public TokenParser {
 public:
  void reset() override;

  void on(NullT /*unused*/) override;
  void on(bool value) override;
  void on(int64_t value) override;
  void on(double value) override;
  void on(std::string_view value) override;
  void on(MapStartT /*unused*/) override;
  void on(ArrayStartT /*unused*/) override;
  void on(ArrayEndT /*unused*/) override;

 protected:
  TokenParser *_parser_ptr;

 private:
  bool _started = false;
};

class Dispatcher {
 public:
  Dispatcher(TokenParser *parser);
  void pushParser(TokenParser *parser);
  void popParser();
  bool emptyParsersStack();
  void reset();

  template <typename T> void on(T value);

 protected:
  std::deque<TokenParser *> _parsers;
  TokenParser *_root_parser = nullptr;
  std::function<void()> _on_completion;
  std::string _error;
};
}  // namespace SJParser

#include "internals_impl.h"
