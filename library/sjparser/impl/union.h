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
Union<TypeFieldT, Ts...>::Args::Args(
    const ChildArgs &args,
    const std::function<bool(Union<TypeFieldT, Ts...> &)> &on_finish)
    : args(args), on_finish(on_finish) {}

template <typename TypeFieldT, typename... Ts>
Union<TypeFieldT, Ts...>::Args::Args(
    const FieldName &type_field, const ChildArgs &args,
    const std::function<bool(Union<TypeFieldT, Ts...> &)> &on_finish)
    : type_field(type_field), args(args), on_finish(on_finish) {}

template <typename TypeFieldT, typename... Ts>
Union<TypeFieldT, Ts...>::Union(const Args &args)
    : KVParser(args.args),
      _type_field(args.type_field),
      _on_finish(args.on_finish),
      _current_member_id(0) {
  for (size_t i = 0; i < KVParser::_fields_array.size(); ++i) {
    _fields_ids_map[KVParser::_fields_array[i]] = i;
  }
}

template <typename TypeFieldT, typename... Ts>
size_t Union<TypeFieldT, Ts...>::currentMemberId() {
  checkSet();
  return _current_member_id;
}

template <typename TypeFieldT, typename... Ts>
void Union<TypeFieldT, Ts...>::on(TokenType<TypeFieldT> value) {
  KVParser::reset();
  KVParser::onField(value);
  _current_member_id = _fields_ids_map[KVParser::_fields_map[value]];
}

template <typename TypeFieldT, typename... Ts>
void Union<TypeFieldT, Ts...>::on(MapStartT /*unused*/) {
  if (_type_field.empty()) {
    // Should never happen
    throw std::runtime_error("Union with an empty type field can't parse this");
  }
  KVParser::on(MapStartT{});
}

template <typename TypeFieldT, typename... Ts>
void Union<TypeFieldT, Ts...>::on(MapKeyT key) {
  if (_type_field.empty()) {
    // Should never happen
    throw std::runtime_error("Union with an empty type field can't parse this");
  }
  if (key.key != _type_field) {
    std::stringstream err;
    err << "Unexpected field " << key.key;
    throw std::runtime_error(err.str());
  }
}

template <typename TypeFieldT, typename... Ts>
void Union<TypeFieldT, Ts...>::childParsed() {
  KVParser::endParsing();
  if (_type_field.empty()) {
    // The union embedded into an object must propagate the end event to the
    // parent.
    KVParser::_dispatcher->on(MapEndT());
  }
}

template <typename TypeFieldT, typename... Ts>
void Union<TypeFieldT, Ts...>::finish() {
  if (_on_finish && !_on_finish(*this)) {
    throw std::runtime_error("Callback returned false");
  }
}
}  // namespace SJParser
