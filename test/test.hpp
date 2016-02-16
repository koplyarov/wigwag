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
#include <wigwag/thread_task_executor.hpp>
#include <wigwag/token_pool.hpp>

#include <cxxtest/TestSuite.h>

#include <chrono>
#include <iostream>
#include <thread>

#include <test/test_utils.hpp>


#define LOCK_GUARD(...) std::lock_guard<decltype(__VA_ARGS__)> l(__VA_ARGS__);

using namespace wigwag;


class wigwag_tests : public CxxTest::TestSuite
{
public:
	static void test_exception_handling()
	{
		signal<void()> ds;
		signal<void(), exception_handling::none> rs;
		signal<void(), exception_handling::print_to_stderr> ns;

		token_pool tp;
		tp += ds.connect([&] { throw std::runtime_error("Test exception"); });
		tp += rs.connect([&] { throw 0; });
		tp += ns.connect([&] { throw std::runtime_error("Test exception"); });

		TS_ASSERT_THROWS(ds(), std::runtime_error);
		TS_ASSERT_THROWS_ANYTHING(rs());
		TS_ASSERT_THROWS_NOTHING(ns());

		tp.release();
	}


	static void test_threading()
	{
		signal<void()> ds;
		signal<void(), exception_handling::none, threading::own_recursive_mutex> rms;
		signal<void(), exception_handling::none, threading::own_mutex> ms;
		signal<void(), exception_handling::none, threading::none> ns;

		TS_ASSERT_THROWS_NOTHING(ds());
		TS_ASSERT_THROWS_NOTHING(rms());
		TS_ASSERT_THROWS(ms(), std::runtime_error);
		TS_ASSERT_THROWS_NOTHING(ns());

		{
			LOCK_GUARD(ds.lock_primitive());
			TS_ASSERT_THROWS_NOTHING(ds());
		}

		{
			LOCK_GUARD(rms.lock_primitive());
			TS_ASSERT_THROWS_NOTHING(rms());
		}

		{
			LOCK_GUARD(ms.lock_primitive());
			TS_ASSERT_THROWS_NOTHING(ms());
		}

		static_assert(std::is_same<decltype(ns.lock_primitive()), void>::value, "threading::none::get_primitive() return type should be void!");
	}


	static void test_populators()
	{
		using h_type = const std::function<void(int)>&;

		signal<void(int)> ds([](h_type h){ h(1); } );
		signal<void(int), exception_handling::none, threading::none, state_populating::populator_only> ps([](h_type h){ h(2); } );
		signal<void(int), exception_handling::none, threading::none, state_populating::populator_and_withdrawer> pws(state_populating::populator_and_withdrawer::handler_processor<void(int)>([](h_type h){ h(3); }, [](h_type h){ h(4); }));
		signal<void(int), exception_handling::none, threading::none, state_populating::none> ns;

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


	static void test_life_assurance()
	{
		signal<void()> s;

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


	static void test_task_executors()
	{
		signal<void()> s;

		std::mutex m;
		std::thread::id handler_thread_id;

		std::shared_ptr<task_executor> worker = std::make_shared<thread_task_executor>();
		token t = s.connect(worker, [&]{
				std::lock_guard<std::mutex> l(m);
				handler_thread_id = std::this_thread::get_id();
			});

		s();

		std::this_thread::sleep_for(std::chrono::milliseconds(500));

		std::lock_guard<std::mutex> l(m);
		TS_ASSERT_DIFFERS(handler_thread_id, std::thread::id());
		TS_ASSERT_DIFFERS(handler_thread_id, std::this_thread::get_id());
	}
};

#endif
