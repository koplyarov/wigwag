#ifndef BENCHMARKS_MARKERS_HPP
#define BENCHMARKS_MARKERS_HPP

// Copyright (c) 2016, Dmitry Koplyarov <koplyarov.da@gmail.com>
//
// Permission to use, copy, modify, and/or distribute this software for any purpose with or without fee is hereby granted,
// provided that the above copyright notice and this permission notice appear in all copies.
//
// THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS.
// IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS,
// WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.


#include <boost/core/typeinfo.hpp>

#include <chrono>
#include <iostream>
#include <thread>

#include <utils/profiler.hpp>

#if defined(_MSC_VER)
#	include <intrin.h>
#endif


namespace wigwag
{

#if defined(__GNUC__)
#	define WIGWAG_BARRIER asm volatile ("":::"memory")
#elif defined(_MSC_VER)
#	define WIGWAG_BARRIER _ReadWriteBarrier()
#else
#	define WIGWAG_BARRIER do { std::mutex m; std::lock_guard<std::mutex> l(m); } while (false)
#endif

	class operation_profiler
	{
	private:
		std::string		_name;
		int64_t			_count;
		profiler		_prof;

	public:
		operation_profiler(const std::string& name, int64_t count)
			: _name(name), _count(count)
		{
			std::cout << "<" << _name << ">" << std::endl;
			std::this_thread::sleep_for(std::chrono::milliseconds(200));
			WIGWAG_BARRIER;
			_prof.reset();
			WIGWAG_BARRIER;
		}

		~operation_profiler()
		{
			WIGWAG_BARRIER;
			auto d = _prof.reset();
			WIGWAG_BARRIER;
			auto d_ms = std::chrono::duration_cast<std::chrono::milliseconds>(d);
			auto d_ns = std::chrono::duration_cast<std::chrono::nanoseconds>(d);

			if (d_ms.count() > 1)
				std::cout << "<" << _name << " finished: " << d_ns.count() / _count << ">" << std::endl;
			else
				std::cout << "<" << _name << " finished>" << std::endl;
		}
	};


	inline void measure_memory(const std::string& name, int64_t count)
	{
		if (count < 300000)
			return;

		std::cout << "<measure memory, name: " << name << ", count: " << count << ">" << std::endl;
		std::this_thread::sleep_for(std::chrono::seconds(1));
	}

	template < typename T_ >
	inline void measure_memory(int64_t count)
	{
		measure_memory(boost::core::demangled_name(BOOST_CORE_TYPEID(T_)), count);
	}

}

#endif
