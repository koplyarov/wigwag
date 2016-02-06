#ifndef TEST_TEST_HPP
#define TEST_TEST_HPP

// Copyright (c) 2016, Dmitry Koplyarov <koplyarov.da@gmail.com>
//
// Permission to use, copy, modify, and/or distribute this software for any purpose with or without fee is hereby granted,
// provided that the above copyright notice and this permission notice appear in all copies.
//
// THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS.
// IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS,
// WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.


#include <wigwag/signal.hpp>

#include <cxxtest/TestSuite.h>

#include <chrono>
#include <iostream>
#include <thread>

#include <test/test_utils.hpp>


using namespace wigwag;


struct exception_handling_nop
{
	void handle_std_exception(const std::exception& ex) const { }
	void handle_unknown_exception() const { }
};


class wigwag_tests : public CxxTest::TestSuite
{
public:
	static void test_default_life_assurance() { do_test_life_assurance<signal>(); }

	static void test_exception_handling()
	{
		signal<void()> ds;
		basic_signal<void(), exception_handling::rethrow> rs;
		basic_signal<void(), exception_handling_nop> ns;

		token t0 = ds.connect([&] { throw std::runtime_error("Test exception"); });
		token t2 = rs.connect([&] { throw 0; });
		token t3 = ns.connect([&] { throw std::runtime_error("Test exception"); });

		TS_ASSERT_THROWS(ds(), std::runtime_error);
		TS_ASSERT_THROWS_ANYTHING(rs());
		TS_ASSERT_THROWS_NOTHING(ns());
	}

	static void test_populators()
	{
		using h_type = const std::function<void(int)>&;

		signal<void(int)> ds([](h_type h){ h(1); } );
		basic_signal<void(int), exception_handling::rethrow, threading::none, state_populating::populator_only> ps([](h_type h){ h(2); } );
		basic_signal<void(int), exception_handling::rethrow, threading::none, state_populating::populator_and_withdrawer> pws(state_populating::populator_and_withdrawer::handler_processor<void(int)>([](h_type h){ h(3); }, [](h_type h){ h(4); }));
		basic_signal<void(int), exception_handling::rethrow, threading::none, state_populating::none> ns;

		int state = 0;
		ds.connect([&](int i) { state = i; });
		TS_ASSERT_EQUALS(state, 1);

		state = 0;
		ps.connect([&](int i) { state = i; });
		TS_ASSERT_EQUALS(state, 2);

		{
			state = 0;
			token t = pws.connect([&](int i) { state = i; });
			TS_ASSERT_EQUALS(state, 3);
			state = 0;
		}
		TS_ASSERT_EQUALS(state, 4);

		state = 0;
		ns.connect([&](int i) { state = i; });
		TS_ASSERT_EQUALS(state, 0);
	}

private:
	template < template<typename> class Signal_ >
	static void do_test_life_assurance()
	{
		Signal_<void()> s;

		thread t(
			[&](const std::atomic<bool>& alive)
			{
				while (alive)
				{
					s();
					std::this_thread::sleep_for(std::chrono::milliseconds(100));
				}
			}
		);

		profiler p;
		{
			token t = s.connect([]{ std::this_thread::sleep_for(std::chrono::seconds(2)); });
			std::this_thread::sleep_for(std::chrono::milliseconds(500));
			p.reset();
		}
		TS_ASSERT_LESS_THAN(std::chrono::seconds(1), p.reset());
	}
};

#endif
