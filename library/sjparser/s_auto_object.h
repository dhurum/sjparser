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

#include "object.h"

namespace SJParser {

/** @brief %Object parser, that stores the result in a tuple of field parsers
 * types.
 *
 * Instead of using this type directly you should use an SObject, it will
 * automatically dispatch SCustomObject and SAutoObject based on the template
 * parameters.
 *
 * @tparam Ts A list of field parsers types.
 */

template <typename... Ts> class SAutoObject : public Object<Ts...> {
 public:
  /** A std::tuple with arguments for field parsers. */
  using ChildArgs = typename Object<Ts...>::ChildArgs;

  /** Stored value type */
  using Type = std::tuple<typename Ts::Type...>;

  /** Type alias for the parser options. */
  using Options = typename Object<Ts...>::Options;

  /** @brief Struct with arguments for the SAutoObject
   * @ref SAutoObject() "constructor".
   */
  struct Args {
    /** @param [in] args Sets #args.
     *
     * @param[in] default_value Default for the parser value.
     *
     * @param [in] options Sets #options.
     *
     * @param[in] on_finish (optional) Sets #on_finish.
     */
    Args(const ChildArgs &args, const Type &default_value,
         const Options &options,
         const std::function<bool(const Type &)> &on_finish = nullptr);

    /** @param [in] args Sets #args.
     *
     * @param[in] default_value Default for the parser value.
     *
     * @param[in] on_finish (optional) Sets #on_finish.
     */
    Args(const ChildArgs &args, const Type &default_value,
         const std::function<bool(const Type &)> &on_finish = nullptr);

    /** @param [in] args Sets #args.
     *
     * @param [in] options Sets #options.
     *
     * @param[in] on_finish (optional) Sets #on_finish.
     */
    Args(const ChildArgs &args, const Options &options,
         const std::function<bool(const Type &)> &on_finish = nullptr);

    /** @param [in] args Sets #args.
     *
     * @param[in] on_finish (optional) Sets #on_finish.
     */
    Args(const ChildArgs &args,
         const std::function<bool(const Type &)> &on_finish = nullptr);

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

    /** Default for the parser value. */
    Type default_value;

    /** @cond INTERNAL Internal flag, shows if default value was set. */
    bool allow_default_value = true;
    /** @endcond */

    /** #SJParser::ObjectOptions struct with parser options. */
    Options options;

    /** Callback, that will be called after an object is parsed.
     *
     * The callback will be called with a reference to the stored value as an
     * argument.
     *
     * If the callback returns false, parsing will be stopped with an error.
     */
    std::function<bool(const Type &)> on_finish;
  };

  /** @brief SAutoObject constructor.
   *
   * @param [in] args Args stucture.
   * If you do not specify @ref Args::default_value "defult_value" default value
   * and @ref Args::on_finish "on_finish" callback, you can pass a
   * @ref Args::args "tuple of field arguments" directly into the constructor.
   *
   * @note If you are passing a single field name (without arguments), you must
   * pass it directly to the constructor, without surrounding {}:
   * @code {.cpp} SObject<...> object("field"); @endcode
   */
  SAutoObject(const Args &args);
  SAutoObject(const SAutoObject &) = delete;

#ifdef DOXYGEN_ONLY
  /** @brief Check if the parser has a value.
   *
   * @return True if the parser has some value stored or false otherwise.
   */
  bool isSet();
#endif

  /** @brief Parsed value getter.
   *
   * @return Const reference to a parsed value.
   *
   * @throw std::runtime_error Thrown if the value is unset (no value was
   * parsed or #pop was called).
   */
  const Type &get() const;

  /** @brief Get the parsed value and unset the parser.
   *
   * Moves the parsed value out of the parser.
   *
   * @note If you want to use SCustomObject inside this parser, you need to
   * provide both a copy constructor or move constructor and a copy assignment
   * operators in the SCustomObject::Type, they are used by parser.
   *
   * @return Rvalue reference to the parsed value.
   *
   * @throw std::runtime_error Thrown if the value is unset (no value was
   * parsed or #pop was called).
   */
  Type &&pop();

 private:
  using TokenParser::_set;
  using TokenParser::checkSet;

  void finish() override;
  void reset() override;

  // This is placed in the private section because the ValueSetter uses pop on
  // all fields, so they are always unset after parsing.
  using Object<Ts...>::get;
  using Object<Ts...>::pop;

  template <size_t, typename...> struct ValueSetter {
    ValueSetter(Type & /*value*/, SAutoObject<Ts...> & /*parser*/) {}
  };

  template <size_t n, typename T, typename... TDs>
  struct ValueSetter<n, T, TDs...> : private ValueSetter<n + 1, TDs...> {
    ValueSetter(Type &value, SAutoObject<Ts...> &parser);
  };

  Type _value;
  Type _default_value;
  bool _allow_default_value;
  std::function<bool(const Type &)> _on_finish;
};
}  // namespace SJParser

#include "impl/s_auto_object.h"
