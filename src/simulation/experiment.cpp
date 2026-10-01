#include "market/simulation/experiment.hpp"

#include <filesystem>
#include <fstream>
#include <memory>
#include <sstream>
#include <stdexcept>

#include "market/agents/arbitrage_trader.hpp"
#include "market/agents/fundamental_trader.hpp"
#include "market/agents/market_maker.hpp"
#include "market/agents/momentum_trader.hpp"
#include "market/agents/noise_trader.hpp"
#include "market/agents/random_trader.hpp"
#include "market/agents/trader_registry.hpp"
#include "market/core/json.hpp"
#include "market/core/units.hpp"
#include "market/market/market.hpp"
#include "market/models/fundamental_value.hpp"
#include "market/models/garch_model.hpp"
#include "market/models/geometric_brownian_model.hpp"
#include "market/models/random_walk_model.hpp"
#include "market/simulation/simulation.hpp"

namespace market {

namespace {

template <typename T>
T as_count(double value) {
    return value <= 0.0 ? T{0} : static_cast<T>(value);
}

}  // namespace

ExperimentConfig parse_experiment(const std::string& json_text) {
    Json root = Json::parse(json_text);
    if (!root.is_object()) { throw std::runtime_error("config root must be a JSON object"); }

    ExperimentConfig c;
    static const Json empty = Json::parse("{}");
    auto section = [&](const char* name) -> const Json& {
        const Json* s = root.find(name);
        return s ? *s : empty;
    };

    const Json& sim = section("simulation");
    c.simulation.steps = as_count<std::size_t>(sim.number("steps", static_cast<double>(c.simulation.steps)));
    c.simulation.seed = as_count<std::uint64_t>(sim.number("seed", static_cast<double>(c.simulation.seed)));
    c.simulation.activation_probability = sim.number("activation_probability", c.simulation.activation_probability);
    c.simulation.random_order = sim.boolean("random_order", c.simulation.random_order);
    c.simulation.fee_rate = sim.number("fee_rate", c.simulation.fee_rate);

    const Json& mkt = section("market");
    c.initial_price = mkt.number("initial_price", c.initial_price);
    c.simulation.tick_size = mkt.number("tick_size", c.simulation.tick_size);
    c.market.latency = as_count<Timestamp>(mkt.number("latency", 0.0));
    if (c.simulation.tick_size <= 0.0) { throw std::runtime_error("market.tick_size must be positive"); }
    if (c.initial_price <= 0.0) { throw std::runtime_error("market.initial_price must be positive"); }

    const Json& tr = section("traders");
    c.initial_cash = tr.number("cash", c.initial_cash);
    c.initial_assets = static_cast<Position>(tr.number("assets", static_cast<double>(c.initial_assets)));
    c.short_limit = as_count<Quantity>(tr.number("short_limit", 0.0));

    const Json& m = section("model");
    c.model.type = m.string("type", c.model.type);
    c.model.drift = m.number("drift", c.model.drift);
    c.model.volatility = m.number("volatility", c.model.volatility);
    c.model.mu = m.number("mu", c.model.mu);
    c.model.sigma = m.number("sigma", c.model.sigma);
    c.model.omega = m.number("omega", c.model.omega);
    c.model.alpha = m.number("alpha", c.model.alpha);
    c.model.beta = m.number("beta", c.model.beta);
    if (c.model.type != "random_walk" && c.model.type != "gbm" && c.model.type != "garch") {
        throw std::runtime_error("unknown model type: " + c.model.type);
    }

    const Json& a = section("agents");
    c.agents.random_traders = as_count<std::size_t>(a.number("random_traders", static_cast<double>(c.agents.random_traders)));
    c.agents.fundamental_traders = as_count<std::size_t>(a.number("fundamental_traders", static_cast<double>(c.agents.fundamental_traders)));
    c.agents.market_makers = as_count<std::size_t>(a.number("market_makers", static_cast<double>(c.agents.market_makers)));
    c.agents.momentum_traders = as_count<std::size_t>(a.number("momentum_traders", 0.0));
    c.agents.noise_traders = as_count<std::size_t>(a.number("noise_traders", 0.0));
    c.agents.arbitrage_traders = as_count<std::size_t>(a.number("arbitrage_traders", 0.0));

    const Json& p = section("agent_params");
    c.params.fundamental_threshold = p.number("fundamental_threshold", c.params.fundamental_threshold);
    c.params.order_quantity = as_count<Quantity>(p.number("order_quantity", static_cast<double>(c.params.order_quantity)));
    c.params.market_maker_half_spread = p.number("market_maker_half_spread", c.params.market_maker_half_spread);
    c.params.market_maker_quantity = as_count<Quantity>(p.number("market_maker_quantity", static_cast<double>(c.params.market_maker_quantity)));
    c.params.market_maker_skew = p.number("market_maker_skew", c.params.market_maker_skew);
    c.params.momentum_lookback = as_count<std::size_t>(p.number("momentum_lookback", static_cast<double>(c.params.momentum_lookback)));
    c.params.momentum_threshold = p.number("momentum_threshold", c.params.momentum_threshold);
    c.params.noise_max_offset = p.number("noise_max_offset", c.params.noise_max_offset);
    c.params.noise_market_order_probability = p.number("noise_market_order_probability", c.params.noise_market_order_probability);
    c.params.arbitrage_threshold = p.number("arbitrage_threshold", c.params.arbitrage_threshold);

    if (const Json* news = root.find("news"); news && news->is_array()) {
        for (const Json& item : news->items()) {
            ExperimentConfig::News n;
            n.time = as_count<Timestamp>(item.number("time", 0.0));
            n.shock = item.number("shock", 0.0);
            c.news.push_back(n);
        }
    }

    const Json& out = section("output");
    c.output_directory = out.string("directory", "");
    c.trader_interval = as_count<std::size_t>(out.number("trader_interval", 1.0));

    return c;
}

ExperimentConfig load_experiment(const std::string& path) {
    std::ifstream file(path);
    if (!file) { throw std::runtime_error("cannot open config file: " + path); }
    std::stringstream buffer;
    buffer << file.rdbuf();
    return parse_experiment(buffer.str());
}

ExperimentResult run_experiment(const ExperimentConfig& cfg) {
    const double tick = cfg.simulation.tick_size;
    const std::uint64_t seed = cfg.simulation.seed;

    // ---- price model ----
    std::unique_ptr<PriceModel> model;
    if (cfg.model.type == "gbm") {
        model = std::make_unique<GeometricBrownianModel>(cfg.model.mu, cfg.model.sigma, seed);
    } else if (cfg.model.type == "garch") {
        model = std::make_unique<GarchModel>(cfg.model.mu, cfg.model.omega, cfg.model.alpha, cfg.model.beta, seed);
    } else {
        model = std::make_unique<RandomWalkModel>(cfg.model.drift / tick, cfg.model.volatility / tick, seed);
    }

    const Price initial_price = currency_to_ticks(cfg.initial_price, tick);
    FundamentalValue fundamental(initial_price, *model);

    // ---- traders and agents ----
    TraderRegistry traders;
    std::vector<std::unique_ptr<Agent>> agents;
    TraderId next_id = 1;
    std::uint64_t next_seed = seed + 1;

    const double cash_ticks = cfg.initial_cash / tick;
    auto new_trader = [&]() {
        TraderId id = next_id++;
        traders.add_trader(Trader(id, cash_ticks, cfg.initial_assets, cfg.short_limit));
        return id;
    };

    const ExperimentConfig::Params& p = cfg.params;

    for (std::size_t i = 0; i < cfg.agents.market_makers; ++i) {
        agents.push_back(std::make_unique<MarketMaker>(
            new_trader(), currency_to_ticks(p.market_maker_half_spread, tick),
            p.market_maker_quantity, p.market_maker_skew / tick));
    }
    for (std::size_t i = 0; i < cfg.agents.random_traders; ++i) {
        agents.push_back(std::make_unique<RandomTrader>(new_trader(), initial_price, next_seed++));
    }
    for (std::size_t i = 0; i < cfg.agents.fundamental_traders; ++i) {
        agents.push_back(std::make_unique<FundamentalTrader>(
            new_trader(), p.fundamental_threshold, p.order_quantity));
    }
    for (std::size_t i = 0; i < cfg.agents.momentum_traders; ++i) {
        agents.push_back(std::make_unique<MomentumTrader>(
            new_trader(), p.momentum_lookback, p.momentum_threshold, p.order_quantity, initial_price));
    }
    for (std::size_t i = 0; i < cfg.agents.noise_traders; ++i) {
        agents.push_back(std::make_unique<NoiseTrader>(
            new_trader(), initial_price, next_seed++,
            currency_to_ticks(p.noise_max_offset, tick), p.order_quantity,
            p.noise_market_order_probability));
    }
    for (std::size_t i = 0; i < cfg.agents.arbitrage_traders; ++i) {
        agents.push_back(std::make_unique<ArbitrageTrader>(
            new_trader(), p.arbitrage_threshold, p.order_quantity));
    }

    // ---- run ----
    Market market(traders, cfg.market);
    Simulation simulation(cfg.simulation, market, fundamental, traders);
    for (auto& agent : agents) { simulation.add_agent(std::move(agent)); }
    for (const auto& n : cfg.news) { simulation.schedule_news(n.time, n.shock); }

    StatisticsRecorder recorder(tick, cfg.trader_interval);
    simulation.set_recorder(&recorder);
    simulation.run();

    if (!cfg.output_directory.empty()) {
        std::filesystem::create_directories(cfg.output_directory);
        recorder.write_market_csv(cfg.output_directory + "/market.csv");
        recorder.write_traders_csv(cfg.output_directory + "/traders.csv");
    }

    ExperimentResult result;
    result.summary = recorder.summary();
    result.trades = market.trade_count();
    result.total_fees = simulation.settlement().total_fees() * tick;
    return result;
}

}  // namespace market
