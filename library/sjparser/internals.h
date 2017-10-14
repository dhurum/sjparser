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
#include <memory>
#include <sstream>
#include <string>
#include <unordered_map>

namespace SJParser {

struct NullT {};
struct MapStartT {};
struct MapKeyT {
  // This ref is used only to forward the key into the 'on' method.
  const std::string &key;  // NOLINT
};
struct MapEndT {};
struct ArrayStartT {};
struct ArrayEndT {};

class Dispatcher;

class TokenParser {
 public:
  virtual void setDispatcher(Dispatcher *dispatcher) noexcept;
  inline bool isSet() const noexcept;
  virtual void reset();
  void endParsing();
  virtual void finish() = 0;

  virtual void on(NullT /*unused*/);
  virtual void on(const bool & /*value*/);
  virtual void on(const int64_t & /*value*/);
  virtual void on(const double & /*value*/);
  virtual void on(const std::string & /*value*/);
  virtual void on(MapStartT /*unused*/);
  virtual void on(const MapKeyT & /*key*/);
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

// std::string can be constructed from {"x", "y"}, so this wrapper is used
// instead
class FieldName {
 public:
  FieldName(std::string str);
  FieldName(const char *str);
  operator const std::string &() const;
  bool operator==(const FieldName &other) const;
  const std::string &str() const;

 private:
  std::string _str;
};

template <typename I, typename... Ts>
class KeyValueParser : public TokenParser {
 public:
  template <typename T> struct FieldArgs {
    using Args = typename T::Args;

    FieldArgs(const I &field, const Args &value);
    template <typename U = T>
    FieldArgs(const I &field, const typename U::ChildArgs &value);
    FieldArgs(const I &field);
    template <typename U = I>
    FieldArgs(const char *field,
              typename std::enable_if<std::is_same<U, FieldName>::value>::type
                  * /*unused*/
              = 0);
    template <typename U = I>
    FieldArgs(const std::string &field,
              typename std::enable_if<std::is_same<U, FieldName>::value>::type
                  * /*unused*/
              = 0);

    I field;
    Args value;
  };
  using ChildArgs = std::tuple<FieldArgs<Ts>...>;

  KeyValueParser(const ChildArgs &args);

  void setDispatcher(Dispatcher *dispatcher) noexcept override;
  void reset() override;

  void on(MapStartT /*unused*/) override;
  void on(MapEndT /*unused*/) override;

  void onField(const I &field);

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
          std::unordered_map<I, TokenParser *> & /*fields_map*/,
          const Args & /*args*/) {}
  };

  template <size_t n, typename Args, typename T, typename... TDs>
  struct Field<n, Args, T, TDs...> : private Field<n + 1, Args, TDs...> {
    Field(std::array<TokenParser *, sizeof...(Ts)> &fields_array,
          std::unordered_map<I, TokenParser *> &fields_map, const Args &args);

    T _field;
  };

  std::array<TokenParser *, sizeof...(Ts)> _fields_array;
  std::unordered_map<I, TokenParser *> _fields_map;
  Field<0, ChildArgs, Ts...> _fields;
};

class ArrayParser : public TokenParser {
 public:
  void reset() override;

  void on(NullT /*unused*/) override;
  void on(const bool &value) override;
  void on(const int64_t &value) override;
  void on(const double &value) override;
  void on(const std::string &value) override;
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

  template <typename T> void on(const T &value);

 protected:
  std::deque<TokenParser *> _parsers;
  TokenParser *_root_parser = nullptr;
  std::function<void()> _on_completion;
  std::string _error;
};
}  // namespace SJParser

#include "internals_impl.h"
