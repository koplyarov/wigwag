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
	public:
		SignalsBenchmarks()
			: BenchmarksClass("signals")
		{
			AddBenchmark<>("create", &SignalsBenchmarks::Create);
			AddBenchmark<int64_t>("connectInvoke", &SignalsBenchmarks::ConnectInvoke, {"numSlots"});
			//AddBenchmark("connect_disconnect", &SignalsBenchmarks::ConnectDisconnect);
		}

	private:
		static void Create(BenchmarkContext& context)
		{
			using signal_type = typename SignalsDesc_::signal_type;

			const auto n = context.GetIterationsCount();

			StorageFor<signal_type> *v = new StorageFor<signal_type>[(size_t)n];

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

		static void ConnectInvoke(BenchmarkContext& context, int64_t numSlots)
		{
			using signal_type = typename SignalsDesc_::signal_type;
			using handler_type = typename SignalsDesc_::handler_type;
			using connection_type = typename SignalsDesc_::connection_type;

			const auto n = context.GetIterationsCount();
			handler_type handler = SignalsDesc_::make_handler();
			signal_type s;
			StorageFor<connection_type> *c = new StorageFor<connection_type>[(size_t)numSlots];

			{
				auto op = context.Profile("connecting", numSlots);
				for (int64_t i = 0; i < numSlots; ++i)
					c[i].Construct(s.connect(handler));
			}

			context.MeasureMemory("connection", numSlots);

			{
				auto op = context.Profile("invoking", numSlots * n);
				for (int64_t i = 0; i < n; ++i)
					s();
			}

			{
				auto op = context.Profile("disconnecting", numSlots);
				for (int64_t i = 0; i < numSlots; ++i)
					c[i].Destruct();
			}

			delete[] c;
		}

		//static void ConnectDisconnect(int64_t num_slots, int64_t num_iterations)
		//{ throw std::runtime_error("connect_disconnect"); }
	};

}

#endif
