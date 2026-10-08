#pragma once
#include <nlohmann/json.hpp>
#include <cstddef>
#include <string>
#include <vector>

// Candlestick chart with a volume pane and a hover tooltip.
// The JSON is parsed once in SetData(), not on every frame.
class StockPriceChart
{
public:
    // Accepts {"symbol": "...", "prices": [{timestamp, open, high, low,
    // close, volume?}, ...]}. Invalid candles are skipped.
    // Returns the number of valid candles.
    std::size_t SetData(const nlohmann::json& data);
    void Clear();

    // `height` follows ImGui::BeginChild: 0 = fill the window, <0 = fill
    // minus that many pixels.
    void Render(float height = 0.0f) const;

    const std::string& Symbol() const { return symbol; }
    std::size_t Size() const { return candles.size(); }

private:
    struct Candle
    {
        std::string timestamp;
        double open = 0;
        double high = 0;
        double low = 0;
        double close = 0;
        double volume = 0;
    };

    std::vector<Candle> candles;
    std::string symbol = "UNKNOWN";
};
