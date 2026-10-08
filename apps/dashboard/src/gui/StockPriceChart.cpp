#include "StockPriceChart.h"
#include <imgui.h>
#include <algorithm>
#include <cmath>
#include <cstdio>

namespace
{
bool Number(const nlohmann::json& j, const char* key, double& value)
{
    if (!j.is_object() || !j.contains(key) || !j[key].is_number())
        return false;
    value = j[key].get<double>();
    return std::isfinite(value);
}

std::string ShortLabel(const std::string& timestamp)
{
    // "2026-10-07T09:30:00" -> "09:30"; anything else is shown as is.
    if (timestamp.size() >= 16 && (timestamp[10] == 'T' || timestamp[10] == ' '))
        return timestamp.substr(11, 5);
    return timestamp;
}
}

void StockPriceChart::Clear()
{
    candles.clear();
    symbol = "UNKNOWN";
}

std::size_t StockPriceChart::SetData(const nlohmann::json& data)
{
    Clear();

    if (!data.is_object() ||
        !data.contains("prices") ||
        !data["prices"].is_array())
        return 0;

    if (data.contains("symbol") && data["symbol"].is_string())
        symbol = data["symbol"].get<std::string>();

    for (const auto& item : data["prices"])
    {
        if (!item.is_object() ||
            !item.contains("timestamp") ||
            !item["timestamp"].is_string())
            continue;

        Candle c;
        c.timestamp = item["timestamp"].get<std::string>();

        if (!Number(item, "open", c.open) ||
            !Number(item, "high", c.high) ||
            !Number(item, "low", c.low) ||
            !Number(item, "close", c.close))
            continue;

        double volume = 0;
        if (Number(item, "volume", volume) && volume > 0)
            c.volume = volume;

        // Make the candle self-consistent instead of drawing nonsense.
        c.high = std::max({c.high, c.low, c.open, c.close});
        c.low = std::min({c.high, c.low, c.open, c.close});

        candles.push_back(std::move(c));
    }

    return candles.size();
}

void StockPriceChart::Render(float height) const
{
    ImGui::BeginChild(
        "StockChart",
        ImVec2(0.0f, height),
        ImGuiChildFlags_Borders,
        ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse);

    if (candles.empty())
    {
        ImGui::TextDisabled("No stock data available.");
        ImGui::EndChild();
        return;
    }

    const ImVec2 avail = ImGui::GetContentRegionAvail();
    const float width = std::max(240.0f, avail.x);
    const float totalHeight = std::max(220.0f, avail.y);

    constexpr float left = 70.0f;
    constexpr float right = 25.0f;
    constexpr float top = 30.0f;
    constexpr float bottom = 30.0f;
    constexpr float volumeFraction = 0.18f;
    constexpr float paneGap = 8.0f;

    const ImVec2 origin = ImGui::GetCursorScreenPos();
    const ImVec2 plotMin(origin.x + left, origin.y + top);
    const ImVec2 plotMax(origin.x + width - right, origin.y + totalHeight - bottom);

    const float plotWidth = plotMax.x - plotMin.x;
    const float plotHeight = plotMax.y - plotMin.y;
    const float volumeTop = plotMax.y - plotHeight * volumeFraction;
    const float priceBottom = volumeTop - paneGap;
    const float priceHeight = priceBottom - plotMin.y;

    double minPrice = candles.front().low;
    double maxPrice = candles.front().high;
    double maxVolume = 0.0;

    for (const auto& c : candles)
    {
        minPrice = std::min(minPrice, c.low);
        maxPrice = std::max(maxPrice, c.high);
        maxVolume = std::max(maxVolume, c.volume);
    }

    if (std::abs(maxPrice - minPrice) < 1e-9)
    {
        minPrice -= 1;
        maxPrice += 1;
    }

    const double padding = (maxPrice - minPrice) * 0.08;
    minPrice -= padding;
    maxPrice += padding;

    auto priceY = [&](double price)
    {
        const double normalized = (price - minPrice) / (maxPrice - minPrice);
        return priceBottom - static_cast<float>(normalized * priceHeight);
    };

    ImDrawList* draw = ImGui::GetWindowDrawList();

    const ImU32 upColor = IM_COL32(80, 210, 140, 255);
    const ImU32 downColor = IM_COL32(235, 90, 100, 255);
    const ImU32 upVolume = IM_COL32(80, 210, 140, 110);
    const ImU32 downVolume = IM_COL32(235, 90, 100, 110);
    const ImU32 textColor = IM_COL32(200, 205, 215, 255);

    draw->AddRectFilled(plotMin, plotMax, IM_COL32(17, 22, 29, 255));

    constexpr int gridLines = 6;
    for (int i = 0; i <= gridLines; ++i)
    {
        const float y = plotMin.y + priceHeight * static_cast<float>(i) / gridLines;
        draw->AddLine(
            ImVec2(plotMin.x, y), ImVec2(plotMax.x, y),
            IM_COL32(55, 65, 78, 150));

        const double price = maxPrice - (maxPrice - minPrice) * i / gridLines;
        char label[64];
        std::snprintf(label, sizeof(label), "%.2f", price);
        draw->AddText(ImVec2(origin.x + 8, y - 8), textColor, label);
    }

    const float slot = plotWidth / static_cast<float>(candles.size());
    const float candleWidth = std::max(2.0f, std::min(22.0f, slot * 0.62f));

    for (std::size_t i = 0; i < candles.size(); ++i)
    {
        const Candle& c = candles[i];
        const float x = plotMin.x + slot * (static_cast<float>(i) + 0.5f);
        const bool up = c.close >= c.open;

        const float yOpen = priceY(c.open);
        const float yClose = priceY(c.close);
        const float topBody = std::min(yOpen, yClose);
        const float bottomBody = std::max(yOpen, yClose);

        draw->AddLine(
            ImVec2(x, priceY(c.high)), ImVec2(x, priceY(c.low)),
            up ? upColor : downColor, 1.5f);
        draw->AddRectFilled(
            ImVec2(x - candleWidth * 0.5f, topBody),
            ImVec2(x + candleWidth * 0.5f, std::max(topBody + 2.0f, bottomBody)),
            up ? upColor : downColor);

        if (maxVolume > 0.0 && c.volume > 0.0)
        {
            const float barHeight =
                static_cast<float>(c.volume / maxVolume) * (plotMax.y - volumeTop);
            draw->AddRectFilled(
                ImVec2(x - candleWidth * 0.5f, plotMax.y - barHeight),
                ImVec2(x + candleWidth * 0.5f, plotMax.y),
                up ? upVolume : downVolume);
        }
    }

    draw->AddRect(plotMin, plotMax, IM_COL32(100, 110, 125, 255));

    const std::size_t every = std::max<std::size_t>(1, candles.size() / 6);
    for (std::size_t i = 0; i < candles.size(); i += every)
    {
        const float x = plotMin.x + slot * (static_cast<float>(i) + 0.5f);
        const std::string label = ShortLabel(candles[i].timestamp);
        const float labelWidth = ImGui::CalcTextSize(label.c_str()).x;
        draw->AddText(
            ImVec2(x - labelWidth * 0.5f, plotMax.y + 6.0f),
            textColor, label.c_str());
    }

    draw->AddText(
        ImVec2(plotMin.x, origin.y + 5.0f),
        IM_COL32(235, 235, 240, 255),
        symbol.c_str());

    const ImVec2 mouse = ImGui::GetMousePos();
    if (ImGui::IsWindowHovered() &&
        mouse.x >= plotMin.x && mouse.x <= plotMax.x &&
        mouse.y >= plotMin.y && mouse.y <= plotMax.y)
    {
        const float position = std::clamp(
            (mouse.x - plotMin.x) / slot,
            0.0f, static_cast<float>(candles.size() - 1));
        const std::size_t index = static_cast<std::size_t>(position);
        const Candle& c = candles[index];

        const float x = plotMin.x + slot * (static_cast<float>(index) + 0.5f);
        draw->AddLine(
            ImVec2(x, plotMin.y), ImVec2(x, plotMax.y),
            IM_COL32(200, 205, 215, 90));

        ImGui::BeginTooltip();
        ImGui::TextUnformatted(c.timestamp.c_str());
        ImGui::Text("Open   %.2f", c.open);
        ImGui::Text("High   %.2f", c.high);
        ImGui::Text("Low    %.2f", c.low);
        ImGui::Text("Close  %.2f", c.close);
        ImGui::Text("Volume %.0f", c.volume);
        ImGui::EndTooltip();
    }

    ImGui::Dummy(ImVec2(width, totalHeight));
    ImGui::EndChild();
}
