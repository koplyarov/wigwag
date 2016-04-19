#ifndef BENCHMARKS_BENCHMARKCLASS_HPP
#define BENCHMARKS_BENCHMARKCLASS_HPP

// Copyright (c) 2016, Dmitry Koplyarov <koplyarov.da@gmail.com>
//
// Permission to use, copy, modify, and/or distribute this software for any purpose with or without fee is hereby granted,
// provided that the above copyright notice and this permission notice appear in all copies.
//
// THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS.
// IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS,
// WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.


#include <string>
#include <vector>

#include <benchmarks/Benchmark.hpp>


namespace benchmarks
{

	class BenchmarksClass
	{
	private:
		std::string						_name;
		std::vector<IBenchmarkPtr>		_benchmarks;

	public:
		BenchmarksClass(std::string name)
			: _name(std::move(name))
		{ }

		std::string GetName() const { return _name; }
		const std::vector<IBenchmarkPtr>& GetBenchmarks() const { return _benchmarks; }

	protected:
		template < typename... BenchmarkParams_, typename Functor_ >
		void AddBenchmark(std::string name, Functor_ benchmarkFunc, std::vector<std::string> orderedParamNames = {})
		{ _benchmarks.push_back(std::make_shared<Benchmark<BenchmarkParams_...>>(std::move(name), std::move(benchmarkFunc), std::move(orderedParamNames))); }
	};


}

#endif
