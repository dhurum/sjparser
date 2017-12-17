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

#include "token_parser.h"

#include <list>

namespace SJParser {

class Ignore : public TokenParser {
 public:
  void reset() override;

  // Protected because we need to inherit from this class in order to unit test
  // it
 protected:
  enum class Structure { Object, Array };
  std::list<Structure> _structure{};

  void onValue();
  void on(bool /*value*/) override;
  void on(int64_t /*value*/) override;
  void on(double /*value*/) override;
  void on(std::string_view /*value*/) override;
  void on(MapStartT /*unused*/) override;
  void on(MapKeyT /*key*/) override;
  void on(MapEndT /*unused*/) override;
  void on(ArrayStartT /*unused*/) override;
  void on(ArrayEndT /*unused*/) override;
  void finish() override;
};
}  // namespace SJParser
