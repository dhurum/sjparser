#pragma once

#include <yajl/yajl_parse.h>
#include <array>
#include <functional>
#include <initializer_list>
#include <memory>
#include <stack>
#include <string>
#include <unordered_map>
#include <vector>

namespace SJParser {

  struct MapStartT {};
  struct MapKeyT {
    const std::string &key;
  };
  struct MapEndT {};
  struct ArrayStartT {};
  struct ArrayEndT {};

  class Dispatcher;

  class TokenParser {
   public:
    virtual void setDispatcher(Dispatcher *dispatcher);
    bool isSet();
    // TODO: virtual reset
    void reset();
    bool endParsing();

    virtual bool on(const bool & /*value*/) { return false; }
    virtual bool on(const int64_t & /*value*/) { return false; }
    virtual bool on(const double & /*value*/) { return false; }
    virtual bool on(const std::string & /*value*/) { return false; }
    virtual bool on(const MapStartT) { return false; }
    virtual bool on(const MapKeyT & /*key*/) { return false; }
    virtual bool on(const MapEndT) { return false; }
    virtual bool on(const ArrayStartT) { return false; }
    virtual bool on(const ArrayEndT) { return false; }

    virtual bool finish() { return true; }

    virtual ~TokenParser() = default;

   protected:
    Dispatcher *_dispatcher = nullptr;
    bool _set = false;
  };

  template <typename T>
  class ValueParser : public TokenParser {
   public:
    ValueParser(std::function<bool(const T &)> on_finish = nullptr)
        : _on_finish(on_finish) {}
    virtual bool on(const T &value) override;
    virtual bool finish() override;
    const T &get();

   private:
    T _value;
    std::function<bool(const T &)> _on_finish;
  };

  template <typename>
  struct NeedsParameters {
    constexpr static bool value = false;
  };

  class Param {
   public:
    Param(const std::string &str) { _string = str; }
    Param(const char *str) { _string = str; }
    Param(const std::initializer_list<Param> &list) {
      _list = list;
      _is_list = true;
    }

    bool isList() const { return _is_list; }
    const std::string &getString() const {
      if (_is_list) {
        throw std::runtime_error("Attempt to get string from list parameter");
      }
      return _string;
    }
    const std::initializer_list<Param> &getList() const {
      if (!_is_list) {
        throw std::runtime_error("Attempt to get list from string parameter");
      }
      return _list;
    }

   private:
    std::string _string;
    std::initializer_list<Param> _list;
    bool _is_list = false;
  };

  class ObjectParserBase : public TokenParser {
   public:
    virtual void setDispatcher(Dispatcher *dispatcher) override;

    virtual bool on(const MapStartT) override;
    virtual bool on(const MapKeyT &key) override;
    virtual bool on(const MapEndT) override;

   protected:
    std::unordered_map<std::string, TokenParser *> _fields_map;
  };

  template <typename... Ts>
  class ObjectParser : public ObjectParserBase {
   public:
    // XXX: Param -> X, field_names -> x
    ObjectParser(std::initializer_list<Param> field_names,
                 std::function<bool(ObjectParser<Ts...> &)> on_finish = nullptr)
        : _fields_init(field_names),
          _fields(_fields_array, _fields_init),
          _on_finish(on_finish) {
      if (sizeof...(Ts) > field_names.size()) {
        throw std::runtime_error("Too few field names for object parser");
      }

      size_t i = 0;
      for (const auto &name : field_names) {
        if (!name.isList()) {
          _fields_map[name.getString()] = _fields_array[i];
          ++i;
        }
      }
    }

    virtual bool finish() override {
      if (!_on_finish) {
        return true;
      }
      return _on_finish(*this);
    }

    template <size_t n, typename T, typename... TDs>
    struct NthType {
      using type = typename NthType<n - 1, TDs...>::type;
    };

    template <typename T, typename... TDs>
    struct NthType<0, T, TDs...> {
      using type = T;
    };

    template <size_t n>
    typename NthType<n, Ts...>::type &get() {
      return *reinterpret_cast<typename NthType<n, Ts...>::type *>(
          _fields_array[n]);
    }

   private:
    struct FieldInit {
      FieldInit(std::initializer_list<Param> init_list) {
        for (const auto &elt : init_list) {
          if (elt.isList()) {
            init.push(elt);
          }
        }
      }
      std::stack<Param> init;
    };

    template <size_t, typename...>
    struct Field {
      Field(std::array<TokenParser *, sizeof...(Ts)> & /*fields_array*/,
            FieldInit & /*fields_init*/) {}
    };

    template <size_t n, typename T, typename... TDs>
    struct Field<n, T, TDs...> : private Field<n + 1, TDs...> {
      template <typename U = T>
      Field(std::array<TokenParser *, sizeof...(Ts)> &fields_array,
            FieldInit &fields_init,
            typename std::enable_if<NeedsParameters<U>::value>::type * = 0)
          : Field<n + 1, TDs...>(fields_array, fields_init),
            _field(fields_init.init.top().getList()) {
        fields_array[n] = &_field;
        fields_init.init.pop();
      }

      template <typename U = T>
      Field(std::array<TokenParser *, sizeof...(Ts)> &fields_array,
            FieldInit &fields_init,
            typename std::enable_if<!NeedsParameters<U>::value>::type * = 0)
          : Field<n + 1, TDs...>(fields_array, fields_init) {
        fields_array[n] = &_field;
      }

      T _field;
    };

    std::array<TokenParser *, sizeof...(Ts)> _fields_array;
    FieldInit _fields_init;
    Field<0, Ts...> _fields;
    std::function<bool(ObjectParser<Ts...> &)> _on_finish;
  };

  template <typename... Ts>
  struct NeedsParameters<ObjectParser<Ts...>> {
    constexpr static bool value = true;
  };

  template <typename... Ts>
  std::shared_ptr<ObjectParser<Ts...>> makeObjectParser(
      std::initializer_list<Param> field_names,
      std::function<bool(ObjectParser<Ts...> &)> on_finish = nullptr) {
    return std::make_shared<ObjectParser<Ts...>>(field_names, on_finish);
  }

  class ArrayParserBase : public TokenParser {
   public:
    ArrayParserBase(std::function<bool()> on_finish)
        : _on_finish(on_finish) {}

    virtual bool on(const bool &value) override;
    virtual bool on(const int64_t &value) override;
    virtual bool on(const double &value) override;
    virtual bool on(const std::string &value) override;
    virtual bool on(const MapStartT) override;
    virtual bool on(const ArrayStartT) override;
    virtual bool on(const ArrayEndT) override;

    virtual bool finish() override {
      if (!_on_finish) {
        return true;
      }
      return _on_finish();
    }

   protected:
    TokenParser *_parser;
    std::function<bool()> _on_finish;

   private:
    bool _started = false;
  };

  template <typename T>
  class ArrayParser : public ArrayParserBase {};

  template <typename T>
  class ArrayParser<ValueParser<T>> : public ArrayParserBase {
   public:
    ArrayParser(std::function<bool(const T &)> on_element = nullptr,
                std::function<bool()> on_finish = nullptr)
        : ArrayParserBase(on_finish),
          _parser(on_element ? on_element : [&](const T &value) {
            _values.push_back(value);
            return true;
          }) {
      ArrayParserBase::_parser = &_parser;
    }

    const std::vector<T> &get() { return _values; }

   private:
    std::vector<T> _values;
    ValueParser<T> _parser;
  };

  template <typename... Ts>
  class ArrayParser<ObjectParser<Ts...>> : public ArrayParserBase {
   public:
    ArrayParser(std::function<bool(ObjectParser<Ts...> &)> on_element,
                std::initializer_list<Param> params,
                std::function<bool()> on_finish = nullptr)
        : ArrayParserBase(on_finish), _parser(params, on_element) {
      ArrayParserBase::_parser = &_parser;
    }

   private:
    ObjectParser<Ts...> _parser;
  };

  class Dispatcher {
   public:
    Dispatcher(TokenParser *parser);
    void pushParser(TokenParser *parser);
    void popParser();

    template <typename T>
    bool on(const T &value);

   protected:
    std::stack<TokenParser *> _parsers;
    std::function<void()> _on_completion;
  };

  class Parser {
   public:
    Parser(std::shared_ptr<TokenParser> parser);
    ~Parser();
    bool parse(const std::string &data);
    bool finish();
    std::string getError();

   protected:
    const yajl_callbacks _callbacks;
    yajl_handle _handle;
    std::shared_ptr<TokenParser> _parser;
    Dispatcher _dispatcher;
  };

  template <typename T>
  bool ValueParser<T>::on(const T &value) {
    _value = value;
    _set = true;

    return endParsing();
  }

  template <typename T>
  bool ValueParser<T>::finish() {
    if (!_on_finish) {
      return true;
    }
    return _on_finish(_value);
  }

  template <typename T>
  const T &ValueParser<T>::get() {
    // TODO: exception
    return _value;
  }
}
