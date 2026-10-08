#pragma once
#include "api/ApiClient.h"
#include <future>
#include <string>

// Owns the login state. Login runs on a worker thread so the UI never
// freezes; call Poll() once per frame to pick up the result.
class AuthManager
{
public:
    explicit AuthManager(ApiClient& api);

    void BeginLogin(const std::string& username, const std::string& password);
    void Poll();

    // `reason` is shown on the login screen (e.g. "session expired").
    void Logout(const std::string& reason = {});

    bool IsLoginInProgress() const { return pendingLogin.valid(); }
    bool IsAuthenticated() const { return authenticated; }
    const std::string& GetUsername() const { return username; }
    const std::string& LastError() const { return lastError; }

private:
    ApiClient& api;
    std::future<ApiResponse> pendingLogin;
    std::string pendingUsername;
    std::string username;
    std::string lastError;
    bool authenticated = false;
};
