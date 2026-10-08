# Dear ImGui dashboard. Included from the top-level CMakeLists.txt when
# -DBUILD_DASHBOARD=ON. Needs: OpenGL, GLFW, libcurl, and the sources fetched
# by scripts/setup_dependencies.sh (Dear ImGui, nlohmann/json).

find_package(OpenGL REQUIRED)
find_package(glfw3 REQUIRED)
find_package(CURL REQUIRED)
find_package(Threads REQUIRED)

set(IMGUI_DIR "${PROJECT_SOURCE_DIR}/external/imgui")
set(JSON_INCLUDE_DIR "${PROJECT_SOURCE_DIR}/external/json/include")

if (NOT EXISTS "${IMGUI_DIR}/imgui.cpp")
    message(FATAL_ERROR "Dear ImGui is missing. Run scripts/setup_dependencies.sh")
endif ()
if (NOT EXISTS "${JSON_INCLUDE_DIR}/nlohmann/json.hpp")
    message(FATAL_ERROR "nlohmann/json is missing. Run scripts/setup_dependencies.sh")
endif ()

add_library(dashboard_imgui STATIC
    ${IMGUI_DIR}/imgui.cpp
    ${IMGUI_DIR}/imgui_draw.cpp
    ${IMGUI_DIR}/imgui_tables.cpp
    ${IMGUI_DIR}/imgui_widgets.cpp
    ${IMGUI_DIR}/backends/imgui_impl_glfw.cpp
    ${IMGUI_DIR}/backends/imgui_impl_opengl3.cpp
)
# SYSTEM: third-party headers do not produce warnings in our build.
target_include_directories(dashboard_imgui SYSTEM PUBLIC ${IMGUI_DIR} ${IMGUI_DIR}/backends)
target_link_libraries(dashboard_imgui PUBLIC glfw OpenGL::GL)

add_executable(market_dashboard
    src/main.cpp
    src/app/Application.cpp
    src/api/ApiClient.cpp
    src/auth/AuthManager.cpp
    src/gui/LoginWindow.cpp
    src/gui/MainWindow.cpp
    src/gui/JsonView.cpp
    src/gui/StockPriceChart.cpp
)
target_include_directories(market_dashboard PRIVATE src)
target_include_directories(market_dashboard SYSTEM PRIVATE ${JSON_INCLUDE_DIR})
target_link_libraries(market_dashboard PRIVATE dashboard_imgui CURL::libcurl Threads::Threads)

# Pre-fills "admin" and shows the development credentials on the login screen.
option(DASHBOARD_DEV_HINTS "Show development login hints" ON)
if (DASHBOARD_DEV_HINTS)
    target_compile_definitions(market_dashboard PRIVATE DASHBOARD_DEV_HINTS=1)
endif ()

if (MSVC)
    target_compile_options(market_dashboard PRIVATE /W4)
else ()
    target_compile_options(market_dashboard PRIVATE -Wall -Wextra -Wpedantic)
endif ()
