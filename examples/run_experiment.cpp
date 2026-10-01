#include <iomanip>
#include <iostream>

#include "market/simulation/experiment.hpp"

int main(int argc, char** argv) {
    if (argc < 2) {
        std::cerr << "usage: " << argv[0] << " <config.json> [output_directory]\n";
        return 1;
    }

    try {
        market::ExperimentConfig config = market::load_experiment(argv[1]);
        if (argc >= 3) { config.output_directory = argv[2]; }

        market::ExperimentResult result = market::run_experiment(config);
        const market::StatisticsSummary& s = result.summary;

        std::cout << std::fixed << std::setprecision(6)
                  << "steps:               " << s.steps << '\n'
                  << "trades:              " << result.trades << '\n'
                  << "total volume:        " << s.total_volume << '\n'
                  << "first / last price:  " << s.first_price << " / " << s.last_price << '\n'
                  << "total return:        " << s.total_return * 100.0 << " %\n"
                  << "volatility (step):   " << s.volatility << '\n'
                  << "mean spread:         " << s.mean_spread << '\n'
                  << "mean depth bid/ask:  " << s.mean_bid_depth << " / " << s.mean_ask_depth << '\n'
                  << "mean |mispricing|:   " << s.mean_abs_mispricing * 100.0 << " %\n"
                  << "fees paid:           " << result.total_fees << '\n';

        if (!config.output_directory.empty()) {
            std::cout << "output written to:   " << config.output_directory << '\n';
        }
    } catch (const std::exception& e) {
        std::cerr << "error: " << e.what() << '\n';
        return 1;
    }
    return 0;
}
