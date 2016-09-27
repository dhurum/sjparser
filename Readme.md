#SJParser

Streaming json parser, written on top of yajl.  
You specify expected structure of json and callbacks to be called for certain entities.

##Concepts
###Entity parsers
* `SJParser::Value`: Parser for simple value.  
  Takes value type as a template parameter. It can be:
  * `std::string`
  * `int64_t`
  * `bool`
  * `double`

  Constructor receives a finish callback, that will be called after value is parsed: `std::function<bool(const T &value)>`, where `value` is parsed value.

  `SJParser::Value` has methods:
  * `bool isSet()`: returns true if parser parsed some value, false otherwise.
  * `const T &get()`: T is parser's value type. Returns a reference to a parsed value.  
    If `isSet` is false - throws an exception.
  * `const T &&pop()`: T is parser's value type. Returns an rvalue reference to a parsed value.  
    If `isSet` is false - throws an exception.  
    Makes parser unset.


* `SJParser::Object`: Parser for object.  
  Takes a list of entity parsers types of its fields as template parameters.  
  Constructor receives a `struct` with two elements:
  1. `std::tuple` of `struct`s with two elements:
      1. `std::string` field name
      2. Constructor argument for appropriate parser

      If you do not want to provide any arguments to field parser, you can pass only string.

  2. Finish callback,  that will be called after object is parsed: `std::function<bool(T &parser)>`, where `parser` is this parser.

  If you don't want to provide finish callback, you can pass only `std::tuple` to the constructor.

  `SJParser::Object` has methods:
  * `T& get<n>()`: returns a reference to n-th field parser.  


* `SJParser::SObject`: Parser for object, that stores parsed result.  
  Takes a type of value it stores and a list of entity parsers for its fields as template parameters.  
  Constructor receives a `struct` with two elements:
  1. `std::tuple` of `struct`s with two elements:
      1. `std::string` field name
      2. Constructor argument for appropriate parser

      If you do not want to provide any arguments to field parser, you can pass only string.

  2. Finish callback,  that will be called after object is parsed: `std::function<bool(T &parser, V &value)>`, where `parser` is this parser and `value` is a value that this parser stores. You should set it from this callback.

  `SJParser::SObject` has methods:
  * `T& get<n>()`: returns a reference to n-th field parser.  
  * `bool isSet()`: returns true if parser parsed some value, false otherwise.
  * `const T &get()`: T is parser's value type. Returns a reference to a parsed value.  
    If `isSet` is false - throws an exception.
  * `const T &&pop()`: T is parser's value type. Returns an rvalue reference to a parsed value.  
    If `isSet` is false - throws an exception.  
    Makes parser unset.


* `SJParser::Array`: Parser for arrays.  
  Takes entity parser type for elements as a template parameter.  
  Constructor receives a `struct` with two elements:
  1. Argument for elements parser
  2. Finish callback, that will be called after array is parsed: `std::function<bool()>`.

  If you don't want to provide finish callback, you can pass only elements parser argument to the constructor.


* `SJParser::SArray`: Parser for arrays, that stores parsed result.  
  Takes entity parser type for elements as a template parameter.  
  Constructor receives a `struct` with two elements:
  1. Argument for elements parser
  2. Finish callback, that will be called after array is parsed: `std::function<bool(T &value)>`, where `value` is a vector of elements parser's storage type.

  If you don't want to provide finish callback, you can pass only elements parser argument to the constructor.

  `SJParser::SArray` has methods:
  * `bool isSet()`: returns true if parser parsed some value, false otherwise.
  * `const T &get()`: T is parser's value type. Returns a reference to a parsed value.  
    If `isSet` is false - throws an exception.
  * `const T &&pop()`: T is parser's value type. Returns an rvalue reference to a parsed value.  
    If `isSet` is false - throws an exception.  
    Makes parser unset.

###Main parser

* `SJParser::Parser`: Main parser class.  
  Takes entity parser for root element as a template parameter.  
  Constructor receives exactly same arguments as root elemet parser.

  `SJParser::Parser`has methods:
  * `bool parse(const std::string &data)`: parses a piece of json. Returns false in case of error.
  * `bool parse(const char *data, size_t len)`: parses a piece of json. Returns false in case of error.
  * `bool finish()`: finishes parsing. Returns false in case of error.
  * `std::string getError()`: returns parsing error.
  * `T &parser()`: returns root element parser.

##Example
```c++
using namespace SJParser;

using ParserType = Object<Value<std::string>, Value<int64_t>, SArray<Value<std::string>>>;

auto objectCb = [&](ParserType &parser) {
  DB.writeObject(parser.get<0>().pop(), parser.get<1>().pop(), parser.get<2>().pop());
  return true;
};

Parser<Array<ParserType>> parser({{"string", "integer", "array"}, objectCb});

parser.parse(R"(
[{
  "string": "str1",
  "integer": 1,
  "array": ["1", "2"]
}, {
  "string": "str2",
  "integer": 2,
  "array": ["3", "4"]
}])");

parser.finish();
```

For more examples, please see file [test.cpp](https://github.com/dhurum/sjparser/blob/master/test.cpp).
