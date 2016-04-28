#ifndef BENCHMARKS_MUTEXBENCHMARKS_HPP
#define BENCHMARKS_MUTEXBENCHMARKS_HPP

// Copyright (c) 2016, Dmitry Koplyarov <koplyarov.da@gmail.com>
//
// Permission to use, copy, modify, and/or distribute this software for any purpose with or without fee is hereby granted,
// provided that the above copyright notice and this permission notice appear in all copies.
//
// THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS.
// IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS,
// WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.


#include <benchmarks/core/BenchmarkClass.hpp>
#include <benchmarks/core/utils/Storage.hpp>


namespace benchmarks
{

	template < typename MutexDesc_ >
	class MutexBenchmarks : public BenchmarksClass
	{
		using MutexType = typename MutexDesc_::MutexType;

	public:
		MutexBenchmarks()
			: BenchmarksClass("mutex")
		{
			AddBenchmark<>("basic", &MutexBenchmarks::Basic);
		}

	private:
		static void Basic(BenchmarkContext& context)
		{
			const auto n = context.GetIterationsCount();

			StorageArray<MutexType> m(n);

			context.Profile("create", n, [&]{ m.Construct(); });
			context.MeasureMemory("mutex", n);
			context.Profile("lock", n, [&]{ m.ForEach([](MutexType& m){ m.lock(); }); });
			context.Profile("unlock", n, [&]{ m.ForEach([](MutexType& m){ m.unlock(); }); });
			context.Profile("destroy", n, [&]{ m.Destruct(); });
		}
	};

}

#endif
