#ifndef BENCHMARKS_FUNCTIONSBENCHMARKS_HPP
#define BENCHMARKS_FUNCTIONSBENCHMARKS_HPP

// Copyright (c) 2016, Dmitry Koplyarov <koplyarov.da@gmail.com>
//
// Permission to use, copy, modify, and/or distribute this software for any purpose with or without fee is hereby granted,
// provided that the above copyright notice and this permission notice appear in all copies.
//
// THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS.
// IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS,
// WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.


#include <map>

#include <benchmarks/core/BenchmarkClass.hpp>
#include <benchmarks/core/utils/Storage.hpp>


namespace benchmarks
{

	template < typename FunctionDesc_ >
	class FunctionsBenchmarks : public BenchmarksClass
	{
		using FunctionType = typename FunctionDesc_::FunctionType;

	public:
		FunctionsBenchmarks()
			: BenchmarksClass("functions")
		{
			AddBenchmark<>("basic", &FunctionsBenchmarks::Basic);
		}

	private:
		static void Basic(BenchmarkContext& context)
		{
			const auto n = context.GetIterationsCount();

			StorageArray<FunctionType> f(n);

			context.Profile("creating", n, [&]{ f.Construct([]{ return []{}; }); });
			context.MeasureMemory("function", n);
			context.Profile("invoking", n, [&]{ f.ForEach([](const FunctionType& f){ f(); }); });
			context.Profile("destroying", n, [&]{ f.Destruct(); });
		}
	};

}

#endif
