#include "market/core/json.hpp"

#include <cstdlib>
#include <stdexcept>

namespace market {

class JsonParser {
public:
    explicit JsonParser(const std::string& text) : text_(text) {}

    Json parse_document() {
        Json value = parse_value();
        skip_whitespace();
        if (pos_ != text_.size()) { fail("unexpected trailing characters"); }
        return value;
    }

private:
    const std::string& text_;
    std::size_t pos_ = 0;

    [[noreturn]] void fail(const std::string& message) const {
        throw std::runtime_error("JSON error at offset " + std::to_string(pos_) + ": " + message);
    }

    void skip_whitespace() {
        while (pos_ < text_.size() &&
               (text_[pos_] == ' ' || text_[pos_] == '\t' || text_[pos_] == '\n' || text_[pos_] == '\r')) {
            ++pos_;
        }
    }

    char peek() {
        skip_whitespace();
        if (pos_ >= text_.size()) { fail("unexpected end of input"); }
        return text_[pos_];
    }

    void expect(char c) {
        if (peek() != c) { fail(std::string("expected '") + c + "'"); }
        ++pos_;
    }

    bool consume_literal(const char* literal) {
        std::string lit(literal);
        if (text_.compare(pos_, lit.size(), lit) == 0) {
            pos_ += lit.size();
            return true;
        }
        return false;
    }

    Json parse_value() {
        char c = peek();
        Json value;
        if (c == '{') { parse_object(value); }
        else if (c == '[') { parse_array(value); }
        else if (c == '"') { value.type_ = Json::Type::String; value.string_ = parse_string(); }
        else if (consume_literal("true")) { value.type_ = Json::Type::Bool; value.bool_ = true; }
        else if (consume_literal("false")) { value.type_ = Json::Type::Bool; value.bool_ = false; }
        else if (consume_literal("null")) { value.type_ = Json::Type::Null; }
        else { parse_number(value); }
        return value;
    }

    void parse_object(Json& value) {
        value.type_ = Json::Type::Object;
        expect('{');
        if (peek() == '}') { ++pos_; return; }
        while (true) {
            if (peek() != '"') { fail("expected string key"); }
            std::string key = parse_string();
            expect(':');
            value.keys_.push_back(std::move(key));
            value.values_.push_back(parse_value());
            char c = peek();
            ++pos_;
            if (c == '}') { return; }
            if (c != ',') { fail("expected ',' or '}'"); }
        }
    }

    void parse_array(Json& value) {
        value.type_ = Json::Type::Array;
        expect('[');
        if (peek() == ']') { ++pos_; return; }
        while (true) {
            value.values_.push_back(parse_value());
            char c = peek();
            ++pos_;
            if (c == ']') { return; }
            if (c != ',') { fail("expected ',' or ']'"); }
        }
    }

    std::string parse_string() {
        expect('"');
        std::string out;
        while (pos_ < text_.size()) {
            char c = text_[pos_++];
            if (c == '"') { return out; }
            if (c != '\\') { out += c; continue; }
            if (pos_ >= text_.size()) { break; }
            char e = text_[pos_++];
            switch (e) {
                case '"': out += '"'; break;
                case '\\': out += '\\'; break;
                case '/': out += '/'; break;
                case 'n': out += '\n'; break;
                case 't': out += '\t'; break;
                case 'r': out += '\r'; break;
                case 'b': out += '\b'; break;
                case 'f': out += '\f'; break;
                case 'u': {
                    if (pos_ + 4 > text_.size()) { fail("bad unicode escape"); }
                    unsigned code = static_cast<unsigned>(std::strtoul(text_.substr(pos_, 4).c_str(), nullptr, 16));
                    pos_ += 4;
                    if (code < 0x80) {
                        out += static_cast<char>(code);
                    } else if (code < 0x800) {
                        out += static_cast<char>(0xC0 | (code >> 6));
                        out += static_cast<char>(0x80 | (code & 0x3F));
                    } else {
                        out += static_cast<char>(0xE0 | (code >> 12));
                        out += static_cast<char>(0x80 | ((code >> 6) & 0x3F));
                        out += static_cast<char>(0x80 | (code & 0x3F));
                    }
                    break;
                }
                default: fail("bad escape");
            }
        }
        fail("unterminated string");
    }

    void parse_number(Json& value) {
        const char* start = text_.c_str() + pos_;
        char* end = nullptr;
        double number = std::strtod(start, &end);
        if (end == start) { fail("unexpected character"); }
        pos_ += static_cast<std::size_t>(end - start);
        value.type_ = Json::Type::Number;
        value.number_ = number;
    }
};

Json Json::parse(const std::string& text) {
    return JsonParser(text).parse_document();
}

const Json* Json::find(const std::string& key) const {
    if (type_ != Type::Object) { return nullptr; }
    for (std::size_t i = 0; i < keys_.size(); ++i) {
        if (keys_[i] == key) { return &values_[i]; }
    }
    return nullptr;
}

double Json::number(const std::string& key, double fallback) const {
    const Json* v = find(key);
    return (v && v->type_ == Type::Number) ? v->number_ : fallback;
}

bool Json::boolean(const std::string& key, bool fallback) const {
    const Json* v = find(key);
    return (v && v->type_ == Type::Bool) ? v->bool_ : fallback;
}

std::string Json::string(const std::string& key, const std::string& fallback) const {
    const Json* v = find(key);
    return (v && v->type_ == Type::String) ? v->string_ : fallback;
}

}  // namespace market
