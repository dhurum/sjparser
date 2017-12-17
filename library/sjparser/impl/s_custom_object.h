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

template <typename T, typename... Ts>
SCustomObject<T, Ts...>::Args::Args(
    const ChildArgs &args, const Options &options,
    const std::function<bool(SCustomObject<T, Ts...> &, T &)> &on_finish)
    : args(args), options(options), on_finish(on_finish) {}

template <typename T, typename... Ts>
SCustomObject<T, Ts...>::Args::Args(
    const ChildArgs &args,
    const std::function<bool(SCustomObject<T, Ts...> &, T &)> &on_finish)
    : args(args), on_finish(on_finish) {}

template <typename T, typename... Ts>
SCustomObject<T, Ts...>::SCustomObject(const Args &args)
    : Object<Ts...>({args.args, args.options}), _on_finish(args.on_finish) {}

template <typename T, typename... Ts>
const typename SCustomObject<T, Ts...>::Type &SCustomObject<T, Ts...>::get()
    const {
  checkSet();
  return _value;
}

template <typename T, typename... Ts>
typename SCustomObject<T, Ts...>::Type &&SCustomObject<T, Ts...>::pop() {
  checkSet();
  _set = false;
  return std::move(_value);
}

template <typename T, typename... Ts> void SCustomObject<T, Ts...>::finish() {
  if (!_on_finish(*this, _value)) {
    throw std::runtime_error("Callback returned false");
  }
}

template <typename T, typename... Ts> void SCustomObject<T, Ts...>::reset() {
  Object<Ts...>::KVParser::reset();
  _value = Type();
}
}  // namespace SJParser
