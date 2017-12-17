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

#include "key_value_parser.h"

namespace SJParser {

std::basic_ostream<char> &operator<<(std::basic_ostream<char> &stream,
                                     const FieldName &name) {
  return stream << name.str();
}

FieldName::FieldName(std::string str) : _str(std::move(str)) {}

FieldName::FieldName(std::string_view str) : _str(str) {}

FieldName::FieldName(const char *str) : _str(str) {}

FieldName::operator const std::string &() const {
  return _str;
}

bool FieldName::operator==(const FieldName &other) const {
  return _str == other._str;
}

const std::string &FieldName::str() const {
  return _str;
}
}  // namespace SJParser
