#!/usr/bin/env python

import matplotlib.pyplot as plt
import subprocess
import argparse
import os
from pathlib import Path

def run_experiment(output_file, build_dir):
    for threads in [2**i for i in range(0, os.cpu_count().bit_length())]:
        for size in [1e2, 1e3, 1e4 + 1, 1e5, 1e6 - 1, 1e7]:
            print("Measuring p=" + str(threads) + " n=" + str(size))
            executable = Path(build_dir) / "Sorter"
            returncode = subprocess.call([executable, str(size), str(threads)], stdout=output_file)
            if returncode != 0:
                print("Program crashed")

def make_plot(result_file):
    plots = dict()
    maxDuration = 0
    for line in result_file:
        if line.startswith("RESULT"):
            line = line[len("RESULT "):].strip()
            parts = line.split()
            measurement = {}
            for part in parts:
                key, value = part.split('=')
                measurement[key] = value

            n = int(round(int(measurement["n"]), -1))
            t = int(measurement["t"])
            name = measurement["name"]
            durationNanoseconds = int(measurement["durationNanoseconds"])
            constructorNanoseconds = int(measurement["constructorNanoseconds"])

            if t not in plots:
                plots[t] = dict()
            if name not in plots[t]:
                plots[t][name] = list()
                plots[t][name + " (constructor)"] = list()
            plots[t][name].append((n, durationNanoseconds / n))
            plots[t][name + " (constructor)"].append((n, constructorNanoseconds / n))
            maxDuration = max(maxDuration, max(durationNanoseconds, constructorNanoseconds))

    fig, axs = plt.subplots(len(plots), sharex=True, sharey=True, figsize=(10,10))

    for i, t in enumerate(plots):
        if len(plots) > 1:
            axs[i].set_title(f"#p={t}")
            for name in plots[t]:
                if name.endswith(" (constructor)"):
                    continue
                axs[i].plot(*zip(*plots[t][name]), label=name, marker='x')
                axs[i].plot(*zip(*plots[t][name + " (constructor)"]), label=name + " (constructor)", marker='+')
                axs[i].set_xscale('log')
        else:
            axs.plot(*zip(*plots[t][name]), label=name, marker='x')
            axs.plot(*zip(*plots[t][name + " (constructor)"]), label=name + " (constructor)", marker='+')
            axs.set_xscale('log')

    if len(plots) > 1:
        for ax in axs.flat:
            ax.set(xlabel='n', ylabel='Running time per element (ns)')
            ax.legend()
    else:
        axs.set(xlabel='n', ylabel='Running time per element (ns)')
        axs.legend()

    plt.tight_layout()

    plt.savefig("plot.pdf")

def main():
    parser = argparse.ArgumentParser(description='evaluation tools')
    # subcommands run and plot
    subparsers = parser.add_subparsers(dest='command')
    run_parser = subparsers.add_parser('run')
    run_parser.add_argument('output_file', type=argparse.FileType('w'), help='output file')
    run_parser.add_argument("--build-dir", default="build", help="build directory")
    plot_parser = subparsers.add_parser('plot')
    plot_parser.add_argument('result_file', type=argparse.FileType('r'), help='result file')
    args = parser.parse_args()
    if args.command == 'run':
        run_experiment(args.output_file, args.build_dir)
    elif args.command == 'plot':
        make_plot(args.result_file)
    else:
        parser.print_help()

if __name__ == "__main__":
    main()
