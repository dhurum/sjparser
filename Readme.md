# SJParser

Streaming json parser, written on top of yajl.  
You specify expected structure of json and callbacks to be called for certain entities.

## Example

```c++
#include <sjparser.h>

using SJParser::Parser;
using SJParser::Object;
using SJParser::Value;
using SJParser::Array;
using SJParser::SArray;

int main() {
  //Type alias for object parser, for easier callback declaration.
  //Object has 3 fields - std::string, int64_t, std::vector<std::string>
  using ParserType = Object<Value<std::string>, Value<int64_t>, SArray<Value<std::string>>>;

  //Callback, will be called once object is parsed
  auto objectCb = [&](ParserType &parser) {
    //Some external API call
    DB.writeObject(
      //Rvalue reference to the first object field (std::string)
      parser.get<0>().pop(),
      //If second field is present use it, otherwise use some default value
      parser.get<1>().isSet() ? parser.get<1>().get() : 0,
      //Rvalue reference to the third object field (std::vector<std::string>)
      parser.get<2>().pop());
    //Returning false from the callback with make parser stop with an error
    return true;
  };

  //Declare parser. It expects an array of objects.
  //List of field names and a callback are pased as an argument to the object parser
  Parser<Array<ParserType>> parser({{"string", "integer", "array"}, objectCb});

  //Parse a piece of json. During parsing object callback will be called.
  parser.parse(R"(
  [{
    "string": "str1",
    "integer": 1,
    "array": ["1", "2"]
  }, {
    "string": "str2",
    "array": ["3", "4"]
  }])");

  //Finish parsing
  parser.finish();
  return 0;
}
```

For more examples, please see file [test.cpp](https://github.com/dhurum/sjparser/blob/master/tests/test.cpp).

## Concepts

Expected json structure is specified as template parameters of `SJParser::Parser` and entities parsers.  
Then you pass a structure of arguments for those parameters into the constructor.
Each class constructor receives a `struct`.

If constructor description  mentions that *«If you do not want to provide Y you can pass only X»* or *«If you don't want to provide X, you can call constructor without arguments»*, you can write this:

```c++
//Providing both fields
Class obj({X, Y})z;

//Providing one field
Class obj(X);

//Providing nothing
Class obj;
```

Constructors of `Object`, `SObject`, `Union`, `Array` and `SArray` take arguments for nested types:

```c++
//Providing both fields
Object<Class> obj({"field", {X, Y}});

//Providing one field
Object<Class> obj({"field", X});

//Providing nothing
Object<Class> obj({"field"});

//Providing nothing to field 1, two arguments to field 2 and one argument to field 3
Object<Class, Class, Class> obj({"field 1", {"field 2", {X, Y}}, {"field 3", X}});
```

Constructor of `SJParser::Parser` takes exactly the same arguments as it's template type:

```c++
//Providing both fields
Parser<Object<Class>> parser({"field", {X, Y}});

//Providing one field
Parser<Object<Class>> parser({"field", X});

//Providing nothing
Parser<Object<Class>> parser({"field"});

//Providing nothing to field 1, two arguments to field 2 and one argument to field 3
Parser<Object<Class, Class, Class>>({"field 1", {"field 2", {X, Y}}, {"field 3", X}})

//Providing both fields
Parser<Class> parser({X, Y});

//Providing one field
Parser<Class> parser(X);

//Providing nothing
Parser<Class> parser;

```

Fields, specified for `Object` and `SObject` are not mandatory, even empty object would be successfully parsed.  
The validation should be done in finish callback.  
If you call `get()` or `pop()` on a parser of a field, that was not present in the parsed object, exception will be thrown.  
You can check if field was parsed with method `isSet()`.  
So, for your mandatory fields you can just use `get()` or `pop()`, and for optional you can do checks with `isSet()` first.

### Entity parsers

* `SJParser::Value`: Parser for simple value.  
  Takes value type as a template parameter. It can be:
  * `std::string`
  * `int64_t`
  * `bool`
  * `double`

  Constructor receives a finish callback, that will be called after value is parsed: `std::function<bool(const T &value)>`, where `value` is parsed value.  
  Returning false from callback will cause parser to stop parsing.
  If you don't want to provide finish callback, you can call constructor without arguments.

  `SJParser::Value` has methods:
  * `bool isSet()`: returns true if parser parsed some value, false otherwise.
  * `const T &get()`: T is parser's value type. Returns a reference to a parsed value.  
    If `isSet` is false - throws a `std::runtime` exception.
  * `const T &&pop()`: T is parser's value type. Returns an rvalue reference to a parsed value.  
    If `isSet` is false - throws a `std::runtime` exception.  
    Makes parser unset.


* `SJParser::Object`: Parser for object.  
  Takes a list of entity parsers types of its fields as template parameters.  
  Constructor receives a `struct` with two elements:
  1. `std::tuple` of `struct`s with two elements:
      1. `std::string` field name
      2. Constructor argument for appropriate parser

      If you do not want to provide any arguments to field parser, you can pass only string.

  2. Finish callback,  that will be called after object is parsed: `std::function<bool(T &parser)>`, where `parser` is this parser.  
     Returning false from callback will cause parser to stop parsing.

  If you don't want to provide finish callback, you can pass only `std::tuple` to the constructor.  
  If you have only one field without arguments, you need to pass it without brackets.

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
     Returning false from callback will cause parser to stop parsing.

  `SJParser::SObject` has methods:
  * `T& get<n>()`: returns a reference to n-th field parser.  
  * `bool isSet()`: returns true if parser parsed some value, false otherwise.
  * `const T &get()`: T is parser's value type. Returns a reference to a parsed value.  
    If `isSet` is false - throws a `std::runtime` exception.
  * `const T &&pop()`: T is parser's value type. Returns an rvalue reference to a parsed value.  
    If `isSet` is false - throws a `std::runtime` exception.  
    Makes parser unset.


* `SJParser::Union`: Parser for union of objects.
  Takes key field type and list of possible object types as template parameters.  
  Union can be stand-alone or a embedded in an object.  
  In standalone union first field must be a key field, and the rest are those of respective object.  
  If union is embedded into object, fields before key field are considered object's, and after - union's.  

  In stand-alone mode constructor receives a `struct` with three elements:
  1. `std::string` key field name.
  2. `std::tuple` of `struct`s with two elements:
      1. key field value
      2. Arguments for respective member object

  3. Finish callback (optional), that will be called after union is parsed: `std::function<bool(T &parser)>`, where `parser` is this parser.

  In embedded mode constructor receives a `struct` with two elements:
  1. `std::tuple` of `struct`s with two elements:
      1. key field value
      2. Arguments for respective member object

  2. Finish callback (optional), that will be called after union is parsed: `std::function<bool(T &parser)>`, where `parser` is this parser.

  If you don't want to provide finish callback, you can pass only `std::tuple` to the constructor.  

  `SJParser::Union` has methods:
  * `T& get<n>()`: returns a reference to n-th member parser.  
  * `bool isSet()`: returns true if parser parsed some value, false otherwise.
  * `size_t currentMemberId()`: Returns id of parsed member.
    If `isSet` is false - throws a `std::runtime` exception.  


* `SJParser::Array`: Parser for arrays.  
  Takes entity parser type for elements as a template parameter.  
  Constructor receives a `struct` with two elements:
  1. Argument for elements parser
  2. Finish callback, that will be called after array is parsed: `std::function<bool()>`.  
     Returning false from callback will cause parser to stop parsing.

  If you don't want to provide finish callback, you can pass only elements parser argument to the constructor.


* `SJParser::SArray`: Parser for arrays, that stores parsed result.  
  Takes entity parser type for elements as a template parameter.  
  Constructor receives a `struct` with two elements:
  1. Argument for elements parser
  2. Finish callback, that will be called after array is parsed: `std::function<bool(T &value)>`, where `value` is a vector of elements parser's storage type.  
     Returning false from callback will cause parser to stop parsing.

  If you don't want to provide finish callback, you can pass only elements parser argument to the constructor.

  `SJParser::SArray` has methods:
  * `bool isSet()`: returns true if parser parsed some value, false otherwise.
  * `const T &get()`: T is parser's value type. Returns a reference to a parsed value.  
    If `isSet` is false - throws a `std::runtime` exception.
  * `const T &&pop()`: T is parser's value type. Returns an rvalue reference to a parsed value.  
    If `isSet` is false - throws a `std::runtime` exception.  
    Makes parser unset.

### Main parser

* `SJParser::Parser`: Main parser class.  
  Takes entity parser for root element as a template parameter.  
  Constructor receives exactly same arguments as root elemet parser.

  `SJParser::Parser`has methods:
  * `bool parse(const std::string &data)`: parses a piece of json. Returns false in case of error.
  * `bool parse(const char *data, size_t len)`: parses a piece of json. Returns false in case of error.
  * `bool finish()`: finishes parsing. Returns false in case of error.
  * `std::string getError()`: returns parsing error.
  * `T &parser()`: returns root element parser.
