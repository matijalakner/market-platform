cmake_minimum_required(VERSION 3.20)

project(
    market-model
    VERSION 0.3.0
    DESCRIPTION "C++ market simulation, REST API and ImGui dashboard"
    LANGUAGES CXX
)

# ==== C++ standard ===================================

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

if (NOT CMAKE_BUILD_TYPE AND NOT CMAKE_CONFIGURATION_TYPES)
    set(CMAKE_BUILD_TYPE Release)
endif ()

option(BUILD_TESTS "Build tests" ON)
option(BUILD_EXAMPLES "Build examples" ON)
option(BUILD_DASHBOARD "Build the Dear ImGui dashboard (needs OpenGL, GLFW, libcurl)" OFF)

# ==== Library ========================================

add_library(market_model
    src/core/id_generator.cpp
    src/core/json.cpp

    src/market/market.cpp
    src/market/matching_engine.cpp
    src/market/order_book.cpp
    src/market/order_registry.cpp
    src/market/settlement.cpp
    src/market/trade_history.cpp

    src/agents/trader.cpp
    src/agents/trader_registry.cpp
    src/agents/random_trader.cpp
    src/agents/fundamental_trader.cpp
    src/agents/market_maker.cpp
    src/agents/momentum_trader.cpp
    src/agents/noise_trader.cpp
    src/agents/arbitrage_trader.cpp

    src/models/fundamental_value.cpp
    src/models/random_walk_model.cpp
    src/models/geometric_brownian_model.cpp
    src/models/garch_model.cpp

    src/statistics/statistics.cpp

    src/simulation/event_scheduler.cpp
    src/simulation/simulation.cpp
    src/simulation/experiment.cpp
)

target_include_directories(market_model PUBLIC ${PROJECT_SOURCE_DIR}/include)
target_compile_features(market_model PUBLIC cxx_std_20)

# ==== Compiler Warnings =============================

if (MSVC)
    target_compile_options(market_model PRIVATE /W4)
else ()
    target_compile_options(market_model PRIVATE -Wall -Wextra -Wpedantic)
endif ()

# ==== Examples ======================================

if (BUILD_EXAMPLES)
    add_executable(market_model_demo examples/main.cpp)
    target_link_libraries(market_model_demo PRIVATE market_model)

    add_executable(basic_simulation examples/basic_simulation.cpp)
    target_link_libraries(basic_simulation PRIVATE market_model)

    # Runs an experiment described by a JSON config file:
    #   ./build/market_simulation experiments/baseline/config.json
    add_executable(market_simulation examples/run_experiment.cpp)
    target_link_libraries(market_simulation PRIVATE market_model)
endif ()

# ==== Dashboard =====================================

if (BUILD_DASHBOARD)
    add_subdirectory(apps/dashboard)
endif ()

# ==== Tests =========================================

if (BUILD_TESTS)
    enable_testing()

    # The tests rely on assert(), so make sure NDEBUG never disables it.
    function(add_market_test name source)
        add_executable(${name} ${source})
        target_link_libraries(${name} PRIVATE market_model)
        if (MSVC)
            target_compile_options(${name} PRIVATE /UNDEBUG)
        else ()
            target_compile_options(${name} PRIVATE -UNDEBUG)
        endif ()
        add_test(NAME ${name} COMMAND ${name})
    endfunction()

    add_market_test(types_test tests/types_test.cpp)

    # core
    add_market_test(units_test tests/core/units_test.cpp)
    add_market_test(json_test tests/core/json_test.cpp)

    # market
    add_market_test(order_test tests/market/order_test.cpp)
    add_market_test(order_lifecycle_test tests/market/order_lifecycle_test.cpp)
    add_market_test(order_book_test tests/market/order_book_test.cpp)
    add_market_test(order_registry_test tests/market/order_registry_test.cpp)
    add_market_test(matching_engine_test tests/market/matching_engine_test.cpp)
    add_market_test(market_test tests/market/market_test.cpp)
    add_market_test(market_quote_test tests/market/market_quote_test.cpp)
    add_market_test(market_history_test tests/market/market_history_test.cpp)
    add_market_test(market_features_test tests/market/market_features_test.cpp)
    add_market_test(trade_history_test tests/market/trade_history_test.cpp)
    add_market_test(settlement_test tests/market/settlement_test.cpp)
    add_market_test(settlement_market_test tests/market/settlement_market_test.cpp)
    add_market_test(settlement_fee_test tests/market/settlement_fee_test.cpp)

    # agents
    add_market_test(trader_test tests/agents/trader_test.cpp)
    add_market_test(trader_reservation_test tests/agents/trader_reservation_test.cpp)
    add_market_test(trader_short_test tests/agents/trader_short_test.cpp)
    add_market_test(trader_pnl_test tests/agents/trader_pnl_test.cpp)
    add_market_test(random_trader_test tests/agents/random_trader_test.cpp)
    add_market_test(fundamental_trader_test tests/agents/fundamental_trader_test.cpp)
    add_market_test(agents_test tests/agents/agents_test.cpp)

    # models
    add_market_test(random_walk_model_test tests/models/random_walk_model_test.cpp)
    add_market_test(price_models_test tests/models/price_models_test.cpp)

    # statistics and simulation
    add_market_test(statistics_test tests/statistics/statistics_test.cpp)
    add_market_test(event_scheduler_test tests/simulation/event_scheduler_test.cpp)
    add_market_test(simulation_test tests/simulation/simulation_test.cpp)
    add_market_test(experiment_test tests/simulation/experiment_test.cpp)
    add_market_test(conservation_test tests/simulation/conservation_test.cpp)
endif ()
