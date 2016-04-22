// Copyright (c) 2016, Dmitry Koplyarov <koplyarov.da@gmail.com>
//
// Permission to use, copy, modify, and/or distribute this software for any purpose with or without fee is hereby granted,
// provided that the above copyright notice and this permission notice appear in all copies.
//
// THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS.
// IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS,
// WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.


#include <benchmarks/core/BenchmarkApp.hpp>
#include <benchmarks/core/ipc/MessageQueue.hpp>
#include <benchmarks/core/utils/ThreadPriority.hpp>

#include <boost/program_options.hpp>
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
			using namespace boost::program_options;

			std::string subtask, queue_name = "wigwagMessageQueue", benchmark;
			int64_t num_iterations = 0;
			std::vector<std::string> params_vec;

			options_description od;
			od.add_options()
				("help", "Show help")
				("subtask", value<std::string>(&subtask), "Internal option")
				("queue", value<std::string>(&queue_name), "Internal option")
				("iterations", value<int64_t>(&num_iterations), "Internal option")
				("benchmark", value<std::string>(&benchmark), "Benchmark id")
				("params", value<std::vector<std::string>>(&params_vec)->multitoken(), "Benchmark id")
				;

			positional_options_description pd;
			pd.add("benchmark", 1).add("params", -1);

			parsed_options parsed = command_line_parser(argc, argv).options(od).positional(pd).run();

			variables_map vm;
			store(parsed, vm);
			notify(vm);

			if (benchmark.empty() || vm.count("help"))
			{
				std::cerr << boost::lexical_cast<std::string>(od) << std::endl;
				return 0;
			}

			boost::regex benchmark_re(R"(([^.]+)\.([^.]+)\.([^.]+))");
			boost::smatch m;
			if (!boost::regex_match(benchmark, m, benchmark_re))
				throw CmdLineException("Could not parse benchmark id!");

			boost::regex param_re(R"(([^.]+):([^.]+))");
			std::map<std::string, SerializedParam> params;
			for (auto&& param_str : params_vec)
			{
				boost::smatch m;
				if (!boost::regex_match(param_str, m, param_re))
					throw CmdLineException("Could not parse parameter!");
				params[m[1]] = m[2];
			}

			if (subtask == "measureIterationsCount")
			{
				MessageQueue mq(queue_name);
				mq.SendMessage(std::make_shared<IterationsCountMessage>(_suite.MeasureIterationsCount({m[1], m[2], m[3]}, params)));
				return 0;
			}
			else if (subtask == "invokeBenchmark")
			{
				if (vm.count("iterations") == 0)
					throw CmdLineException("Number of iterations is not specified!");
				MessageQueue mq(queue_name);
				SetMaxThreadPriority();
				_suite.InvokeBenchmark(num_iterations, {m[1], m[2], m[3]}, params);
				return 0;
			}

			if (!subtask.empty())
				throw CmdLineException("Unknown subtask!");

			MessageQueue::Remove(queue_name);
			MessageQueue mq(queue_name);

			{
				std::stringstream cmd;
				cmd << argv[0] << " --subtask measureIterationsCount --queue " << queue_name << " " << benchmark;
				for (auto&& p : params_vec)
					cmd << " " << p;

				if (system(cmd.str().c_str()) != 0)
					throw std::runtime_error(cmd.str() + " failed!");
			}

			{
				auto it_msg = mq.ReceiveMessage<IterationsCountMessage>();
				std::stringstream cmd;
				cmd << argv[0] << " --subtask invokeBenchmark --queue " << queue_name << " --iterations " << it_msg->GetCount() << " " << benchmark;
				for (auto&& p : params_vec)
					cmd << " " << p;

				if (system(cmd.str().c_str()) != 0)
					throw std::runtime_error(cmd.str() + " failed!");
			}

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
