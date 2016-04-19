#include <benchmarks/BenchmarkSuite.hpp>


namespace benchmarks
{

	class BenchmarkSuite::PreMeasureBenchmarkContext : public BenchmarkContext
	{
		using DurationsMap = std::map<std::string, std::chrono::milliseconds>;

		class OperationProfiler : public IOperationProfiler
		{
		private:
			PreMeasureBenchmarkContext*		_inst;
			std::string						_name;
			int64_t							_count;
			wigwag::profiler				_prof;

		public:
			OperationProfiler(PreMeasureBenchmarkContext* inst, const std::string& name, int64_t count)
				: _inst(inst), _name(name), _count(count)
			{ _prof.reset(); }

			~OperationProfiler()
			{
				auto d = _prof.reset();
				_inst->_durations.insert({_name, std::chrono::duration_cast<std::chrono::milliseconds>(d)});
			}
		};

	private:
		DurationsMap		_durations;

	public:
		PreMeasureBenchmarkContext(int64_t iterationsCount)
			: BenchmarkContext(iterationsCount)
		{ }

		const DurationsMap& GetDurationsMap() const { return _durations; }

		virtual void MeasureMemory(const std::string& name, int64_t count) const
		{ }

		virtual IOperationProfilerPtr Profile(const std::string& name, int64_t count)
		{ return std::make_shared<OperationProfiler>(this, name, count); }
	};


	class BenchmarkSuite::MeasureBenchmarkContext : public BenchmarkContext
	{
		class OperationProfiler : public IOperationProfiler
		{
		private:
			wigwag::operation_profiler	_p;

		public:
			OperationProfiler(const std::string& name, int64_t count) : _p(name, count) { }
		};

	public:
		MeasureBenchmarkContext(int64_t iterationsCount)
			: BenchmarkContext(iterationsCount)
		{ }

		virtual void MeasureMemory(const std::string& name, int64_t count) const
		{ wigwag::measure_memory(name, count); }

		virtual IOperationProfilerPtr Profile(const std::string& name, int64_t count)
		{ return std::make_shared<OperationProfiler>(name, count); }
	};


	void BenchmarkSuite::InvokeBenchmark(const BenchmarkId& id, const SerializedParamsMap& serializedParams)
	{
		auto it = _benchmarks.find(id);
		if (it == _benchmarks.end())
			throw std::runtime_error("Benchmark " + id.ToString() + " not found!");

		int64_t num_iterations = 1;
		while (true)
		{
			PreMeasureBenchmarkContext ctx(num_iterations);
			it->second->Perform(ctx, serializedParams);

			bool enough_iterations = true;
			for (auto p : ctx.GetDurationsMap())
				if (p.second < std::chrono::milliseconds(300))
				{
					enough_iterations = false;
					break;
				}

			if (enough_iterations)
				break;

			num_iterations *= 10;
		}

		MeasureBenchmarkContext ctx(num_iterations);
		it->second->Perform(ctx, serializedParams);
	}

}
