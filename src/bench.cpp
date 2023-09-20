/**
 * @file bench.cpp
 * 
 * @author Gavin Dan (xfdan10@gmail.com)
 * @brief Benchmarking weighted red-black tree Sorted Bucket
 * @version 1.1
 * @date 2023-09-19
 * 
 * 
 * Benchmarking performance using Google Benchmark
 */

#include <benchmark/benchmark.h>
#include <random>
#include <string>
#include "sortedBucketRBT.h"
#include "sortedBucketLL.h"
#include "sortedBucketVV.h"


/* Benchmark iteration factors */
constexpr size_t benchMultiplier	= 10;
constexpr size_t benchIterLow		= 1000;
constexpr size_t benchIterHigh		= 1000000;

/* Mersenne Twister random number gen */
static std::mt19937_64 random{uint64_t(rand())};


/* Benchmark wrappers for RBT */
template <typename T>
static void BM_RBT_find(benchmark::State& state) {
	for (auto _ : state) {
		state.PauseTiming();
		SortedBucketRBT<T> rbt;
		const size_t ops = state.range(0);
		for (size_t i = 0; i < ops; ++i) {
			rbt.insert(random());
		}
		state.ResumeTiming();
		for (size_t i = 0; i < ops; ++i) {
			benchmark::DoNotOptimize(rbt.find(random()));
		}
	}
}
template <typename T>
static void BM_RBT_insert(benchmark::State& state) {
	for (auto _ : state) {
		state.PauseTiming();
		SortedBucketRBT<T> rbt;
		const size_t ops = state.range(0);
		state.ResumeTiming();
		for (size_t i = 0; i < ops; ++i) {
			benchmark::DoNotOptimize(rbt.insert(random()));
		}
	}
}
template <typename T>
static void BM_RBT_distance(benchmark::State& state) {
	for (auto _ : state) {
		state.PauseTiming();
		SortedBucketRBT<T> rbt;
		const size_t ops = state.range(0);
		for (size_t i = 0; i < ops; ++i) {
			rbt.insert(random());
		}
		state.ResumeTiming();
		for (size_t i = 0; i < ops; ++i) {
			benchmark::DoNotOptimize(rbt.distance(random()));
		}
	}
}
template <typename T>
static void BM_RBT_erase(benchmark::State& state) {
	for (auto _ : state) {
		state.PauseTiming();
		SortedBucketRBT<T> rbt;
		const size_t ops = state.range(0);
		for (size_t i = 0; i < ops; ++i) {
			rbt.insert(random());
		}
		state.ResumeTiming();
		for (size_t i = 0; i < ops; ++i) {
			benchmark::DoNotOptimize(rbt.erase(random()));
		}
	}
}


/* Benchmark wrappers for LL */
template <typename T>
static void BM_LL_find(benchmark::State& state) {
	for (auto _ : state) {
		state.PauseTiming();
		SortedBucketLL<T> ll;
		const size_t ops = state.range(0);
		for (size_t i = 0; i < ops; ++i) {
			ll.insert(random());
		}
		state.ResumeTiming();
		for (size_t i = 0; i < ops; ++i) {
			benchmark::DoNotOptimize(ll.find(random()));
		}
	}
}
template <typename T>
static void BM_LL_insert(benchmark::State& state) {
	for (auto _ : state) {
		state.PauseTiming();
		SortedBucketLL<T> ll;
		const size_t ops = state.range(0);
		state.ResumeTiming();
		for (size_t i = 0; i < ops; ++i) {
			benchmark::DoNotOptimize(ll.insert(random()));
		}
	}
}
template <typename T>
static void BM_LL_distance(benchmark::State& state) {
	for (auto _ : state) {
		state.PauseTiming();
		SortedBucketLL<T> ll;
		const size_t ops = state.range(0);
		for (size_t i = 0; i < ops; ++i) {
			ll.insert(random());
		}
		state.ResumeTiming();
		for (size_t i = 0; i < ops; ++i) {
			benchmark::DoNotOptimize(ll.distance(random()));
		}
	}
}
template <typename T>
static void BM_LL_erase(benchmark::State& state) {
	for (auto _ : state) {
		state.PauseTiming();
		SortedBucketLL<T> ll;
		const size_t ops = state.range(0);
		for (size_t i = 0; i < ops; ++i) {
			ll.insert(random());
		}
		state.ResumeTiming();
		for (size_t i = 0; i < ops; ++i) {
			benchmark::DoNotOptimize(ll.erase(random()));
		}
	}
}


/* Benchmark wrappers for VV */
template <typename T>
static void BM_VV_find(benchmark::State& state) {
	for (auto _ : state) {
		state.PauseTiming();
		SortedBucketVV<T> vv;
		const size_t ops = state.range(0);
		for (size_t i = 0; i < ops; ++i) {
			vv.insert(random());
		}
		state.ResumeTiming();
		for (size_t i = 0; i < ops; ++i) {
			benchmark::DoNotOptimize(vv.find(random()));
		}
	}
}
template <typename T>
static void BM_VV_insert(benchmark::State& state) {
	for (auto _ : state) {
		state.PauseTiming();
		SortedBucketVV<T> vv;
		const size_t ops = state.range(0);
		state.ResumeTiming();
		for (size_t i = 0; i < ops; ++i) {
			benchmark::DoNotOptimize(vv.insert(random()));
		}
	}
}
template <typename T>
static void BM_VV_distance(benchmark::State& state) {
	for (auto _ : state) {
		state.PauseTiming();
		SortedBucketVV<T> vv;
		const size_t ops = state.range(0);
		for (size_t i = 0; i < ops; ++i) {
			vv.insert(random());
		}
		state.ResumeTiming();
		for (size_t i = 0; i < ops; ++i) {
			benchmark::DoNotOptimize(vv.distance(random()));
		}
	}
}
template <typename T>
static void BM_VV_erase(benchmark::State& state) {
	for (auto _ : state) {
		state.PauseTiming();
		SortedBucketVV<T> vv;
		const size_t ops = state.range(0);
		for (size_t i = 0; i < ops; ++i) {
			vv.insert(random());
		}
		state.ResumeTiming();
		for (size_t i = 0; i < ops; ++i) {
			benchmark::DoNotOptimize(vv.erase(random()));
		}
	}
}


/* Register wrappers for benchmark */
BENCHMARK_TEMPLATE(BM_RBT_find, uint64_t)
	->RangeMultiplier(benchMultiplier)
	->Range(benchIterLow, benchIterHigh)
	->Unit(benchmark::kMillisecond);
BENCHMARK_TEMPLATE(BM_LL_find, uint64_t)
	->RangeMultiplier(benchMultiplier)
	->Range(benchIterLow, benchIterHigh)
	->Unit(benchmark::kMillisecond);
BENCHMARK_TEMPLATE(BM_VV_find, uint64_t)
	->RangeMultiplier(benchMultiplier)
	->Range(benchIterLow, benchIterHigh)
	->Unit(benchmark::kMillisecond);

BENCHMARK_TEMPLATE(BM_RBT_distance, uint64_t)
	->RangeMultiplier(benchMultiplier)
	->Range(benchIterLow, benchIterHigh)
	->Unit(benchmark::kMillisecond);
BENCHMARK_TEMPLATE(BM_LL_distance, uint64_t)
	->RangeMultiplier(benchMultiplier)
	->Range(benchIterLow, benchIterHigh)
	->Unit(benchmark::kMillisecond);
BENCHMARK_TEMPLATE(BM_VV_distance, uint64_t)
	->RangeMultiplier(benchMultiplier)
	->Range(benchIterLow, benchIterHigh)
	->Unit(benchmark::kMillisecond);

BENCHMARK_TEMPLATE(BM_RBT_insert, uint64_t)
	->RangeMultiplier(benchMultiplier)
	->Range(benchIterLow, benchIterHigh)
	->Unit(benchmark::kMillisecond);
BENCHMARK_TEMPLATE(BM_LL_insert, uint64_t)
	->RangeMultiplier(benchMultiplier)
	->Range(benchIterLow, benchIterHigh)
	->Unit(benchmark::kMillisecond);
BENCHMARK_TEMPLATE(BM_VV_insert, uint64_t)
	->RangeMultiplier(benchMultiplier)
	->Range(benchIterLow, benchIterHigh)
	->Unit(benchmark::kMillisecond);

BENCHMARK_TEMPLATE(BM_RBT_erase, uint64_t)
	->RangeMultiplier(benchMultiplier)
	->Range(benchIterLow, benchIterHigh)
	->Unit(benchmark::kMillisecond);
BENCHMARK_TEMPLATE(BM_LL_erase, uint64_t)
	->RangeMultiplier(benchMultiplier)
	->Range(benchIterLow, benchIterHigh)
	->Unit(benchmark::kMillisecond);
BENCHMARK_TEMPLATE(BM_VV_erase, uint64_t)
	->RangeMultiplier(benchMultiplier)
	->Range(benchIterLow, benchIterHigh)
	->Unit(benchmark::kMillisecond);


BENCHMARK_MAIN();