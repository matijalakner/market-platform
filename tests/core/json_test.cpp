#include <cassert>
#include <stdexcept>

#include "market/core/json.hpp"

int main() {
    market::Json j = market::Json::parse(R"({
        "name": "test \"quoted\"",
        "n": -12.5e1,
        "ok": true,
        "nothing": null,
        "list": [1, 2, {"x": 3}],
        "nested": {"a": {"b": 7}}
    })");

    assert(j.is_object());
    assert(j.string("name", "") == "test \"quoted\"");
    assert(j.number("n", 0.0) == -125.0);
    assert(j.boolean("ok", false));
    assert(j.number("missing", 42.0) == 42.0);
    assert(j.number("name", 1.0) == 1.0);              // wrong type -> fallback

    const market::Json* list = j.find("list");
    assert(list != nullptr && list->is_array());
    assert(list->items().size() == 3);
    assert(list->items()[2].number("x", 0.0) == 3.0);
    assert(j.find("nested")->find("a")->number("b", 0.0) == 7.0);

    bool threw = false;
    try { market::Json::parse("{\"a\": }"); } catch (const std::runtime_error&) { threw = true; }
    assert(threw);

    threw = false;
    try { market::Json::parse("{\"a\": 1} trailing"); } catch (const std::runtime_error&) { threw = true; }
    assert(threw);

    assert(market::Json::parse("[]").items().empty());
    assert(market::Json::parse("{}").is_object());

    return 0;
}
