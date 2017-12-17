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

#include "s_auto_object.h"
#include "s_custom_object.h"

namespace SJParser {

/** @cond INTERNAL Internal class, needed for SObject */

template <bool auto_type, typename... Ts> struct SObjectDispatcher {
  using Type = SCustomObject<Ts...>;
};

template <typename... Ts> struct SObjectDispatcher<true, Ts...> {
  using Type = SAutoObject<Ts...>;
};

/** @endcond */

#ifdef DOXYGEN_ONLY
/** @brief SCustomObject and SAutoObject dispatcher
 *
 * Will point to SCustomObject or SAutoObject based on the template parameters.
 * You should use it instead of using those types directly.
 */

template <typename... Ts> class SObject;
#endif

template <typename T, typename... Ts>
using SObject = typename SObjectDispatcher<std::is_base_of_v<TokenParser, T>, T,
                                           Ts...>::Type;

/** @cond INTERNAL Internal class, needed for Union */

template <typename T> struct UnionFieldType { using Type = T; };

template <> struct UnionFieldType<std::string> { using Type = FieldName; };

/** @endcond */
}  // namespace SJParser
