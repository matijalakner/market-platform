#include "ApiClient.h"
#include <curl/curl.h>
#include <iostream>
#include <memory>
#include <mutex>

namespace
{
size_t WriteCallback(void* contents, size_t size, size_t nmemb, void* userp)
{
    const size_t total = size * nmemb;
    static_cast<std::string*>(userp)->append(
        static_cast<char*>(contents), total);
    return total;
}

void EnsureCurlInitialized()
{
    static std::once_flag flag;
    std::call_once(flag, [] {
        curl_global_init(CURL_GLOBAL_DEFAULT);
    });
}

struct CurlDeleter
{
    void operator()(CURL* handle) const { curl_easy_cleanup(handle); }
};

struct HeaderListDeleter
{
    void operator()(curl_slist* list) const { curl_slist_free_all(list); }
};

// FastAPI sends "detail" as a string for HTTPException and as an array of
// objects for validation errors (422).
std::string ExtractError(const nlohmann::json& data, long statusCode)
{
    if (data.is_object() && data.contains("detail"))
    {
        const auto& detail = data["detail"];
        if (detail.is_string())
            return detail.get<std::string>();

        if (detail.is_array() && !detail.empty() &&
            detail[0].is_object() && detail[0].contains("msg") &&
            detail[0]["msg"].is_string())
        {
            return detail[0]["msg"].get<std::string>();
        }
    }
    return "HTTP error " + std::to_string(statusCode);
}
}

ApiClient::ApiClient(const std::string& url) : baseUrl(url)
{
    EnsureCurlInitialized();
    while (!baseUrl.empty() && baseUrl.back() == '/')
        baseUrl.pop_back();
}

void ApiClient::SetToken(const std::string& value)
{
    std::lock_guard<std::mutex> lock(tokenMutex);
    token = value;
}

void ApiClient::ClearToken()
{
    std::lock_guard<std::mutex> lock(tokenMutex);
    token.clear();
}

ApiResponse ApiClient::Get(const std::string& endpoint)
{
    return Request("GET", endpoint);
}

ApiResponse ApiClient::Post(const std::string& endpoint, const nlohmann::json& body)
{
    return Request("POST", endpoint, body.dump());
}

ApiResponse ApiClient::Put(const std::string& endpoint, const nlohmann::json& body)
{
    return Request("PUT", endpoint, body.dump());
}

ApiResponse ApiClient::Delete(const std::string& endpoint)
{
    return Request("DELETE", endpoint);
}

ApiResponse ApiClient::Request(
    const std::string& method,
    const std::string& endpoint,
    const std::string& body)
{
    ApiResponse response;

    std::unique_ptr<CURL, CurlDeleter> curl(curl_easy_init());
    if (!curl)
    {
        response.error = "Failed to initialize libcurl";
        return response;
    }

    std::string currentToken;
    {
        std::lock_guard<std::mutex> lock(tokenMutex);
        currentToken = token;
    }

    const std::string url = baseUrl + endpoint;
    std::string responseBody;
    char errorBuffer[CURL_ERROR_SIZE] = {};

    curl_easy_setopt(curl.get(), CURLOPT_URL, url.c_str());
    curl_easy_setopt(curl.get(), CURLOPT_CUSTOMREQUEST, method.c_str());
    curl_easy_setopt(curl.get(), CURLOPT_WRITEFUNCTION, WriteCallback);
    curl_easy_setopt(curl.get(), CURLOPT_WRITEDATA, &responseBody);
    curl_easy_setopt(curl.get(), CURLOPT_ERRORBUFFER, errorBuffer);
    // Required when libcurl is used from several threads with timeouts.
    curl_easy_setopt(curl.get(), CURLOPT_NOSIGNAL, 1L);
    curl_easy_setopt(curl.get(), CURLOPT_CONNECTTIMEOUT, 3L);
    curl_easy_setopt(curl.get(), CURLOPT_TIMEOUT, 10L);
    // The API never redirects; following redirects could send the bearer
    // token to another host.
    curl_easy_setopt(curl.get(), CURLOPT_FOLLOWLOCATION, 0L);

    curl_slist* rawHeaders = nullptr;
    rawHeaders = curl_slist_append(rawHeaders, "Accept: application/json");
    if (!body.empty())
        rawHeaders = curl_slist_append(rawHeaders, "Content-Type: application/json");
    if (!currentToken.empty())
    {
        const std::string header = "Authorization: Bearer " + currentToken;
        rawHeaders = curl_slist_append(rawHeaders, header.c_str());
    }
    std::unique_ptr<curl_slist, HeaderListDeleter> headers(rawHeaders);
    curl_easy_setopt(curl.get(), CURLOPT_HTTPHEADER, headers.get());

    if (!body.empty())
    {
        curl_easy_setopt(curl.get(), CURLOPT_POSTFIELDS, body.c_str());
        curl_easy_setopt(curl.get(), CURLOPT_POSTFIELDSIZE_LARGE,
                         static_cast<curl_off_t>(body.size()));
    }

    if (debug)
        std::cerr << method << " " << url << "\n";

    const CURLcode result = curl_easy_perform(curl.get());
    if (result != CURLE_OK)
    {
        const char* reason = errorBuffer[0] ? errorBuffer : curl_easy_strerror(result);
        response.error = "Cannot reach the API (" + baseUrl + "): " + reason;
        return response;
    }

    curl_easy_getinfo(curl.get(), CURLINFO_RESPONSE_CODE, &response.statusCode);
    response.success = response.statusCode >= 200 && response.statusCode < 300;

    if (responseBody.empty())
    {
        response.data = nlohmann::json::object();
    }
    else
    {
        // allow_exceptions = false: a bad body yields a "discarded" value.
        response.data = nlohmann::json::parse(responseBody, nullptr, false);
        if (response.data.is_discarded())
        {
            response.data = nlohmann::json::object();
            response.success = false;
            response.error = response.statusCode >= 400
                ? "HTTP error " + std::to_string(response.statusCode)
                : "Server returned invalid JSON";
            return response;
        }
    }

    if (!response.success)
        response.error = ExtractError(response.data, response.statusCode);

    return response;
}
