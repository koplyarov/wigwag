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
#include <benchmarks/markers.hpp>
#include <utils/storage_for.hpp>


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
			//AddBenchmark("connect_invoke", &SignalsBenchmarks::ConnectInvoke);
			//AddBenchmark("connect_disconnect", &SignalsBenchmarks::ConnectDisconnect);
		}

	private:
		static void Create(BenchmarkContext& context)
		{
			using namespace wigwag;
			using signal_type = typename SignalsDesc_::signal_type;

			const auto n = context.GetIterationsCount();

			storage_for<signal_type> *v = new storage_for<signal_type>[(size_t)n];

			{
				auto op = context.Profile("creating", n);
				for (int64_t i = 0; i < n; ++i)
					new(&v[i].obj) signal_type();
			}

			context.MeasureMemory("object", n);

			{
				auto op = context.Profile("destroying", n);
				for (int64_t i = 0; i < n; ++i)
					v[i].obj.~signal_type();
			}

			delete[] v;
		}

		//static void ConnectInvoke(int64_t num_slots, int64_t num_calls)
		//{ throw std::runtime_error("connect_invoke"); }

		//static void ConnectDisconnect(int64_t num_slots, int64_t num_iterations)
		//{ throw std::runtime_error("connect_disconnect"); }
	};

}

#endif
