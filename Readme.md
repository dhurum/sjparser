##SJParser

Stream json parser, written on top of yajl.  
Operates on level of objects.

###Example
```
using ParserType = Object<Value<std::string>, Value<int64_t>, SArray<Value<std::string>>>;

auto objectCb = [&](ParserType &parser) {
  DB.writeObject(parser.get<0>().pop(), parser.get<1>().pop(), parser.get<2>().pop());
  return true;
};

Parser<Array<ParserType>> parser({{"string", "integer", "array"}, objectCb});

parser.parse(buffer);
parser.finish();
```
