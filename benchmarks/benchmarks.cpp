/*
 * Benchmark approach referenced: cameron314/readerwriterqueue
 * (https://github.com/cameron314/readerwriterqueue)
 * Distributed under BSD Licence
 */

#include <algorithm>
#include <assert.h>
#include <chrono>
#include <iostream>
#include <random>
#include <thread>

#include "../readerwriter_queue.h"
#include "time.cpp"


enum BenchmarkType {
    benchmark_add,
    benchmark_remove,
    benchmark_single_thread,
    benchmark_concurrent,

    BENCHMARKS_TOTAL
};

template<typename Q>
double runBenchmark(BenchmarkType benchmark, double& opsPerIter);

int main(int argc, char**argv)
{
    const int FASTEST_PERCENT_CONSIDERED = 20;

    const int ITER = 20;
    double spscResults[BENCHMARKS_TOTAL][ITER];
    double spscOps[BENCHMARKS_TOTAL][ITER];
    for (int benchmark = 0; benchmark < BENCHMARKS_TOTAL; ++benchmark)
    {
        for (int i = 0; i < ITER; ++i)
        {
            spscResults[benchmark][i] = runBenchmark<NonBlockingQueue<int>>((BenchmarkType) benchmark, spscOps[benchmark][i]);
        }
    }

    // sort results per benchmark
    for (int benchmark = 0; benchmark < BENCHMARKS_TOTAL; ++benchmark)
    {
        std::sort(&spscResults[benchmark][0], &spscResults[benchmark][ITER - 1]);
    }

    int max = std::max(2, (int)(ITER * FASTEST_PERCENT_CONSIDERED / 100));
    assert(max > 0);
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
                assert(success && element == num++);
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
