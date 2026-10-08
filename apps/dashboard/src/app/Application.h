#pragma once
#include <memory>
#include <GLFW/glfw3.h>

class ApiClient;
class AuthManager;
class LoginWindow;
class MainWindow;

class Application
{
public:
    Application();
    ~Application();

    Application(const Application&) = delete;
    Application& operator=(const Application&) = delete;

    bool Initialize();
    void Run();

private:
    void Shutdown();

    GLFWwindow* window = nullptr;

    // Tracks what was actually initialised, so a failed Initialize() cleans
    // up exactly that and nothing more.
    bool glfwReady = false;
    bool imguiReady = false;
    bool glfwBackendReady = false;
    bool openglBackendReady = false;

    // Declaration order matters: members are destroyed in reverse, and the
    // GUI objects may still be waiting on requests that use apiClient.
    std::unique_ptr<ApiClient> apiClient;
    std::unique_ptr<AuthManager> authManager;
    std::unique_ptr<LoginWindow> loginWindow;
    std::unique_ptr<MainWindow> mainWindow;
};
