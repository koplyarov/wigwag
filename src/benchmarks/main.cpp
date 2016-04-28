// Copyright (c) 2016, Dmitry Koplyarov <koplyarov.da@gmail.com>
//
// Permission to use, copy, modify, and/or distribute this software for any purpose with or without fee is hereby granted,
// provided that the above copyright notice and this permission notice appear in all copies.
//
// THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS.
// IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS,
// WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.


#include <benchmarks/FunctionsBenchmarks.hpp>
#include <benchmarks/GenericBenchmarks.hpp>
#include <benchmarks/MutexBenchmarks.hpp>
#include <benchmarks/SignalsBenchmarks.hpp>
#include <benchmarks/core/BenchmarkApp.hpp>
#include <benchmarks/core/BenchmarkSuite.hpp>
#include <benchmarks/descriptors/functions/boost.hpp>
#include <benchmarks/descriptors/functions/std.hpp>
#include <benchmarks/descriptors/generic/boost.hpp>
#include <benchmarks/descriptors/generic/std.hpp>
#include <benchmarks/descriptors/generic/wigwag.hpp>
#include <benchmarks/descriptors/mutex/boost.hpp>
#include <benchmarks/descriptors/mutex/std.hpp>
#include <benchmarks/descriptors/signals/boost.hpp>
#include <benchmarks/descriptors/signals/qt5.hpp>
#include <benchmarks/descriptors/signals/sigcpp.hpp>
#include <benchmarks/descriptors/signals/wigwag.hpp>

#include <iostream>


int main(int argc, char* argv[])
{
	try
	{
		using namespace benchmarks;
		using namespace descriptors;

		BenchmarkSuite s;
		s.RegisterBenchmarks<SignalsBenchmarks,
			signals::wigwag::regular,
			signals::wigwag::ui,
			signals::boost::regular,
			signals::boost::tracking,
#if WIGWAG_BENCHMARKS_SIGCPP2
			signals::sigcpp::regular,
#endif
#if WIGWAG_BENCHMARKS_QT5
			signals::qt5::regular
#endif
			>();

		s.RegisterBenchmarks<FunctionsBenchmarks,
			functions::std::regular,
			functions::boost::regular>();

		s.RegisterBenchmarks<MutexBenchmarks,
			mutex::std::mutex,
			mutex::std::recursive_mutex,
			mutex::boost::mutex,
			mutex::boost::recursive_mutex
			>();

		s.RegisterBenchmarks<GenericBenchmarks,
			generic::std::condition_variable,
			generic::boost::condition_variable,
			generic::wigwag::life_token
			>();

		return BenchmarkApp(s).Run(argc, argv);
	}
	catch (const std::exception& ex)
	{
		std::cerr << "Uncaught exception: " << ex.what() << std::endl;
		return 1;
	}
	return 0;
}
