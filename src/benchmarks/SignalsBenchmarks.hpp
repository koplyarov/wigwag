#ifndef SRC_BENCHMARKS_SIGNALSBENCHMARKS_HPP
#define SRC_BENCHMARKS_SIGNALSBENCHMARKS_HPP

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
#include <benchmarks/core/utils/StorageFor.hpp>
#include <benchmarks/markers.hpp>


namespace benchmarks
{

	template < typename SignalsDesc_ >
	class SignalsBenchmarks : public BenchmarksClass
	{
		using SignalType = typename SignalsDesc_::signal_type;
		using HandlerType = typename SignalsDesc_::handler_type;
		using ConnectionType = typename SignalsDesc_::connection_type;

	public:
		SignalsBenchmarks()
			: BenchmarksClass("signals")
		{
			AddBenchmark<>("create", &SignalsBenchmarks::Create);
			AddBenchmark<int64_t>("invoke", &SignalsBenchmarks::Invoke, {"numSlots"});
		}

	private:
		static void Create(BenchmarkContext& context)
		{
			const auto n = context.GetIterationsCount();

			StorageFor<SignalType> *v = new StorageFor<SignalType>[(size_t)n];

			{
				auto op = context.Profile("creating", n);
				for (int64_t i = 0; i < n; ++i)
					v[i].Construct();
			}

			context.MeasureMemory("object", n);

			{
				auto op = context.Profile("destroying", n);
				for (int64_t i = 0; i < n; ++i)
					v[i].Destruct();
			}

			delete[] v;
		}

		static void Invoke(BenchmarkContext& context, int64_t numSlots)
		{
			const auto n = context.GetIterationsCount();

			HandlerType handler = SignalsDesc_::make_handler();
			SignalType s;
			StorageFor<ConnectionType> *c = new StorageFor<ConnectionType>[(size_t)numSlots];

			for (int64_t i = 0; i < numSlots; ++i)
				c[i].Construct(s.connect(handler));

			{
				auto op = context.Profile("invoking", numSlots * n);
				for (int64_t i = 0; i < n; ++i)
					s();
			}

			for (int64_t i = 0; i < numSlots; ++i)
				c[i].Destruct();

			delete[] c;
		}
	};

}

#endif
