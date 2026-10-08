#include "Application.h"
#include "api/ApiClient.h"
#include "auth/AuthManager.h"
#include "gui/LoginWindow.h"
#include "gui/MainWindow.h"

#include <imgui.h>
#include <backends/imgui_impl_glfw.h>
#include <backends/imgui_impl_opengl3.h>

#include <chrono>
#include <cstdlib>
#include <iostream>
#include <thread>

// glfw3.h (included by Application.h) pulls in the platform's OpenGL header,
// which provides glViewport/glClear on Linux, macOS and Windows.

namespace
{
constexpr const char* kWindowTitle = "Market Dashboard";
constexpr const char* kDefaultApiUrl = "http://127.0.0.1:8000";

void GlfwErrorCallback(int code, const char* description)
{
    std::cerr << "GLFW error " << code << ": " << description << "\n";
}

std::string ApiUrlFromEnvironment()
{
    const char* value = std::getenv("MARKET_API_URL");
    return (value && *value) ? value : kDefaultApiUrl;
}
}

Application::Application() = default;

Application::~Application()
{
    Shutdown();
}

bool Application::Initialize()
{
    glfwSetErrorCallback(GlfwErrorCallback);

    if (!glfwInit())
    {
        std::cerr << "Failed to initialize GLFW\n";
        return false;
    }
    glfwReady = true;

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
#ifdef __APPLE__
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GLFW_TRUE);
#endif

    window = glfwCreateWindow(1280, 800, kWindowTitle, nullptr, nullptr);
    if (!window)
    {
        std::cerr << "Failed to create GLFW window\n";
        Shutdown();
        return false;
    }

    glfwMakeContextCurrent(window);
    glfwSwapInterval(1);

    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    imguiReady = true;
    ImGui::GetIO().ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
    ImGui::StyleColorsDark();

    if (!ImGui_ImplGlfw_InitForOpenGL(window, true))
    {
        Shutdown();
        return false;
    }
    glfwBackendReady = true;

    if (!ImGui_ImplOpenGL3_Init("#version 330"))
    {
        Shutdown();
        return false;
    }
    openglBackendReady = true;

    const std::string apiUrl = ApiUrlFromEnvironment();
    std::cerr << "Using API at " << apiUrl << "\n";

    apiClient = std::make_unique<ApiClient>(apiUrl);
    apiClient->SetDebug(std::getenv("MARKET_API_DEBUG") != nullptr);
    authManager = std::make_unique<AuthManager>(*apiClient);
    loginWindow = std::make_unique<LoginWindow>(*authManager);
    mainWindow = std::make_unique<MainWindow>(*apiClient, *authManager);

    return true;
}

void Application::Run()
{
    while (!glfwWindowShouldClose(window))
    {
        glfwPollEvents();

        if (glfwGetWindowAttrib(window, GLFW_ICONIFIED))
        {
            std::this_thread::sleep_for(std::chrono::milliseconds(50));
            continue;
        }

        authManager->Poll();

        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

        if (authManager->IsAuthenticated())
            mainWindow->Render();
        else
            loginWindow->Render();

        ImGui::Render();

        int w = 0;
        int h = 0;
        glfwGetFramebufferSize(window, &w, &h);

        glViewport(0, 0, w, h);
        glClearColor(0.055f, 0.065f, 0.080f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);

        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
        glfwSwapBuffers(window);
    }
}

void Application::Shutdown()
{
    if (openglBackendReady)
        ImGui_ImplOpenGL3_Shutdown();
    if (glfwBackendReady)
        ImGui_ImplGlfw_Shutdown();
    if (imguiReady)
        ImGui::DestroyContext();
    if (window)
        glfwDestroyWindow(window);
    if (glfwReady)
        glfwTerminate();

    openglBackendReady = glfwBackendReady = imguiReady = glfwReady = false;
    window = nullptr;
}
