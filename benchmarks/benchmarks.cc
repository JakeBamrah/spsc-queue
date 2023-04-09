/*
 * Benchmark approach referenced: cameron314/readerwriterqueue
 * (https://github.com/cameron314/readerwriterqueue)
 * Distributed under BSD Licence
 */

#include <algorithm>
#include <assert.h>
#include <chrono>
#include <iomanip>
#include <iostream>
#include <random>
#include <thread>

#include "../readerwriter_queue.h"
#include "../circular_buffer.h"
#include "time.cc"


enum BenchmarkType {
    benchmark_add,
    benchmark_remove,
    benchmark_single_thread,
    benchmark_concurrent,

    BENCHMARKS_TOTAL
};

const int LONGEST_BENCHMARK_NAME = 16;

template<typename Q>
double runBenchmark(BenchmarkType benchmark, double& opsPerIter);
const char* benchmarkName(BenchmarkType benchmark);

int main(int argc, char**argv)
{
    const int ITER = 20;
    const int FASTEST_PERCENT_CONSIDERED = 20;

    double spscResults[BENCHMARKS_TOTAL][ITER];
    double circBufferResults[BENCHMARKS_TOTAL][ITER];

    double spscOps[BENCHMARKS_TOTAL][ITER];
    double circBufferOps[BENCHMARKS_TOTAL][ITER];

    for (int benchmark = 0; benchmark < BENCHMARKS_TOTAL; ++benchmark)
    {
        for (int i = 0; i < ITER; ++i)
        {
            spscResults[benchmark][i] = runBenchmark<NonBlockingQueue<int>>((BenchmarkType) benchmark, spscOps[benchmark][i]);
            circBufferResults[benchmark][i] = runBenchmark<CircularBuffer<int, 100>>((BenchmarkType) benchmark, circBufferOps[benchmark][i]);
        }
    }

    // sort results per benchmark
    for (int benchmark = 0; benchmark < BENCHMARKS_TOTAL; ++benchmark)
    {
        std::sort(&spscResults[benchmark][0], &spscResults[benchmark][ITER - 1]);
        std::sort(&circBufferResults[benchmark][0], &circBufferResults[benchmark][ITER - 1]);
    }

    int max = std::max(2, (int)(ITER * FASTEST_PERCENT_CONSIDERED / 100));
    assert(max > 0);

    // build header for results table
    std::cout              << std::setw(LONGEST_BENCHMARK_NAME) << "         " << " |------ Min ------|------ Max ------|------ Avg ------|\n";
    std::cout << std::left << std::setw(LONGEST_BENCHMARK_NAME) << "Benchmark" << " |  SPSC  |  CIRC  |  SPSC  |  CIRC  |  SPSC  |  CIRC  |\n";
    std::cout.fill('-');
    std::cout              << std::setw(LONGEST_BENCHMARK_NAME) << "---------" << "-+--------+--------+--------+--------+--------+--------+\n";
    std::cout.fill(' ');

    // find min and max averages
    double spscOpsPerSec        = 0;
    double circBufferOpsPerSec  = 0;
    int opTimedBenchmarks       = 0;
    for (int benchmark = 0; benchmark < BENCHMARKS_TOTAL; benchmark++)
    {
        double spscMin = spscResults[benchmark][0], spscMax = spscResults[benchmark][max - 1];
        double circMin = circBufferResults[benchmark][0], circMax = circBufferResults[benchmark][max - 1];
        double spscAvg = std::accumulate(&spscResults[benchmark][0], &spscResults[benchmark][0] + max, 0.0) / max;
        double circAvg = std::accumulate(&circBufferResults[benchmark][0], &circBufferResults[benchmark][0] + max, 0.0) / max;

        double spscTotalAvg = std::accumulate(&spscResults[benchmark][0], &spscResults[benchmark][0] + ITER, 0.0) / ITER;
        double circTotalAvg = std::accumulate(&circBufferResults[benchmark][0], &circBufferResults[benchmark][0] + ITER, 0.0) / ITER;
        spscOpsPerSec       += spscTotalAvg == 0 ? 0 : std::accumulate(&spscOps[benchmark][0], &spscOps[benchmark][0] + ITER, 0.0) / ITER / spscTotalAvg;
        circBufferOpsPerSec += circTotalAvg == 0 ? 0 : std::accumulate(&circBufferOps[benchmark][0], &circBufferOps[benchmark][0] + ITER, 0.0) / ITER / circTotalAvg;

        ++opTimedBenchmarks;

        std::cout
            << std::left << std::setw(LONGEST_BENCHMARK_NAME) << benchmarkName((BenchmarkType)benchmark) << " | "
            << std::fixed << std::setprecision(3) << spscMin << "s | "
            << std::fixed << std::setprecision(3) << circMin << "s | "
            << std::fixed << std::setprecision(3) << spscMax << "s | "
            << std::fixed << std::setprecision(3) << circMax << "s | "
            << std::fixed << std::setprecision(3) << spscAvg << "s | "
            << std::fixed << std::setprecision(3) << circAvg << "s | "
            << "\n";
    }

    spscOpsPerSec       /= opTimedBenchmarks;
    circBufferOpsPerSec /= opTimedBenchmarks;
    std::cout
        << "\nAverage ops/s:\n"
        << "    SPSC Queue:         " << std::fixed << std::setprecision(2) << spscOpsPerSec / 1000000 << " million\n"
        << "    Circular Buffer:    " << std::fixed << std::setprecision(2) << circBufferOpsPerSec / 1000000 << " million\n"
    ;
    std::cout << std::endl;

    return 0;
}

template<typename Q>
double runBenchmark(BenchmarkType benchmark, double& opsPerIter)
{
    int SEED = 1337;

    double result = 0;
    switch (benchmark)
    {
        case benchmark_add:
        {
            Q queue;
            int num = 0;
            const int MAX = 200 * 1000;
            opsPerIter = MAX;
            TimePoint start = getTimePoint();
            for (int i = 0; i != MAX; ++i)
            {
                queue.enqueue(num);
                ++num;
            }
            result = getTimeDelta(start);
        } break;
        case benchmark_remove:
        {
            const int MAX = 200 * 1000;
            opsPerIter = MAX;

            Q queue;
            int num = 0;
            for (int i = 0; i != MAX; ++i)
            {
                queue.enqueue(num);
                ++num;
            }

            num = 0;
            int element = -1;
            TimePoint start = getTimePoint();
            for (int i = 0; i != MAX; ++i)
            {
                bool success = queue.dequeue(element);
                /* assert(success && element == num++); */
            }
            result = getTimeDelta(start);
            assert(queue.is_empty());
        } break;
        case benchmark_single_thread:
        {
            std::minstd_rand rng(SEED);
            std::uniform_int_distribution<int> rand(0, 1);

            const int MAX = 200 * 1000;
            opsPerIter = MAX;

            Q queue;
            int num = 0;
            int element = -1;
            TimePoint start = getTimePoint();
            for (int i = 0; i != MAX; ++i)
            {
                if (rand(rng) == 1)
                {
                    queue.enqueue(num++);
                }
                else
                {
                    queue.dequeue(element);
                }
            }
            result = getTimeDelta(start);
        } break;
        case benchmark_concurrent:
        {
            const int MAX = 1000 * 1000;
            opsPerIter = MAX * 2;

            Q queue;
            int element = -1;
            TimePoint start = getTimePoint();
            std::thread consumer([&]() {
                for (int i = 0; i != MAX; ++i)
                {
                    queue.dequeue(element);
                }
            });

            std::thread producer([&]() {
                int num = 0;
                for (int i = 0; i != MAX; ++i)
                {
                    queue.enqueue(num);
                    ++num;
                }
            });

            producer.join();
            consumer.join();
            result = getTimeDelta(start);
        } break;
        default:
        {
            // handle BENCHMARKS_TOTAL enum path
            return 0;
        }
    }

    return result / 1000.0;
}

const char* benchmarkName(BenchmarkType benchmark)
{
    switch (benchmark) {
        case benchmark_add: return "Raw add";
        case benchmark_remove: return "Raw remove";
        case benchmark_single_thread: return "Single-threaded";
        case benchmark_concurrent: return "Concurrent";
        default: return "";
    }
}
