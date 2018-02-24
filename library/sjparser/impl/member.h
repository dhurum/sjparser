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

namespace {  // NOLINT

template <typename NameType, typename ParserType>
void checkTemplateParameters() {
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

}  // namespace

template <typename NameType, typename ParserType>
Member<NameType, ParserType>::Member(NameType name, ParserType &&parser)
    : name{std::move(name)}, parser{std::forward<ParserType>(parser)} {
  checkTemplateParameters<NameType, ParserType>();
}

template <typename NameType, typename ParserType>
Member<NameType, ParserType>::Member(NameType name, ParserType &&parser,
                                     Presence /*presence*/)
    : name{std::move(name)},
      parser{std::forward<ParserType>(parser)},
      optional{true} {
  checkTemplateParameters<NameType, ParserType>();
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
  checkTemplateParameters<NameType, ParserType>();
}

template <typename NameType, typename ParserType>
Member<NameType, ParserType>::Member(Member &&other) noexcept
    : name(std::move(other.name)),
      parser(std::forward<ParserType>(other.parser)),
      optional(other.optional),
      default_value(std::move(other.default_value)) {}
}  // namespace SJParser
