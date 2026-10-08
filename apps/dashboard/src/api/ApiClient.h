#pragma once
#include <nlohmann/json.hpp>
#include <atomic>
#include <mutex>
#include <string>

struct ApiResponse
{
    long statusCode = 0;
    bool success = false;
    nlohmann::json data;
    std::string error;
};

// Thin libcurl wrapper. All methods may be called from several threads.
class ApiClient
{
public:
    explicit ApiClient(const std::string& baseUrl);

    ApiResponse Get(const std::string& endpoint);
    ApiResponse Post(const std::string& endpoint, const nlohmann::json& body);
    ApiResponse Put(const std::string& endpoint, const nlohmann::json& body);
    ApiResponse Delete(const std::string& endpoint);

    void SetToken(const std::string& token);
    void ClearToken();

    const std::string& BaseUrl() const { return baseUrl; }

    // Logs "METHOD url" to stderr. Request bodies are never logged.
    void SetDebug(bool enabled) { debug = enabled; }

private:
    ApiResponse Request(
        const std::string& method,
        const std::string& endpoint,
        const std::string& body = "");

    std::string baseUrl;
    std::mutex tokenMutex;
    std::string token;
    std::atomic<bool> debug{false};
};
