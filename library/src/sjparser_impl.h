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

struct MapStartT {};
struct MapKeyT {
  const std::string &key;
};
struct MapEndT {};
struct ArrayStartT {};
struct ArrayEndT {};

class Dispatcher;

class TokenParser {
 public:
  virtual void setDispatcher(Dispatcher *dispatcher) noexcept;
  inline bool isSet() const noexcept;
  virtual void reset() noexcept;
  bool endParsing();
  virtual bool finish() = 0;

  virtual bool on(const bool & /*value*/);
  virtual bool on(const int64_t & /*value*/);
  virtual bool on(const double & /*value*/);
  virtual bool on(const std::string & /*value*/);
  virtual bool on(const MapStartT);
  virtual bool on(const MapKeyT & /*key*/);
  virtual bool on(const MapEndT);
  virtual bool on(const ArrayStartT);
  virtual bool on(const ArrayEndT);

  virtual bool childParsed();

  virtual ~TokenParser() = default;

 protected:
  Dispatcher *_dispatcher = nullptr;
  bool _set = false;

  inline void checkSet() const;

  inline bool unexpectedToken(const std::string &type);
};

// std::string can be constructed from {"x", "y"}, so this wrapper is used
// instead
class FieldName {
 public:
  FieldName(const std::string &str);
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
    FieldArgs(
        const char *field,
        typename std::enable_if<std::is_same<U, FieldName>::value>::type * = 0);
    template <typename U = I>
    FieldArgs(
        const std::string &field,
        typename std::enable_if<std::is_same<U, FieldName>::value>::type * = 0);

    I field;
    Args value;
  };

  virtual void setDispatcher(Dispatcher *dispatcher) noexcept override;
  virtual void reset() noexcept override;

  virtual bool on(const MapStartT) noexcept override;
  virtual bool on(const MapEndT) override;

  bool onField(const I &field);

  template <size_t n, typename T, typename... TDs> struct NthType {
    using type = typename NthType<n - 1, TDs...>::type;
  };

  template <typename T, typename... TDs> struct NthType<0, T, TDs...> {
    using type = T;
  };

  template <size_t n> inline typename NthType<n, Ts...>::type &get();

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
};

class ArrayParser : public TokenParser {
 public:
  ArrayParser(std::function<bool()> on_finish) : _on_finish(on_finish) {}
  virtual void reset() noexcept override;

  virtual bool on(const bool &value) override;
  virtual bool on(const int64_t &value) override;
  virtual bool on(const double &value) override;
  virtual bool on(const std::string &value) override;
  virtual bool on(const MapStartT) override;
  virtual bool on(const ArrayStartT) override;
  virtual bool on(const ArrayEndT) override;

  virtual bool finish() override;

 protected:
  TokenParser *_parser;
  std::function<bool()> _on_finish;

 private:
  bool _started = false;
};

class Dispatcher {
 public:
  Dispatcher(TokenParser *parser);
  void pushParser(TokenParser *parser);
  bool popParser();
  bool noParser();
  void reset();

  template <typename T> bool on(const T &value);

  inline void setError(const std::string &error);
  inline std::string &getError() noexcept;

 protected:
  std::deque<TokenParser *> _parsers;
  TokenParser *_root_parser = nullptr;
  std::function<void()> _on_completion;
  std::string _error;
};

class YajlInfo;

class ParserImpl {
 public:
  ParserImpl(TokenParser *parser);
  ~ParserImpl();
  bool parse(const char *data, size_t len);
  bool finish();
  std::string getError(bool verbose);

 private:
  void collectErrors();

  Dispatcher _dispatcher;
  std::unique_ptr<YajlInfo> _yajl_info;
  const unsigned char *_data;
  size_t _len;
  std::string _internal_error;
  std::string _yajl_error;
};

/******************************** Definitions ********************************/

bool TokenParser::isSet() const noexcept {
  return _set;
}

void TokenParser::checkSet() const {
  if (!isSet()) {
    throw std::runtime_error("Can't get value, parser is unset");
  }
}

bool TokenParser::unexpectedToken(const std::string &type) {
  if (_dispatcher) {
    _dispatcher->setError("Unexpected token " + type);
  }
  return false;
}

template <typename I, typename... Ts>
template <typename T>
KeyValueParser<I, Ts...>::FieldArgs<T>::FieldArgs(const I &field,
                                                  const Args &value)
    : field(field), value(value) {}

template <typename I, typename... Ts>
template <typename T>
template <typename U>
KeyValueParser<I, Ts...>::FieldArgs<T>::FieldArgs(
    const I &field, const typename U::ChildArgs &value)
    : field(field), value(value) {}

template <typename I, typename... Ts>
template <typename T>
KeyValueParser<I, Ts...>::FieldArgs<T>::FieldArgs(const I &field)
    : field(field) {}

template <typename I, typename... Ts>
template <typename T>
template <typename U>
KeyValueParser<I, Ts...>::FieldArgs<T>::FieldArgs(
    const char *field,
    typename std::enable_if<std::is_same<U, FieldName>::value>::type *)
    : field(field) {}

template <typename I, typename... Ts>
template <typename T>
template <typename U>
KeyValueParser<I, Ts...>::FieldArgs<T>::FieldArgs(
    const std::string &field,
    typename std::enable_if<std::is_same<U, FieldName>::value>::type *)
    : field(field) {}

template <typename I, typename... Ts>
void KeyValueParser<I, Ts...>::setDispatcher(Dispatcher *dispatcher) noexcept {
  TokenParser::setDispatcher(dispatcher);
  for (auto &field : _fields_map) {
    field.second->setDispatcher(dispatcher);
  }
}

template <typename I, typename... Ts>
void KeyValueParser<I, Ts...>::reset() noexcept {
  TokenParser::reset();

  for (auto &field : _fields_map) {
    field.second->reset();
  }
}

template <typename I, typename... Ts>
bool KeyValueParser<I, Ts...>::on(const MapStartT /*unused*/) noexcept {
  reset();
  return true;
}

template <typename I, typename... Ts>
bool KeyValueParser<I, Ts...>::on(const MapEndT /*unused*/) {
  return endParsing();
}

template <typename I, typename... Ts>
bool KeyValueParser<I, Ts...>::onField(const I &field) {
  try {
    auto &parser = _fields_map.at(field);
    _dispatcher->pushParser(parser);
  } catch (...) {
    std::stringstream error;
    error << "Unexpected field " << field;
    _dispatcher->setError(error.str());
    return false;
  }
  return true;
}

template <typename I, typename... Ts>
template <size_t n>
typename KeyValueParser<I, Ts...>::template NthType<n, Ts...>::type &
KeyValueParser<I, Ts...>::get() {
  return *reinterpret_cast<typename NthType<n, Ts...>::type *>(
      _fields_array[n]);
}

template <typename I, typename... Ts>
template <size_t n, typename Args, typename T, typename... TDs>
KeyValueParser<I, Ts...>::Field<n, Args, T, TDs...>::Field(
    std::array<TokenParser *, sizeof...(Ts)> &fields_array,
    std::unordered_map<I, TokenParser *> &fields_map, const Args &args)
    : Field<n + 1, Args, TDs...>(fields_array, fields_map, args),
      _field(std::get<n>(args).value) {
  fields_array[n] = &_field;
  fields_map[std::get<n>(args).field] = &_field;
}

template <typename T> bool Dispatcher::on(const T &value) {
  if (_parsers.empty()) {
    setError("Parsers stack is empty");
    return false;
  }
  return _parsers.back()->on(value);
}

void Dispatcher::setError(const std::string &error) {
  _error = error;
}

std::string &Dispatcher::getError() noexcept {
  return _error;
}
}

namespace std {
template <> struct hash<SJParser::FieldName> {
  std::size_t operator()(const SJParser::FieldName &key) const;
};

std::basic_ostream<char> &operator<<(std::basic_ostream<char> &stream,
                                     const SJParser::FieldName &str);
}
