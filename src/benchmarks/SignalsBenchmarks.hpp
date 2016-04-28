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


#include <benchmarks/core/BenchmarkClass.hpp>
#include <benchmarks/core/utils/Storage.hpp>


namespace benchmarks
{

	template < typename SignalsDesc_ >
	class SignalsBenchmarks : public BenchmarksClass
	{
		using SignalType = typename SignalsDesc_::SignalType;
		using HandlerType = typename SignalsDesc_::HandlerType;
		using ConnectionType = typename SignalsDesc_::ConnectionType;

	public:
		SignalsBenchmarks()
			: BenchmarksClass("signals")
		{
			AddBenchmark<>("create", &SignalsBenchmarks::Create);
			AddBenchmark<>("handlerSize", &SignalsBenchmarks::HandlerSize);
			AddBenchmark<int64_t>("invoke", &SignalsBenchmarks::Invoke, {"numSlots"});
			AddBenchmark<int64_t>("connect", &SignalsBenchmarks::Connect, {"numSlots"});
		}

	private:
		static void Create(BenchmarkContext& context)
		{
			const auto n = context.GetIterationsCount();

			StorageArray<SignalType> s(n);

			context.Profile("creating", n, [&]{ s.Construct(); });
			context.MeasureMemory("signal", n);
			context.Profile("destroying", n, [&]{ s.Destruct(); });
		}

		static void HandlerSize(BenchmarkContext& context)
		{
			HandlerType handler = SignalsDesc_::MakeHandler();
			SignalType s;
			StorageArray<ConnectionType> c(context.GetIterationsCount());

			c.Construct([&]{ return s.connect(handler); });
			context.MeasureMemory("handler", context.GetIterationsCount());
			c.Destruct();
		}

		static void Invoke(BenchmarkContext& context, int64_t numSlots)
		{
			const auto n = context.GetIterationsCount();

			HandlerType handler = SignalsDesc_::MakeHandler();
			SignalType s;
			StorageArray<ConnectionType> c(numSlots);

			c.Construct([&]{ return s.connect(handler); });

			{
				auto op = context.Profile("invoking", numSlots * n);
				for (int64_t i = 0; i < n; ++i)
					s();
			}

			c.Destruct();
		}

		static void Connect(BenchmarkContext& context, int64_t numSlots)
		{
			const auto n = context.GetIterationsCount();

			HandlerType handler = SignalsDesc_::MakeHandler();
			std::vector<SignalType> s(n);
			StorageArray<ConnectionType> c(numSlots * n);

			{
				auto op = context.Profile("connecting", numSlots * n);
				for (int64_t j = 0; j < n; ++j)
					for (int64_t i = 0; i < numSlots; ++i)
						c[i + j * numSlots].Construct(s[j].connect(handler));
			}

			context.Profile("disconnecting", numSlots * n, [&]{ c.Destruct(); });
		}
	};

}

#endif
