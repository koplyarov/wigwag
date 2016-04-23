// Copyright (c) 2016, Dmitry Koplyarov <koplyarov.da@gmail.com>
//
// Permission to use, copy, modify, and/or distribute this software for any purpose with or without fee is hereby granted,
// provided that the above copyright notice and this permission notice appear in all copies.
//
// THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS.
// IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS,
// WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.


#include <benchmarks/core/BenchmarkSuite.hpp>

#include <algorithm>
#include <iostream>
#include <thread>

#include <benchmarks/core/utils/Barrier.hpp>
#include <benchmarks/core/utils/Memory.hpp>
#include <benchmarks/core/utils/Profiler.hpp>


namespace benchmarks
{

	using namespace std::chrono;

	class BenchmarkSuite::PreMeasureBenchmarkContext : public BenchmarkContext
	{
	public:
		using DurationsMap = std::map<std::string, nanoseconds>;

	private:
		class OperationProfiler : public IOperationProfiler
		{
		private:
			PreMeasureBenchmarkContext*		_inst;
			std::string						_name;
			int64_t							_count;
			Profiler						_prof;

		public:
			OperationProfiler(PreMeasureBenchmarkContext* inst, const std::string& name, int64_t count)
				: _inst(inst), _name(name), _count(count)
			{
				BENCHMARKS_BARRIER;
				_prof.Reset();
				BENCHMARKS_BARRIER;
			}

			~OperationProfiler()
			{
				BENCHMARKS_BARRIER;
				auto d = _prof.Reset();
				BENCHMARKS_BARRIER;
				_inst->_durations.insert({_name, duration_cast<nanoseconds>(d)});
			}
		};

	private:
		DurationsMap		_durations;
		int64_t				_maxRss;

	public:
		PreMeasureBenchmarkContext(int64_t iterationsCount)
			: BenchmarkContext(iterationsCount), _maxRss(0)
		{ }

		const DurationsMap& GetDurationsMap() const { return _durations; }
		int64_t GetMaxRss() const { return _maxRss; }

		virtual void MeasureMemory(const std::string& name, int64_t count)
		{
			BENCHMARKS_BARRIER;
			auto rss = Memory::GetRss();
			BENCHMARKS_BARRIER;
			_maxRss = std::max(rss, _maxRss);
		}

		virtual IOperationProfilerPtr Profile(const std::string& name, int64_t count)
		{ return std::make_shared<OperationProfiler>(this, name, count); }
	};


	////////////////////////////////////////////////////////////////////////////////


	class BenchmarkSuite::MeasureBenchmarkContext : public BenchmarkContext
	{
		class OperationProfiler : public IOperationProfiler
		{
		private:
			MeasureBenchmarkContext*		_inst;
			std::string						_name;
			int64_t							_count;
			Profiler						_prof;

		public:
			OperationProfiler(MeasureBenchmarkContext* inst, const std::string& name, int64_t count)
				: _inst(inst), _name(name), _count(count)
			{
				BENCHMARKS_BARRIER;
				_prof.Reset();
				BENCHMARKS_BARRIER;
			}

			~OperationProfiler()
			{
				BENCHMARKS_BARRIER;
				auto d = _prof.Reset();
				BENCHMARKS_BARRIER;
				auto ns = duration_cast<duration<double, std::nano>>(d).count();
				_inst->_resultsReporter->ReportOperationDuration(_name, ns / _count);
			}
		};

	private:
		IBenchmarksResultsReporterPtr		_resultsReporter;
		int64_t								_baselineRss;

	public:
		MeasureBenchmarkContext(int64_t iterationsCount, IBenchmarksResultsReporterPtr resultsReporter)
			: BenchmarkContext(iterationsCount), _resultsReporter(std::move(resultsReporter))
		{ _baselineRss = Memory::GetRss(); }

		virtual void MeasureMemory(const std::string& name, int64_t count)
		{
			BENCHMARKS_BARRIER;
			auto rss = Memory::GetRss() - _baselineRss;
			BENCHMARKS_BARRIER;
			_resultsReporter->ReportMemoryConsumption(name, rss / count);
		}

		virtual IOperationProfilerPtr Profile(const std::string& name, int64_t count)
		{ return std::make_shared<OperationProfiler>(this, name, count); }
	};


	////////////////////////////////////////////////////////////////////////////////


	BENCHMARKS_LOGGER(BenchmarkSuite);

	int64_t BenchmarkSuite::MeasureIterationsCount(const BenchmarkId& id, const SerializedParamsMap& serializedParams)
	{
		const int multiplier = 2;

		auto it = _benchmarks.find(id);
		if (it == _benchmarks.end())
			throw std::runtime_error("Benchmark " + id.ToString() + " not found!");

		int64_t total_mem = Memory::GetTotalPhys();
		int64_t num_iterations = 1;
		while (true)
		{
			PreMeasureBenchmarkContext ctx(num_iterations);
			it->second->Perform(ctx, serializedParams);

			using DurationsMapPair = PreMeasureBenchmarkContext::DurationsMap::value_type;
			auto& dm = ctx.GetDurationsMap();
			auto minmax_element = std::minmax_element(dm.begin(), dm.end(), [](const DurationsMapPair& l, const DurationsMapPair& r) { return l.second < r.second; } );
			auto min_duration = minmax_element.first->second;
			auto max_duration = minmax_element.second->second;

			auto max_rss = ctx.GetMaxRss();

			auto next_min_duration = min_duration * multiplier;
			auto next_max_duration = max_duration * multiplier;
			auto next_max_rss = max_rss * multiplier;

			if (max_duration > seconds(10))
			{
				s_logger.Warning() << "Max time limit exceeded!";
				for (auto p : ctx.GetDurationsMap())
					s_logger.Warning() << "  " << p.first << ": " << p.second.count() << " ms";
				break;
			}

			if (next_max_rss > total_mem * 1 / 2)
			{
				s_logger.Warning() << "Memory limit exceeded!";
				s_logger.Warning() << "  total mem: " << total_mem;
				s_logger.Warning() << "  max rss: " << max_rss;
				s_logger.Warning() << "  next max rss: " << next_max_rss;
				s_logger.Warning() << "durations:";
				for (auto p : ctx.GetDurationsMap())
					s_logger.Warning() << "  " << p.first << ": " << p.second.count() << " ms";
				break;
			}

			if (next_min_duration > milliseconds(100))
			{
				num_iterations *= multiplier;
				break;
			}

			num_iterations *= multiplier;
		}

		return num_iterations;
	}


	void BenchmarkSuite::InvokeBenchmark(int64_t iterations, const BenchmarkId& id, const SerializedParamsMap& serializedParams, const IBenchmarksResultsReporterPtr& resultsReporter)
	{
		s_logger.Debug() << "iterations: " << iterations;

		auto it = _benchmarks.find(id);
		if (it == _benchmarks.end())
			throw std::runtime_error("Benchmark " + id.ToString() + " not found!");

		MeasureBenchmarkContext ctx(iterations, resultsReporter);
		it->second->Perform(ctx, serializedParams);
	}

}
