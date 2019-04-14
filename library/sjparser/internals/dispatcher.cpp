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

#include "dispatcher.h"
#include "token_parser.h"

namespace SJParser {

Dispatcher::Dispatcher(TokenParser *parser) : _root_parser{parser} {
  _parsers.push_back(parser);
  parser->setDispatcher(this);
}

void Dispatcher::pushParser(TokenParser *parser) {
  _parsers.push_back(parser);
}

void Dispatcher::popParser() {
  if (_parsers.empty()) {
    throw std::runtime_error("Can not pop parser, parsers stack is empty");
  }
  _parsers.pop_back();

  if (!_parsers.empty()) {
    _parsers.back()->childParsed();
  }
}

bool Dispatcher::emptyParsersStack() {
  return _parsers.empty();
}

void Dispatcher::reset() {
  _parsers.clear();
  _parsers.push_back(_root_parser);
}
}  // namespace SJParser
