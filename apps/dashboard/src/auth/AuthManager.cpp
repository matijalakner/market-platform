#include "AuthManager.h"
#include <chrono>

AuthManager::AuthManager(ApiClient& api) : api(api) {}

void AuthManager::BeginLogin(
    const std::string& requestedUsername,
    const std::string& password)
{
    if (pendingLogin.valid() || authenticated)
        return;

    lastError.clear();
    pendingUsername = requestedUsername;

    ApiClient* client = &api;
    pendingLogin = std::async(
        std::launch::async,
        [client, requestedUsername, password] {
            return client->Post(
                "/login",
                nlohmann::json{
                    {"username", requestedUsername},
                    {"password", password}});
        });
}

void AuthManager::Poll()
{
    using namespace std::chrono_literals;

    if (!pendingLogin.valid() ||
        pendingLogin.wait_for(0s) != std::future_status::ready)
        return;

    const ApiResponse response = pendingLogin.get();

    if (!response.success)
    {
        // Shows the server's message (wrong password, too many attempts, ...)
        // or the network error.
        lastError = response.error.empty() ? "Login failed." : response.error;
        return;
    }

    if (!response.data.is_object() ||
        !response.data.contains("token") ||
        !response.data["token"].is_string())
    {
        lastError = "The server returned an invalid login response.";
        return;
    }

    api.SetToken(response.data["token"].get<std::string>());
    username = pendingUsername;
    authenticated = true;
    lastError.clear();
}

void AuthManager::Logout(const std::string& reason)
{
    username.clear();
    authenticated = false;
    api.ClearToken();
    lastError = reason;
}
