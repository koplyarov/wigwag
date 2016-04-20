#ifndef BENCHMARKS_CORE_UTILS_PROFILER_HPP
#define BENCHMARKS_CORE_UTILS_PROFILER_HPP

// Copyright (c) 2016, Dmitry Koplyarov <koplyarov.da@gmail.com>
//
// Permission to use, copy, modify, and/or distribute this software for any purpose with or without fee is hereby granted,
// provided that the above copyright notice and this permission notice appear in all copies.
//
// THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS.
// IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS,
// WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.


#include <chrono>


namespace benchmarks
{

	template < typename ClockT_ >
	class BasicProfiler
	{
		using TimePoint = std::chrono::time_point<ClockT_>;

	private:
		TimePoint		_start;

	public:
		BasicProfiler() { _start = ClockT_::now(); }

		auto Reset() -> decltype(TimePoint() - TimePoint())
		{
			TimePoint end = ClockT_::now();
			auto delta = end - _start;
			_start = end;
			return delta;
		}
	};

	using Profiler = BasicProfiler<std::chrono::high_resolution_clock>;

}

#endif
