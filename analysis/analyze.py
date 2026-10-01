#!/usr/bin/env python3
"""Analyse the CSV files written by market_simulation.

Usage:
    python3 analysis/analyze.py output/baseline
    python3 analysis/analyze.py output/baseline output/high_volatility   # compare runs
    python3 analysis/analyze.py output/baseline --plot                   # save PNG charts

Only the standard library is needed for the text report. Charts (--plot)
need matplotlib:  pip install -r analysis/requirements.txt
"""

import argparse
import csv
import math
import os
import statistics
import sys


def read_csv(path):
    with open(path, newline="") as f:
        return list(csv.DictReader(f))


def num(value):
    return float(value) if value not in ("", None) else None


def load_run(directory):
    market = read_csv(os.path.join(directory, "market.csv"))
    traders_path = os.path.join(directory, "traders.csv")
    traders = read_csv(traders_path) if os.path.exists(traders_path) else []
    return market, traders


def log_returns(prices):
    return [math.log(b / a) for a, b in zip(prices, prices[1:]) if a > 0 and b > 0]


def max_drawdown(prices):
    peak, worst = prices[0], 0.0
    for p in prices:
        peak = max(peak, p)
        worst = min(worst, p / peak - 1.0)
    return worst


def autocorrelation(xs, lag=1):
    if len(xs) <= lag + 1:
        return 0.0
    mean = statistics.fmean(xs)
    var = sum((x - mean) ** 2 for x in xs)
    if var == 0:
        return 0.0
    cov = sum((xs[i] - mean) * (xs[i + lag] - mean) for i in range(len(xs) - lag))
    return cov / var


def summarise(market, traders):
    price = [num(r["price"]) for r in market]
    fundamental = [num(r["fundamental"]) for r in market]
    spread = [num(r["spread"]) for r in market if r["spread"] != ""]
    volume = [num(r["volume"]) for r in market]
    bid_depth = [num(r["bid_depth"]) for r in market]
    ask_depth = [num(r["ask_depth"]) for r in market]
    returns = log_returns(price)

    out = {
        "steps": len(market),
        "first price": price[0],
        "last price": price[-1],
        "total return %": (price[-1] / price[0] - 1.0) * 100.0,
        "max drawdown %": max_drawdown(price) * 100.0,
        "volatility / step": statistics.stdev(returns) if len(returns) > 1 else 0.0,
        "return autocorr (lag 1)": autocorrelation(returns),
        "mean spread": statistics.fmean(spread) if spread else float("nan"),
        "book with both sides %": 100.0 * len(spread) / len(market),
        "total volume": num(market[-1]["total_volume"]),
        "mean volume / step": statistics.fmean(volume),
        "mean bid depth": statistics.fmean(bid_depth),
        "mean ask depth": statistics.fmean(ask_depth),
        "mean |price - fundamental| %": 100.0 * statistics.fmean(
            abs(p - f) / f for p, f in zip(price, fundamental) if f > 0),
    }

    if traders:
        last_t = max(int(r["timestamp"]) for r in traders)
        final = [r for r in traders if int(r["timestamp"]) == last_t]
        pnls = [num(r["pnl"]) for r in final]
        out["traders"] = len(final)
        out["mean final P&L"] = statistics.fmean(pnls)
        out["best / worst P&L"] = f"{max(pnls):.2f} / {min(pnls):.2f}"
        out["max |inventory|"] = max(abs(int(r["inventory"])) for r in final)
    return out


def print_report(name, summary):
    print(f"\n=== {name} ===")
    for key, value in summary.items():
        if isinstance(value, float):
            print(f"  {key:<30} {value:>14.6f}")
        else:
            print(f"  {key:<30} {value!s:>14}")


def plot_run(name, directory, market, traders):
    try:
        import matplotlib
        matplotlib.use("Agg")
        import matplotlib.pyplot as plt
    except ImportError:
        print("matplotlib not installed - skipping charts "
              "(pip install -r analysis/requirements.txt)", file=sys.stderr)
        return

    t = [int(r["timestamp"]) for r in market]
    fig, axes = plt.subplots(4, 1, figsize=(10, 12), sharex=True)

    axes[0].plot(t, [num(r["price"]) for r in market], label="market price")
    axes[0].plot(t, [num(r["fundamental"]) for r in market], label="fundamental", alpha=0.7)
    axes[0].set_ylabel("price")
    axes[0].legend()

    axes[1].plot(t, [num(r["spread"]) if r["spread"] != "" else float("nan") for r in market])
    axes[1].set_ylabel("spread")

    axes[2].bar(t, [num(r["volume"]) for r in market], width=1.0)
    axes[2].set_ylabel("volume / step")

    axes[3].plot(t, [num(r["bid_depth"]) for r in market], label="bid depth")
    axes[3].plot(t, [num(r["ask_depth"]) for r in market], label="ask depth")
    axes[3].set_ylabel("depth")
    axes[3].set_xlabel("step")
    axes[3].legend()

    fig.suptitle(name)
    fig.tight_layout()
    path = os.path.join(directory, "market.png")
    fig.savefig(path, dpi=120)
    plt.close(fig)
    print(f"saved {path}")

    if traders:
        by_trader = {}
        for r in traders:
            by_trader.setdefault(int(r["trader_id"]), []).append(
                (int(r["timestamp"]), num(r["pnl"]), int(r["inventory"])))
        fig, (a, b) = plt.subplots(2, 1, figsize=(10, 8), sharex=True)
        for tid, rows in by_trader.items():
            a.plot([x[0] for x in rows], [x[1] for x in rows], linewidth=0.8)
            b.plot([x[0] for x in rows], [x[2] for x in rows], linewidth=0.8)
        a.set_ylabel("P&L")
        b.set_ylabel("inventory")
        b.set_xlabel("step")
        fig.suptitle(f"{name}: traders")
        fig.tight_layout()
        path = os.path.join(directory, "traders.png")
        fig.savefig(path, dpi=120)
        plt.close(fig)
        print(f"saved {path}")


def main():
    parser = argparse.ArgumentParser(description=__doc__,
                                     formatter_class=argparse.RawDescriptionHelpFormatter)
    parser.add_argument("directories", nargs="+", help="output directories containing market.csv")
    parser.add_argument("--plot", action="store_true", help="save PNG charts (needs matplotlib)")
    args = parser.parse_args()

    for directory in args.directories:
        if not os.path.exists(os.path.join(directory, "market.csv")):
            print(f"error: {directory}/market.csv not found", file=sys.stderr)
            return 1
        market, traders = load_run(directory)
        name = os.path.basename(os.path.normpath(directory))
        print_report(name, summarise(market, traders))
        if args.plot:
            plot_run(name, directory, market, traders)
    return 0


if __name__ == "__main__":
    sys.exit(main())
