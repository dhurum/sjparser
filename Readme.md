[//]: # (start nodoc)
[![Build Status](https://travis-ci.org/dhurum/sjparser.svg?branch=master)](https://travis-ci.org/dhurum/sjparser)
[![License](https://img.shields.io/:license-mit-blue.svg)](https://badges.mit-license.org)

[//]: # (end nodoc)
# SJParser

Streaming json parser, written on top of yajl.  

The main use case for this parser is very long json documents with known structure, e.g. importing some data from a json representation.  

Basically it is a SAX parser on steroids - you specify the expected json structure and your callbacks, and they will be called after a whole object or array is parsed, not just on a `MapKey` or `ArrayEnd` events.

[//]: # (start nodoc)
## Documentation

You can find class reference [here](https://dhurum.github.io/sjparser/documentation/html/).  

Also you can check the [Concepts](#concepts).

[//]: # (end nodoc)
## Example

~~~cpp
#include <sjparser.h>

using SJParser::Parser;
using SJParser::Object;
using SJParser::Value;
using SJParser::Array;
using SJParser::SArray;

int main() {
  // Type alias for object parser, for easier callback declaration.
  // Object has 3 fields - std::string, int64_t, std::vector<std::string>
  using ParserType = Object<Value<std::string>, Value<int64_t>, SArray<Value<std::string>>>;

  // Callback, will be called once object is parsed
  auto objectCb = [&](ParserType &parser) {
    // Some external API call
    DB.writeObject(
      // Rvalue reference to the first object field (std::string)
      parser.get<0>().pop(),
      // If second field is present use it, otherwise use some default value
      parser.get<1>().isSet() ? parser.get<1>().get() : 0,
      // Rvalue reference to the third object field (std::vector<std::string>)
      parser.get<2>().pop());
    // Returning false from the callback with make parser stop with an error
    return true;
  };

  // Declare parser. It expects an array of objects.
  // List of field names and a callback are pased as an argument to the object parser,
  // array parser "understands" that they are not for its use and forwards them to
  // the child parser.
  Parser<Array<ParserType>> parser({{"string", "integer", "array"}, objectCb});

  // Parse a piece of json. During parsing object callback will be called.
  parser.parse(R"(
  [{
    "string": "str1",
    "integer": 1,
    "array": ["1", "2"]
  }, {
    "string": "str2",
    "array": ["3", "4"]
  }])");

  // Finish parsing
  parser.finish();
  return 0;
}
~~~

For more examples, please see [tests](https://github.com/dhurum/sjparser/blob/master/tests).

## Concepts <a name="concepts"></a>

Expected json structure is specified as template parameters of `SJParser::Parser` and entities parsers.  
Then you pass a structure of arguments for those parameters into the constructor.
Each class constructor receives a `struct`.

If constructor description  mentions that *«If you do not want to provide Y you can pass only X»* or *«If you don't want to provide X, you can call constructor without arguments»*, you can write this:

~~~cpp
//Providing both fields
Class obj({X, Y})z;

//Providing one field
Class obj(X);

//Providing nothing
Class obj;
~~~

Constructors of `SJParser::Object`, `SJParser::SObject`, `SJParser::Union`, `SJParser::Array` and `SJParser::SArray` take arguments of their nested types:

~~~cpp
//Providing both fields
Object<Class> obj({"field", {X, Y}});

//Providing one field
Object<Class> obj({"field", X});

//Providing nothing
Object<Class> obj({"field"});

//Providing nothing to field 1, two arguments to field 2 and one argument to field 3
Object<Class, Class, Class> obj({"field 1", {"field 2", {X, Y}}, {"field 3", X}});
~~~

Constructor of `SJParser::Parser` takes exactly the same arguments as it's template type:

~~~cpp
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

~~~

Fields, specified for `SJParser::Object`, `SJParser::SObject` with a custom value type and `SJParser::SObject` with auto value type and a default value are not mandatory, even empty object would be successfully parsed.  
The validation should be done in finish callback.  
If you call `get()` or `pop()` on a parser of a field, that was not present in the parsed object, exception will be thrown.  
You can check if field was parsed with method `isSet()`.  
So, for your mandatory fields you can just use `get()` or `pop()`, and for optional you can do checks with `isSet()` first.

