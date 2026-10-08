#include "JsonView.h"
#include <imgui.h>

void JsonView::Render(const std::string& text)
{
    ImGui::BeginChild(
        "JsonView",
        ImVec2(0, 250),
        ImGuiChildFlags_Borders,
        ImGuiWindowFlags_HorizontalScrollbar);
    ImGui::TextUnformatted(text.c_str(), text.c_str() + text.size());
    ImGui::EndChild();
}
