#ifndef BENCHMARKS_DETAIL_BENCHMARKID_HPP
#define BENCHMARKS_DETAIL_BENCHMARKID_HPP

// Copyright (c) 2016, Dmitry Koplyarov <koplyarov.da@gmail.com>
//
// Permission to use, copy, modify, and/or distribute this software for any purpose with or without fee is hereby granted,
// provided that the above copyright notice and this permission notice appear in all copies.
//
// THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS.
// IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS,
// WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.


#include <string>
#include <tuple>


namespace benchmarks
{

	class BenchmarkId
	{
	private:
		std::string		_className;
		std::string		_benchmarkName;
		std::string		_objectName;

	public:
		BenchmarkId()
		{ }

		BenchmarkId(std::string className, std::string benchmarkName, std::string objectName)
			: _className(std::move(className)), _benchmarkName(std::move(benchmarkName)), _objectName(std::move(objectName))
		{ }

		std::string GetClassName() const		{ return _className; }
		std::string GetBenchmarkName() const	{ return _benchmarkName; }
		std::string GetObjectName() const		{ return _objectName; }

		bool operator < (const BenchmarkId& other) const
		{ return std::tie(_className, _benchmarkName, _objectName) < std::tie(other._className, other._benchmarkName, other._objectName); }

		std::string ToString() const
		{ return _className + "." + _benchmarkName + "." + _objectName; }
	};

}

#endif
