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

 private:
  constexpr void checkTemplateParameters();
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

/****************************** Implementations *******************************/

template <typename NameType, typename ParserType>
Member<NameType, ParserType>::Member(NameType name, ParserType &&parser)
    : name{std::move(name)}, parser{std::forward<ParserType>(parser)} {
  checkTemplateParameters();
}

template <typename NameType, typename ParserType>
Member<NameType, ParserType>::Member(NameType name, ParserType &&parser,
                                     Presence /*presence*/)
    : name{std::move(name)},
      parser{std::forward<ParserType>(parser)},
      optional{true} {
  checkTemplateParameters();
}

template <typename NameType, typename ParserType>
template <typename U>
Member<NameType, ParserType>::Member(NameType name, ParserType &&parser,
                                     Presence /*presence*/,
                                     typename U::Type default_value)
    : name{std::move(name)},
      parser{std::forward<ParserType>(parser)},
      optional{true},
      default_value{true, std::move(default_value)} {
  checkTemplateParameters();
}

template <typename NameType, typename ParserType>
Member<NameType, ParserType>::Member(Member &&other) noexcept
    : name(std::move(other.name)),
      parser(std::forward<ParserType>(other.parser)),
      optional(other.optional),
      default_value(std::move(other.default_value)) {}

template <typename NameType, typename ParserType>
constexpr void Member<NameType, ParserType>::checkTemplateParameters() {
  // Formatting disabled because of a bug in clang-format
  // clang-format off
  static_assert(
      std::is_same_v<NameType, int64_t>
      || std::is_same_v<NameType, bool>
      || std::is_same_v<NameType, double>
      || std::is_same_v<NameType, std::string>,
      "Invalid type used for Member name, only int64_t, bool, double or "
      "std::string are allowed");
  // clang-format on
  static_assert(std::is_base_of_v<TokenParser, std::decay_t<ParserType>>,
                "Invalid parser used in Member");
}

}  // namespace SJParser
