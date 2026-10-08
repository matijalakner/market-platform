#include "MainWindow.h"
#include "JsonView.h"
#include "auth/AuthManager.h"

#include <imgui.h>
#include <algorithm>
#include <cctype>
#include <chrono>

namespace
{
constexpr const char* kWindowTitle = "Market Dashboard";
constexpr float kRawJsonPanelHeight = 260.0f;
const ImVec4 kErrorColor(1.0f, 0.35f, 0.35f, 1.0f);

bool IsReady(const std::future<ApiResponse>& future)
{
    using namespace std::chrono_literals;
    return future.valid() &&
           future.wait_for(0s) == std::future_status::ready;
}

// Only characters that are safe inside a URL path segment.
std::string SanitizeSymbol(const char* raw)
{
    std::string out;
    for (const char* p = raw; *p && out.size() < 12; ++p)
    {
        const unsigned char c = static_cast<unsigned char>(*p);
        if (std::isalnum(c) || c == '.' || c == '-')
            out.push_back(static_cast<char>(std::toupper(c)));
    }
    return out;
}

bool IsSafeName(const std::string& name)
{
    if (name.empty() || name.size() > 64)
        return false;
    return std::all_of(name.begin(), name.end(), [](unsigned char c) {
        return std::isalnum(c) || c == '_' || c == '-';
    });
}
}

MainWindow::MainWindow(ApiClient& api, AuthManager& auth)
    : api(api), auth(auth)
{
}

void MainWindow::ResetState()
{
    ++generation;
    chart.Clear();
    error.clear();
    rawJson.clear();
    dataSource.clear();
    simulations.clear();
    selectedSimulation = -1;
    pricesRefreshQueued = false;
    needsInitialLoad = true;
}

void MainWindow::Logout()
{
    auth.Logout();
    ResetState();
}

void MainWindow::HandleUnauthorized()
{
    auth.Logout("Your session has expired. Please sign in again.");
    ResetState();
}

void MainWindow::RequestPrices()
{
    // One request at a time. If one is in flight (possibly from a previous
    // session), run again as soon as it finishes.
    if (pricesRequest.future.valid())
    {
        pricesRefreshQueued = true;
        return;
    }

    std::string endpoint;

    if (source == Source::Stock)
    {
        const std::string requested = SanitizeSymbol(symbol);
        if (requested.empty())
        {
            error = "Enter a stock symbol.";
            return;
        }
        endpoint = "/stocks/" + requested + "/prices";
    }
    else
    {
        if (selectedSimulation < 0 ||
            selectedSimulation >= static_cast<int>(simulations.size()))
        {
            error = "No simulation run selected. Run market_simulation first, "
                    "then use \"Reload list\".";
            return;
        }
        endpoint = "/simulations/" + simulations[selectedSimulation] +
                   "/candles?interval=" + std::to_string(candleSteps);
    }

    error.clear();
    pricesRequest.generation = generation;

    ApiClient* client = &api;
    pricesRequest.future = std::async(
        std::launch::async,
        [client, endpoint] { return client->Get(endpoint); });
}

void MainWindow::RequestSimulations()
{
    if (simulationsRequest.future.valid())
        return;

    simulationsRequest.generation = generation;

    ApiClient* client = &api;
    simulationsRequest.future = std::async(
        std::launch::async,
        [client] { return client->Get("/simulations"); });
}

void MainWindow::PollRequests()
{
    if (IsReady(pricesRequest.future))
    {
        const std::uint64_t requestGeneration = pricesRequest.generation;
        const ApiResponse response = pricesRequest.future.get();

        if (requestGeneration == generation)
            HandlePrices(response);

        if (pricesRefreshQueued)
        {
            pricesRefreshQueued = false;
            RequestPrices();
        }
    }

    if (IsReady(simulationsRequest.future))
    {
        const std::uint64_t requestGeneration = simulationsRequest.generation;
        const ApiResponse response = simulationsRequest.future.get();

        if (requestGeneration == generation)
            HandleSimulations(response);
    }
}

void MainWindow::HandlePrices(const ApiResponse& response)
{
    if (!response.success)
    {
        if (response.statusCode == 401)
        {
            HandleUnauthorized();
            return;
        }

        error = response.error.empty() ? "Failed to load stock data." : response.error;
        chart.Clear();
        rawJson.clear();
        return;
    }

    if (chart.SetData(response.data) == 0)
    {
        error = "The API returned no valid candles.";
        rawJson.clear();
        return;
    }

    dataSource = "demo";
    if (response.data.contains("source") && response.data["source"].is_string())
        dataSource = response.data["source"].get<std::string>();

    rawJson = response.data.dump(2);
    error.clear();
}

void MainWindow::HandleSimulations(const ApiResponse& response)
{
    if (!response.success)
    {
        if (response.statusCode == 401)
        {
            HandleUnauthorized();
            return;
        }
        // Not fatal: the stock view still works.
        if (source == Source::Simulation)
            error = response.error;
        return;
    }

    const std::string previous =
        (selectedSimulation >= 0 &&
         selectedSimulation < static_cast<int>(simulations.size()))
            ? simulations[selectedSimulation]
            : std::string();

    simulations.clear();
    if (response.data.is_object() &&
        response.data.contains("runs") &&
        response.data["runs"].is_array())
    {
        for (const auto& item : response.data["runs"])
        {
            if (item.is_string() && IsSafeName(item.get<std::string>()))
                simulations.push_back(item.get<std::string>());
        }
    }

    selectedSimulation = -1;
    for (std::size_t i = 0; i < simulations.size(); ++i)
    {
        if (simulations[i] == previous)
            selectedSimulation = static_cast<int>(i);
    }
    if (selectedSimulation < 0 && !simulations.empty())
        selectedSimulation = 0;

    if (source == Source::Simulation)
        RequestPrices();
}

void MainWindow::Render()
{
    if (needsInitialLoad)
    {
        needsInitialLoad = false;
        RequestPrices();
        RequestSimulations();
    }

    PollRequests();

    // A 401 reply may have logged us out just now.
    if (!auth.IsAuthenticated())
        return;

    ImGui::Begin(kWindowTitle);

    ImGui::Text("Welcome, %s", auth.GetUsername().c_str());
    ImGui::SameLine();

    if (ImGui::Button("Logout"))
    {
        Logout();
        ImGui::End();
        return;
    }

    ImGui::Separator();

    bool reload = false;

    if (ImGui::RadioButton("Stocks", source == Source::Stock) &&
        source != Source::Stock)
    {
        source = Source::Stock;
        reload = true;
    }
    ImGui::SameLine();
    if (ImGui::RadioButton("Simulation runs", source == Source::Simulation) &&
        source != Source::Simulation)
    {
        source = Source::Simulation;
        reload = true;
    }

    if (source == Source::Stock)
    {
        ImGui::Text("Stock symbol");
        ImGui::SameLine();
        ImGui::SetNextItemWidth(120.0f);

        if (ImGui::InputText(
                "##symbol", symbol, sizeof(symbol),
                ImGuiInputTextFlags_CharsUppercase |
                ImGuiInputTextFlags_EnterReturnsTrue))
            reload = true;
    }
    else
    {
        const char* preview =
            (selectedSimulation >= 0 &&
             selectedSimulation < static_cast<int>(simulations.size()))
                ? simulations[selectedSimulation].c_str()
                : "(none)";

        ImGui::Text("Run");
        ImGui::SameLine();
        ImGui::SetNextItemWidth(220.0f);

        if (ImGui::BeginCombo("##run", preview))
        {
            for (int i = 0; i < static_cast<int>(simulations.size()); ++i)
            {
                const bool selected = (i == selectedSimulation);
                if (ImGui::Selectable(simulations[i].c_str(), selected))
                {
                    selectedSimulation = i;
                    reload = true;
                }
                if (selected)
                    ImGui::SetItemDefaultFocus();
            }
            ImGui::EndCombo();
        }

        ImGui::SameLine();
        if (ImGui::Button("Reload list"))
            RequestSimulations();

        ImGui::SameLine();
        ImGui::SetNextItemWidth(130.0f);
        if (ImGui::InputInt("steps / candle", &candleSteps))
            candleSteps = std::clamp(candleSteps, 1, 10000);
        if (ImGui::IsItemDeactivatedAfterEdit())
            reload = true;
    }

    ImGui::SameLine();

    const bool loading = pricesRequest.future.valid();

    ImGui::BeginDisabled(loading);
    if (ImGui::Button("Refresh"))
        reload = true;
    ImGui::EndDisabled();

    if (loading)
    {
        ImGui::SameLine();
        ImGui::TextDisabled("Loading...");
    }

    if (reload)
        RequestPrices();

    if (!error.empty())
    {
        ImGui::Spacing();
        ImGui::TextColored(kErrorColor, "%s", error.c_str());
        ImGui::Spacing();
    }

    if (chart.Size() > 0)
    {
        ImGui::Text(
            "%s | %zu candles | %s data",
            chart.Symbol().c_str(),
            chart.Size(),
            dataSource.c_str());
        ImGui::SameLine();
        ImGui::Checkbox("Show raw JSON", &showRawJson);
    }

    ImGui::Spacing();

    if (showRawJson && !rawJson.empty())
    {
        chart.Render(-kRawJsonPanelHeight);
        JsonView::Render(rawJson);
    }
    else
    {
        chart.Render(0.0f);
    }

    ImGui::End();
}
