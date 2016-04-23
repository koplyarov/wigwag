#ifndef BENCHMARKS_BENCHMARKSUITE_HPP
#define BENCHMARKS_BENCHMARKSUITE_HPP

// Copyright (c) 2016, Dmitry Koplyarov <koplyarov.da@gmail.com>
//
// Permission to use, copy, modify, and/or distribute this software for any purpose with or without fee is hereby granted,
// provided that the above copyright notice and this permission notice appear in all copies.
//
// THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS.
// IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS,
// WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.


#include <benchmarks/core/Benchmark.hpp>
#include <benchmarks/core/BenchmarkId.hpp>
#include <benchmarks/core/utils/Logger.hpp>

#include <map>
#include <stdexcept>


namespace benchmarks
{

	namespace detail
	{
		using BenchmarksMap = std::map<BenchmarkId, IBenchmarkPtr>;


		template < template <typename> class BenchmarksClass_, typename... ObjectsDesc_ >
		struct BenchmarksClassRegistrar
		{
			static void Register(BenchmarksMap& benchmarks)
			{ }
		};

		template < template <typename> class BenchmarksClass_, typename ObjectsDescHead_, typename... ObjectsDescTail_ >
		struct BenchmarksClassRegistrar<BenchmarksClass_, ObjectsDescHead_, ObjectsDescTail_...>
		{
			static void Register(BenchmarksMap& benchmarks)
			{
				BenchmarksClass_<ObjectsDescHead_> bm;
				for (auto b : bm.GetBenchmarks())
					benchmarks.insert({BenchmarkId(bm.GetName(), b->GetName(), ObjectsDescHead_::GetName()), b});

				BenchmarksClassRegistrar<BenchmarksClass_, ObjectsDescTail_...>::Register(benchmarks);
			}
		};
	}


	class BenchmarkSuite
	{
		using BenchmarksMap = std::map<BenchmarkId, IBenchmarkPtr>;

	public:
		struct IBenchmarksResultsReporter;
		using IBenchmarksResultsReporterPtr = std::shared_ptr<IBenchmarksResultsReporter>;

	private:
		class PreMeasureBenchmarkContext;
		class MeasureBenchmarkContext;

	private:
		static NamedLogger	s_logger;
		BenchmarksMap		_benchmarks;

	public:
		template < template <typename> class BenchmarksClass_, typename... ObjectsDesc_ >
		void RegisterBenchmarks()
		{ detail::BenchmarksClassRegistrar<BenchmarksClass_, ObjectsDesc_...>::Register(_benchmarks); }

		int64_t MeasureIterationsCount(const BenchmarkId& id, const SerializedParamsMap& serializedParams);
		void InvokeBenchmark(int64_t iterations, const BenchmarkId& id, const SerializedParamsMap& serializedParams);
	};
}

#endif
