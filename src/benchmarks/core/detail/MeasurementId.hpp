#ifndef BENCHMARKS_CORE_DETAIL_MEASUREMENTID_HPP
#define BENCHMARKS_CORE_DETAIL_MEASUREMENTID_HPP

// Copyright (c) 2016, Dmitry Koplyarov <koplyarov.da@gmail.com>
//
// Permission to use, copy, modify, and/or distribute this software for any purpose with or without fee is hereby granted,
// provided that the above copyright notice and this permission notice appear in all copies.
//
// THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS.
// IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS,
// WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.


#include <benchmarks/core/detail/ParameterizedBenchmarkId.hpp>


namespace benchmarks
{

	class MeasurementId
	{
	private:
		ParameterizedBenchmarkId	_benchmarkId;
		std::string					_measurementLocalId;

	public:
		MeasurementId() { }

		MeasurementId(ParameterizedBenchmarkId benchmarkId, std::string measurementLocalId)
			: _benchmarkId(benchmarkId), _measurementLocalId(measurementLocalId)
		{ }

		ParameterizedBenchmarkId GetBenchmarkId() const { return _benchmarkId; }
		std::string GetMeasurementLocalId() const { return _measurementLocalId; }

		std::string ToString() const
		{ return _benchmarkId.ToString() + "[" + _measurementLocalId + "]"; }

		bool operator < (const MeasurementId& other) const
		{ return std::tie(_benchmarkId, _measurementLocalId) < std::tie(other._benchmarkId, other._measurementLocalId); }
	};

}

#endif
