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

#include "internals/key_value_parser.h"

#include <functional>

namespace SJParser {

/** @brief %Object parser.
 *
 * @tparam Ts A list of field parsers types.
 */

template <typename... Ts>
class Object : public KeyValueParser<FieldName, Ts...> {
 protected:
  /** @cond INTERNAL Internal typedef */
  using KVParser = KeyValueParser<FieldName, Ts...>;
  /** @endcond */

 public:
  /** A std::tuple with arguments for field parsers. */
  using ChildArgs = typename KVParser::ChildArgs;

  /** Type alias for the parser options. */
  using Options = ObjectOptions;

  /** @brief Struct with arguments for the Object @ref Object() "constructor".
   */
  struct Args {
    /** @param [in] args Sets #args.
     *
     * @param [in] options Sets #options.
     *
     * @param [in] on_finish (optional) Sets #on_finish.
     */
    Args(const ChildArgs &args, const Options &options,
         const std::function<bool(Object<Ts...> &)> &on_finish = nullptr);

    /** @param [in] args Sets #args.
     *
     * @param [in] on_finish (optional) Sets #on_finish.
     *
     * @note If you are passing a single field name (without arguments)
     * to the constructor, you must pass it without surrounding {}:
     * @code {.cpp} Object<...>::Args args("field"); @endcode
     * However, if you initialize Args, you can pass the extra {}:
     * @code {.cpp} Object<...>::Args args = {"field"}; @endcode
     * @code {.cpp} Object<...>::Args args = {{"field"}}; @endcode
     */
    Args(const ChildArgs &args,
         const std::function<bool(Object<Ts...> &)> &on_finish = nullptr);

    /** A std::tuple with field arguments.
     *
     * A field argument is a structure where the first element is a string
     * with field's name and the second element is arguments to that field's
     * parser. If you do not want to provide any arguments to the field's
     * parser, you can pass only the field's name:
     * @code {.cpp} {"field1", {"field2", ...}, "field3"} @endcode
     * In this example field1 and field3 parsers do not receive any agruments
     * while field2 does receive something.
     */
    ChildArgs args;

    /** #SJParser::ObjectOptions struct with parser options. */
    Options options;

    /** Callback, that will be called after an object is parsed.
     *
     * The callback will be called with a reference to the parser as an
     * argument.
     *
     * If the callback returns false, parsing will be stopped with an error.
     */
    std::function<bool(Object<Ts...> &)> on_finish;
  };

  /** @brief Object constructor.
   *
   * @param [in] args Args stucture.
   * If you do not specify @ref Args::on_finish "on_finish" callback,
   * you can pass a @ref Args::args "tuple of field arguments" directly
   * into the constructor.
   *
   * @note If you are passing a single field name (without arguments) directly
   * to the constructor, you must pass it without surrounding {}:
   * @code {.cpp} Object<...> object("field"); @endcode
   */
  Object(const Args &args);
  Object(const Object &) = delete;

#ifdef DOXYGEN_ONLY
  /** @brief Universal field getter.
   *
   * @tparam n Index of the parser's field.
   *
   * @return If the n-th field parser stores value (is a Value, SObject or
   * SArray), then the method returns a const reference to the n-th field parser
   * value. Otherwise, returns a reference to the n-th field parser.
   *
   * @throw std::runtime_error thrown if the field parser value is unset (no
   * value was parsed or #pop was called for the field parser).
   */
  template <size_t n>
  const typename NthTypes<n, Ts...>::template ValueType<> &get();

  /** @brief Universal field getter.
   *
   * @tparam n Index of the parser's field.
   *
   * @return If the n-th field parser stores value (is a Value, SObject or
   * SArray), then the method returns a const reference to the n-th field parser
   * value. Otherwise, returns a reference to the n-th field parser.
   */
  template <size_t n> typename NthTypes<n, Ts...>::ParserType &get();

  /** @brief Field parser getter.
   *
   * @tparam n Index of the parser's field.
   *
   * @return Reference to n-th field parser.
   */
  template <size_t n> typename NthTypes<n, Ts...>::ParserType &parser();

  /** @brief Get the field parsed value and unset the field parser.
   *
   * Moves the n-th field parsed value out of the field parser.
   *
   * @tparam n Index of the parser's field.
   *
   * @return Rvalue reference to n-th field parser value.
   *
   * @throw std::runtime_error thrown if the field parser value is unset (no
   * value was parsed or #pop was called for the field parser).
   */
  template <size_t n> typename NthTypes<n, Ts...>::template ValueType<> &&pop();
#endif

 private:
  using KVParser::on;
  void on(MapKeyT key) override;

  void finish() override;

  std::function<bool(Object<Ts...> &)> _on_finish;
};
}  // namespace SJParser

#include "impl/object.h"
