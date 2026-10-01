#pragma once

#include <string>
#include <vector>

namespace market {

// A small JSON value + parser (enough for configuration files).
// parse() throws std::runtime_error on malformed input.
class Json {
public:
    enum class Type { Null, Bool, Number, String, Array, Object };

    static Json parse(const std::string& text);

    Type type() const { return type_; }
    bool is_object() const { return type_ == Type::Object; }
    bool is_array() const { return type_ == Type::Array; }

    // Object member lookup; nullptr if missing or not an object.
    const Json* find(const std::string& key) const;

    // Typed member access with a default for missing/mistyped members.
    double number(const std::string& key, double fallback) const;
    bool boolean(const std::string& key, bool fallback) const;
    std::string string(const std::string& key, const std::string& fallback) const;

    double number_value() const { return number_; }
    bool bool_value() const { return bool_; }
    const std::string& string_value() const { return string_; }
    const std::vector<Json>& items() const { return values_; }  // array elements

private:
    friend class JsonParser;

    Type type_ = Type::Null;
    bool bool_ = false;
    double number_ = 0.0;
    std::string string_;
    std::vector<std::string> keys_;   // object keys (parallel to values_)
    std::vector<Json> values_;        // array elements / object values
};

}  // namespace market
