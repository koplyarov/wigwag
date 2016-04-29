#ifndef BENCHMARKS_CORE_DETAIL_PARAMETERIZEDBENCHMARKID_HPP
#define BENCHMARKS_CORE_DETAIL_PARAMETERIZEDBENCHMARKID_HPP

// Copyright (c) 2016, Dmitry Koplyarov <koplyarov.da@gmail.com>
//
// Permission to use, copy, modify, and/or distribute this software for any purpose with or without fee is hereby granted,
// provided that the above copyright notice and this permission notice appear in all copies.
//
// THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS.
// IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS,
// WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.


#include <benchmarks/core/Benchmark.hpp>
#include <benchmarks/core/detail/BenchmarkId.hpp>


namespace benchmarks
{

	class ParameterizedBenchmarkId
	{
	private:
		BenchmarkId				_id;
		SerializedParamsMap		_params;

	public:
		ParameterizedBenchmarkId()
		{ }

		ParameterizedBenchmarkId(BenchmarkId id, SerializedParamsMap params)
			: _id(std::move(id)), _params(std::move(params))
		{ }

		BenchmarkId GetId() const { return _id; }
		const SerializedParamsMap& GetParams() const { return _params; }

		std::string ToString() const
		{
			std::stringstream s;
			s << _id.ToString();
			if (!_params.empty())
			{
				s << "(";
				for (auto it = _params.begin(); it != _params.end(); ++it)
					s << (it == _params.begin() ? "" : ", ") << it->first << ":" << it->second;
				s << ")";
			}
			return s.str();
		}

		bool operator < (const ParameterizedBenchmarkId& other) const
		{ return std::tie(_id, _params) < std::tie(other._id, other._params); }
	};

}

#endif
