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
Value<T>::Value(const Args &on_finish) : _on_finish(on_finish) {}

template <typename T> void Value<T>::on(const T &value) {
  _value = value;
  endParsing();
}

template <typename T> void Value<T>::finish() {
  if (_on_finish && !_on_finish(_value)) {
    throw std::runtime_error("Callback returned false");
  }
}

template <typename T> const T &Value<T>::get() const {
  checkSet();
  return _value;
}

template <typename T> T &&Value<T>::pop() {
  checkSet();
  _set = false;
  return std::move(_value);
}

template <typename... Ts>
Object<Ts...>::Args::Args(const ChildArgs &args,
                          const std::function<bool(Object<Ts...> &)> &on_finish)
    : args(args), on_finish(on_finish) {}

template <typename... Ts>
Object<Ts...>::Object(const Args &args)
    : KVParser(args.args), _on_finish(args.on_finish) {}

template <typename... Ts> void Object<Ts...>::on(const MapKeyT &key) {
  KVParser::onField(key.key);
}

template <typename... Ts> void Object<Ts...>::finish() {
  if (_on_finish && !_on_finish(*this)) {
    throw std::runtime_error("Callback returned false");
  }
}

template <typename T, typename... Ts>
SCustomObject<T, Ts...>::Args::Args(
    const ChildArgs &args,
    const std::function<bool(SCustomObject<T, Ts...> &, T &)> &on_finish)
    : args(args), on_finish(on_finish) {}

template <typename T, typename... Ts>
SCustomObject<T, Ts...>::SCustomObject(const Args &args)
    : Object<Ts...>(args.args), _on_finish(args.on_finish) {}

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

template <typename... Ts>
SAutoObject<Ts...>::Args::Args(
    const ChildArgs &args, const Type &default_value,
    const std::function<bool(const Type &)> &on_finish)
    : args(args), default_value(default_value), on_finish(on_finish) {}

template <typename... Ts>
SAutoObject<Ts...>::Args::Args(
    const ChildArgs &args, const std::function<bool(const Type &)> &on_finish)
    : args(args), allow_default_value(false), on_finish(on_finish) {}

template <typename... Ts>
SAutoObject<Ts...>::SAutoObject(const Args &args)
    : Object<Ts...>(args.args),
      _default_value(args.default_value),
      _allow_default_value(args.allow_default_value),
      _on_finish(args.on_finish) {}

template <typename... Ts>
const typename SAutoObject<Ts...>::Type &SAutoObject<Ts...>::get() const {
  checkSet();
  return _value;
}

template <typename... Ts>
typename SAutoObject<Ts...>::Type &&SAutoObject<Ts...>::pop() {
  checkSet();
  _set = false;
  return std::move(_value);
}

template <typename... Ts> void SAutoObject<Ts...>::finish() {
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

template <typename... Ts> void SAutoObject<Ts...>::reset() {
  Object<Ts...>::KVParser::reset();
  _value = _default_value;
}

template <typename... Ts>
template <size_t n, typename T, typename... TDs>
SAutoObject<Ts...>::ValueSetter<n, T, TDs...>::ValueSetter(
    Type &value, SAutoObject<Ts...> &parser)
    : ValueSetter<n + 1, TDs...>(value, parser) {
  auto &field_parser = parser.template parser<n>();
  if (field_parser.isSet()) {
    std::get<n>(value) = field_parser.pop();
  } else if (!parser._allow_default_value) {
    throw std::runtime_error(
        "Not all fields are set in an storage object without a default value");
  }
}

template <typename I, typename... Ts>
Union<I, Ts...>::Args::Args(
    const ChildArgs &args,
    const std::function<bool(Union<I, Ts...> &)> &on_finish)
    : args(args), on_finish(on_finish) {}

template <typename I, typename... Ts>
Union<I, Ts...>::Args::Args(
    const FieldName &type_field, const ChildArgs &args,
    const std::function<bool(Union<I, Ts...> &)> &on_finish)
    : type_field(type_field), args(args), on_finish(on_finish) {}

template <typename I, typename... Ts>
Union<I, Ts...>::Union(const Args &args)
    : KVParser(args.args),
      _type_field(args.type_field),
      _on_finish(args.on_finish),
      _current_member_id(0) {
  for (size_t i = 0; i < KVParser::_fields_array.size(); ++i) {
    _fields_ids_map[KVParser::_fields_array[i]] = i;
  }
}

template <typename I, typename... Ts>
size_t Union<I, Ts...>::currentMemberId() {
  checkSet();
  return _current_member_id;
}

template <typename I, typename... Ts> void Union<I, Ts...>::on(const I &value) {
  KVParser::reset();
  KVParser::onField(value);
  _current_member_id = _fields_ids_map[KVParser::_fields_map[value]];
}

template <typename I, typename... Ts>
void Union<I, Ts...>::on(MapStartT /*unused*/) {
  if (_type_field.empty()) {
    // Should never happen
    throw std::runtime_error("Union with an empty type field can't parse this");
  }
}

template <typename I, typename... Ts>
void Union<I, Ts...>::on(const MapKeyT &key) {
  if (_type_field.empty()) {
    // Should never happen
    throw std::runtime_error("Union with an empty type field can't parse this");
  }
  if (key.key != _type_field) {
    throw std::runtime_error("Unexpected field " + key.key);
  }
}

template <typename I, typename... Ts> void Union<I, Ts...>::childParsed() {
  KVParser::endParsing();
  if (_type_field.empty()) {
    // The union embedded into an object must propagate the end event to the
    // parent.
    KVParser::_dispatcher->on(MapEndT());
  }
}

template <typename I, typename... Ts> void Union<I, Ts...>::finish() {
  if (_on_finish && !_on_finish(*this)) {
    throw std::runtime_error("Callback returned false");
  }
}

template <typename T>
Array<T>::Args::Args(const ChildArgs &args,
                     const std::function<bool(Array<T> &)> &on_finish)
    : args(args), on_finish(on_finish) {}

template <typename T>
template <typename U>
Array<T>::Args::Args(const GrandChildArgs<U> &args,
                     const std::function<bool(Array<T> &)> &on_finish)
    : args(args), on_finish(on_finish) {}

template <typename T>
Array<T>::Array(const Args &args)
    : _parser(args.args), _on_finish(args.on_finish) {
  _parser_ptr = &_parser;
}

template <typename T> void Array<T>::finish() {
  if (_on_finish && !_on_finish(*this)) {
    throw std::runtime_error("Callback returned false");
  }
}

template <typename T>
SArray<T>::Args::Args(const ChildArgs &args,
                      const std::function<bool(const Type &)> &on_finish)
    : args(args), on_finish(on_finish) {}

template <typename T>
template <typename U>
SArray<T>::Args::Args(const GrandChildArgs<U> &args,
                      const std::function<bool(const Type &)> &on_finish)
    : args(args), on_finish(on_finish) {}

template <typename T>
SArray<T>::Args::Args(const std::function<bool(const Type &)> &on_finish)
    : on_finish(on_finish) {}

template <typename T>
SArray<T>::SArray(const Args &args)
    : Array<T>(args.args), _on_finish(args.on_finish) {}

template <typename T> const typename SArray<T>::Type &SArray<T>::get() const {
  checkSet();
  return _values;
}

template <typename T> typename SArray<T>::Type &&SArray<T>::pop() {
  checkSet();
  _set = false;
  return std::move(_values);
}

template <typename T> void SArray<T>::childParsed() {
  _values.push_back(Array<T>::_parser.pop());
}

template <typename T> void SArray<T>::finish() {
  if (_on_finish && !_on_finish(_values)) {
    throw std::runtime_error("Callback returned false");
  }
}

template <typename T> void SArray<T>::reset() {
  _values = Type();
}

template <typename T, typename Impl>
Parser<T, Impl>::Parser(const typename T::Args &args) : _parser(args) {
  this->setTokenParser(&_parser);
}

template <typename T, typename Impl>
template <typename U>
Parser<T, Impl>::Parser(const typename U::ChildArgs &args)
    : Parser(typename T::Args(args)) {}

template <typename T, typename Impl>
template <typename U>
Parser<T, Impl>::Parser(const typename U::template GrandChildArgs<U> &args)
    : Parser(typename T::Args(args)) {}

template <typename T, typename Impl>
template <typename U>
Parser<T, Impl>::Parser(const typename U::CallbackType &callback)
    : Parser(typename T::Args(callback)) {}

template <typename T, typename Impl> T &Parser<T, Impl>::parser() {
  return _parser;
}
}  // namespace SJParser
