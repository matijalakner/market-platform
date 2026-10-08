#pragma once
#include "StockPriceChart.h"
#include "api/ApiClient.h"
#include <cstdint>
#include <future>
#include <string>
#include <vector>

class AuthManager;

// Dashboard shown after login. All network calls run on worker threads;
// Render() only starts requests and collects finished ones, so the UI never
// blocks.
class MainWindow
{
public:
    MainWindow(ApiClient& api, AuthManager& auth);
    void Render();

private:
    enum class Source { Stock, Simulation };

    struct Pending
    {
        std::future<ApiResponse> future;
        std::uint64_t generation = 0;  // session the request belongs to
    };

    void RequestPrices();
    void RequestSimulations();
    void PollRequests();
    void HandlePrices(const ApiResponse& response);
    void HandleSimulations(const ApiResponse& response);
    void HandleUnauthorized();
    void Logout();
    void ResetState();

    ApiClient& api;
    AuthManager& auth;

    Source source = Source::Stock;
    char symbol[32] = "AAPL";
    std::vector<std::string> simulations;
    int selectedSimulation = -1;
    int candleSteps = 10;

    StockPriceChart chart;
    std::string error;
    std::string rawJson;
    std::string dataSource;
    bool showRawJson = false;

    bool needsInitialLoad = true;
    bool pricesRefreshQueued = false;
    std::uint64_t generation = 0;  // bumped on logout; stale replies are dropped
    Pending pricesRequest;
    Pending simulationsRequest;
};
