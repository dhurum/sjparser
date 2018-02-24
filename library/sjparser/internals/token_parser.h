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

#include <stdexcept>
#include <string>
#include <string_view>

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
  inline bool isEmpty() const noexcept;
  virtual void reset();
  void endParsing();
  virtual void finish() = 0;

  virtual void on(NullT unused);
  virtual void on(bool value);
  virtual void on(int64_t value);
  virtual void on(double value);
  virtual void on(std::string_view value);
  virtual void on(MapStartT /*unused*/);
  virtual void on(MapKeyT key);
  virtual void on(MapEndT /*unused*/);
  virtual void on(ArrayStartT /*unused*/);
  virtual void on(ArrayEndT /*unused*/);

  virtual void childParsed();

  virtual ~TokenParser() = default;

 protected:
  Dispatcher *_dispatcher = nullptr;
  bool _set = false;
  bool _empty = true;

  inline void checkSet() const;

  inline void unexpectedToken(const std::string &type);
};
}  // namespace SJParser

#include "impl/token_parser.h"
