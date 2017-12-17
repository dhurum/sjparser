
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
Map<T>::Args::Args(const ChildArgs &args,
                   const std::function<bool(const std::string &, T &)> &on_key,
                   const std::function<bool(Map<T> &)> &on_finish)
    : args(args), on_key(on_key), on_finish(on_finish) {}

template <typename T>
Map<T>::Args::Args(const ChildArgs &args,
                   const std::function<bool(Map<T> &)> &on_finish)
    : args(args), on_key(nullptr), on_finish(on_finish) {}

template <typename T>
Map<T>::Args::Args(const std::function<bool(const std::string &, T &)> &on_key,
                   const std::function<bool(Map<T> &)> &on_finish)
    : args({}), on_key(on_key), on_finish(on_finish) {}

template <typename T>
Map<T>::Args::Args(const std::function<bool(Map<T> &)> &on_finish)
    : args({}), on_key(nullptr), on_finish(on_finish) {}

template <typename T>
template <typename U>
Map<T>::Args::Args(const GrandChildArgs<U> &args,
                   const std::function<bool(const std::string &, T &)> &on_key,
                   const std::function<bool(Map<T> &)> &on_finish)
    : args(args), on_key(on_key), on_finish(on_finish) {}

template <typename T>
template <typename U>
Map<T>::Args::Args(const GrandChildArgs<U> &args,
                   const std::function<bool(Map<T> &)> &on_finish)
    : args(args), on_key(nullptr), on_finish(on_finish) {}

template <typename T>
Map<T>::Map(const Args &args)
    : _parser(args.args), _on_key(args.on_key), _on_finish(args.on_finish) {}

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
  if (_on_key && !_on_key(_current_key, _parser)) {
    throw std::runtime_error("Key callback returned false");
  }
}

template <typename T> void Map<T>::finish() {
  if (_on_finish && !_on_finish(*this)) {
    throw std::runtime_error("Callback returned false");
  }
}
}  // namespace SJParser
