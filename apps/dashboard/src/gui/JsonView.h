#pragma once
#include <string>

// Scrollable read-only text box, used to show the raw API response.
class JsonView
{
public:
    static void Render(const std::string& text);
};
