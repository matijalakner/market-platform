#include "LoginWindow.h"
#include "auth/AuthManager.h"
#include <imgui.h>
#include <cstddef>
#include <cstring>

namespace
{
// A plain memset on a buffer that is never read again may be optimised away.
void Wipe(char* data, std::size_t size)
{
    volatile char* p = data;
    while (size--)
        *p++ = 0;
}
}

LoginWindow::LoginWindow(AuthManager& auth) : auth(auth)
{
#ifdef DASHBOARD_DEV_HINTS
    std::strncpy(username, "admin", sizeof(username) - 1);
#endif
}

void LoginWindow::Render()
{
    const ImGuiViewport* viewport = ImGui::GetMainViewport();

    ImGui::SetNextWindowPos(viewport->GetCenter(), ImGuiCond_Always, ImVec2(0.5f, 0.5f));
    ImGui::SetNextWindowSize(ImVec2(430.0f, 360.0f), ImGuiCond_Always);

    ImGui::Begin(
        "Market Dashboard",
        nullptr,
        ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoCollapse);

    ImGui::Spacing();
    ImGui::Text("Market Dashboard");
    ImGui::TextDisabled("Sign in to continue");
    ImGui::Spacing();
    ImGui::Separator();
    ImGui::Spacing();

    const bool busy = auth.IsLoginInProgress();
    bool submit = false;

    ImGui::BeginDisabled(busy);

    ImGui::Text("Username");
    ImGui::SetNextItemWidth(-1.0f);
    ImGui::InputText("##username", username, sizeof(username));

    ImGui::Spacing();
    ImGui::Text("Password");
    ImGui::SetNextItemWidth(-1.0f);

    const ImGuiInputTextFlags flags =
        (showPassword ? 0 : ImGuiInputTextFlags_Password) |
        ImGuiInputTextFlags_EnterReturnsTrue;

    if (ImGui::InputText("##password", password, sizeof(password), flags))
        submit = true;

    ImGui::Checkbox("Show password", &showPassword);
    ImGui::Spacing();

    if (ImGui::Button(busy ? "Signing in..." : "Login", ImVec2(-1.0f, 42.0f)))
        submit = true;

    ImGui::EndDisabled();

    if (submit && !busy && username[0] != '\0')
    {
        auth.BeginLogin(username, password);
        Wipe(password, sizeof(password));  // the request holds its own copy
    }

    if (!auth.LastError().empty())
    {
        ImGui::Spacing();
        ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.0f, 0.35f, 0.35f, 1.0f));
        ImGui::TextWrapped("%s", auth.LastError().c_str());
        ImGui::PopStyleColor();
    }

#ifdef DASHBOARD_DEV_HINTS
    ImGui::Spacing();
    ImGui::TextDisabled("Development account: admin / password");
#endif

    ImGui::End();
}
