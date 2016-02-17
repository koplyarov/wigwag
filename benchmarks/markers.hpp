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


#include <chrono>
#include <iostream>
#include <thread>

#include <test/profiler.hpp>


namespace wigwag
{

	class operation_profiler
	{
	private:
		std::string		_name;
		profiler		_prof;

	public:
		operation_profiler(const std::string& name)
			: _name(name)
		{ std::cout << "<" << _name << ">" << std::endl; }

		~operation_profiler()
		{ std::cout << "<" << _name << " finished: " << std::chrono::duration_cast<std::chrono::nanoseconds>(_prof.reset()).count() << ">" << std::endl; }
	};


	static void measure_memory()
	{
		std::cout << "<measure memory>" << std::endl;
		std::this_thread::sleep_for(std::chrono::seconds(1));
	}

}

#endif
