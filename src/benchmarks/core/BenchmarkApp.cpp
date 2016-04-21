// Copyright (c) 2016, Dmitry Koplyarov <koplyarov.da@gmail.com>
//
// Permission to use, copy, modify, and/or distribute this software for any purpose with or without fee is hereby granted,
// provided that the above copyright notice and this permission notice appear in all copies.
//
// THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS.
// IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS,
// WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.


#include <benchmarks/core/BenchmarkApp.hpp>
#include <benchmarks/core/utils/ThreadPriority.hpp>

#include <boost/regex.hpp>


namespace benchmarks
{

	BenchmarkApp::BenchmarkApp(const BenchmarkSuite& suite)
		: _suite(suite)
	{ }


	namespace
	{
		struct CmdLineException : public std::runtime_error
		{ CmdLineException(const std::string& msg) : std::runtime_error(msg) { } };
	}

	int BenchmarkApp::Run(int argc, char* argv[])
	{
		try
		{
			if (argc <= 1)
			{
				std::cerr << "TODO: show help" << std::endl;
				return 0;
			}

			std::string benchmark(argv[1]);
			if (benchmark.empty())
				throw CmdLineException("Benchmark not specified!");

			boost::regex benchmark_re(R"(([^.]+)\.([^.]+)\.([^.]+))");
			boost::smatch m;
			if (!boost::regex_match(benchmark, m, benchmark_re))
				throw CmdLineException("Could not parse benchmark id!");

			boost::regex param_re(R"(([^.]+):([^.]+))");
			std::map<std::string, SerializedParam> params;
			for (int i = 2; i < argc; ++i)
			{
				std::string param_str(argv[i]);
				boost::smatch m;
				if (!boost::regex_match(param_str, m, param_re))
					throw CmdLineException("Could not parse parameter!");
				params[m[1]] = m[2];
			}

			SetMaxThreadPriority();
			auto num_iterations = _suite.MeasureIterationsCount({m[1], m[2], m[3]}, params);
			_suite.InvokeBenchmark(num_iterations, {m[1], m[2], m[3]}, params);

			return 0;
		}
		catch (const CmdLineException& ex)
		{
			std::cerr << ex.what() << std::endl;
			return 1;
		}
		catch (const std::exception& ex)
		{
			std::cerr << "Uncaught exception: " << ex.what() << std::endl;
			return 1;
		}
	}

}
