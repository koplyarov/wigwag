#ifndef BENCHMARKS_CORE_DETAIL_BENCHMARKRESULT_HPP
#define BENCHMARKS_CORE_DETAIL_BENCHMARKRESULT_HPP

// Copyright (c) 2016, Dmitry Koplyarov <koplyarov.da@gmail.com>
//
// Permission to use, copy, modify, and/or distribute this software for any purpose with or without fee is hereby granted,
// provided that the above copyright notice and this permission notice appear in all copies.
//
// THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS.
// IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS,
// WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.


#include <boost/serialization/nvp.hpp>

#include <map>
#include <string>


namespace benchmarks
{

	class BenchmarkResult
	{
	public:
		using OperationTimesMap = std::map<std::string, double>;
		using MemoryConsumptionMap = std::map<std::string, int64_t>;

	private:
		OperationTimesMap		_operationTimes;
		MemoryConsumptionMap	_memoryConsumption;

	public:
		BenchmarkResult() { }

		BenchmarkResult(OperationTimesMap operationTimes, MemoryConsumptionMap memoryConsumption)
			: _operationTimes(std::move(operationTimes)), _memoryConsumption(std::move(memoryConsumption))
		{ }

		const OperationTimesMap& GetOperationTimes() const { return _operationTimes; }
		const MemoryConsumptionMap& GetMemoryConsumption() const { return _memoryConsumption; }

		void Update(const BenchmarkResult& other);

		template<class Archive>
		void serialize(Archive &ar, const unsigned int version)
		{ ar & BOOST_SERIALIZATION_NVP(_operationTimes) & BOOST_SERIALIZATION_NVP(_memoryConsumption); }
	};

}

#endif
