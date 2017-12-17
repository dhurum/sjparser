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

template <typename TypeFieldT, typename... Ts>
SUnion<TypeFieldT, Ts...>::Args::Args(
    const ChildArgs &args, const std::function<bool(const Type &)> &on_finish)
    : args(args), allow_default_value(false), on_finish(on_finish) {}

template <typename TypeFieldT, typename... Ts>
SUnion<TypeFieldT, Ts...>::Args::Args(
    const FieldName &type_field, const ChildArgs &args,
    const std::function<bool(const Type &)> &on_finish)
    : type_field(type_field),
      args(args),
      allow_default_value(false),
      on_finish(on_finish) {}

template <typename TypeFieldT, typename... Ts>
SUnion<TypeFieldT, Ts...>::Args::Args(
    const FieldName &type_field, const ChildArgs &args,
    const Type &default_value,
    const std::function<bool(const Type &)> &on_finish)
    : type_field(type_field),
      args(args),
      default_value(default_value),
      on_finish(on_finish) {}

template <typename TypeFieldT, typename... Ts>
SUnion<TypeFieldT, Ts...>::SUnion(const Args &args)
    : Union<TypeFieldT, Ts...>({args.type_field, args.args}),
      _default_value(args.default_value),
      _allow_default_value(args.allow_default_value),
      _on_finish(args.on_finish) {}

template <typename TypeFieldT, typename... Ts>
const typename SUnion<TypeFieldT, Ts...>::Type &SUnion<TypeFieldT, Ts...>::get()
    const {
  checkSet();
  return _value;
}

template <typename TypeFieldT, typename... Ts>
typename SUnion<TypeFieldT, Ts...>::Type &&SUnion<TypeFieldT, Ts...>::pop() {
  checkSet();
  _set = false;
  return std::move(_value);
}

template <typename TypeFieldT, typename... Ts>
void SUnion<TypeFieldT, Ts...>::finish() {
  try {
    ValueSetter<0, Ts...>(_value, *this);
  } catch (std::exception &e) {
    _set = false;
    throw std::runtime_error(std::string("Can not set value: ") + e.what());
  } catch (...) {
    _set = false;
    throw std::runtime_error("Can not set value: unknown exception");
  }

  if (_on_finish && !_on_finish(_value)) {
    throw std::runtime_error("Callback returned false");
  }
}

template <typename TypeFieldT, typename... Ts>
void SUnion<TypeFieldT, Ts...>::reset() {
  Union<TypeFieldT, Ts...>::KVParser::reset();
  _value = _default_value;
}

template <typename TypeFieldT, typename... Ts>
template <size_t n, typename T, typename... TDs>
SUnion<TypeFieldT, Ts...>::ValueSetter<n, T, TDs...>::ValueSetter(
    Type &value, SUnion<TypeFieldT, Ts...> &parser)
    : ValueSetter<n + 1, TDs...>(value, parser) {
  if (parser.currentMemberId() != n) {
    return;
  }

  if (auto &field_parser = parser.template parser<n>(); field_parser.isSet()) {
    value = field_parser.pop();
  } else if (!parser._allow_default_value) {
    throw std::runtime_error("Empty storage union without a default value");
  }
}
}  // namespace SJParser
