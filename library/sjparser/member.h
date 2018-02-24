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

#include <type_traits>
#include "internals/default_value.h"
#include "internals/traits.h"

namespace SJParser {

/** Enum with member presence options */
enum class Presence {
  /** Optional member */
  Optional
};

/** @brief Member of object or union parsers specification.
 *
 * This structure holds a specification of individual member for object and
 * union parsers.
 *
 * @tparam NameType Type of member name
 *
 * @tparam ParserType Type of member parser
 */
template <typename NameType, typename ParserType> struct Member {
  /** Member name */
  NameType name;
  /** Member parser */
  ParserType parser;

  /** Is this member optional */
  bool optional = false;

  /** Default value */
  DefaultValue<ParserType, IsStorageParser<ParserType>> default_value;

  /** @brief Constructor
   *
   * @param [in] name Member name.
   *
   * @param [in] parser Member parser, can be an lvalue reference or an rvalue.
   */
  Member(NameType name, ParserType &&parser);

  /** @brief Constructor
   *
   * Creates an optional member.
   *
   * @param [in] name Member name.
   *
   * @param [in] parser Member parser, can be an lvalue reference or an rvalue.
   *
   * @param [in] presence Presence enum, indicates that this is an optional
   * member.
   */
  Member(NameType name, ParserType &&parser, Presence presence);

  /** @brief Constructor
   *
   * Creates an optional member with a default value, can be used only with
   * storage parsers.
   *
   * @param [in] name Member name.
   *
   * @param [in] parser Member parser, can be an lvalue reference or an rvalue.
   *
   * @param [in] presence Presence enum, indicates that this is an optional
   * member.
   *
   * @param [in] default_value Default value.
   */
  template <typename U = std::decay_t<ParserType>>
  Member(NameType name, ParserType &&parser, Presence presence,
         typename U::Type default_value);

  /** Move constructor. */
  Member(Member &&other) noexcept;
};

template <typename ParserType>
Member(const char *, ParserType &&)->Member<std::string, ParserType>;

template <typename ParserType>
Member(int, ParserType &&)->Member<int64_t, ParserType>;

template <typename ParserType>
Member(float, ParserType &&)->Member<double, ParserType>;

template <typename NameType, typename ParserType>
Member(NameType, ParserType &&)->Member<NameType, ParserType>;

template <typename ParserType>
Member(const char *, ParserType &&, Presence)->Member<std::string, ParserType>;

template <typename ParserType>
Member(int, ParserType &&, Presence)->Member<int64_t, ParserType>;

template <typename ParserType>
Member(float, ParserType &&, Presence)->Member<double, ParserType>;

template <typename NameType, typename ParserType>
Member(NameType, ParserType &&, Presence)->Member<NameType, ParserType>;

template <typename ParserType, typename ValueType>
Member(const char *, ParserType &&, Presence, ValueType)
    ->Member<std::string, ParserType>;

template <typename ParserType, typename ValueType>
Member(int, ParserType &&, Presence, ValueType)->Member<int64_t, ParserType>;

template <typename ParserType, typename ValueType>
Member(float, ParserType &&, Presence, ValueType)->Member<double, ParserType>;

template <typename NameType, typename ParserType, typename ValueType>
Member(NameType, ParserType &&, Presence, ValueType)
    ->Member<NameType, ParserType>;
}  // namespace SJParser

#include "impl/member.h"
