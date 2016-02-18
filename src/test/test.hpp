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

#include <test/profiler.hpp>
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


	static void test_populators()
	{
		using h_type = const std::function<void(int)>&;

		{
			int signal_state = 1;
			signal<void(int)> s([&](h_type h){ h(signal_state); } );
			thread th(
				[&](const std::atomic<bool>& alive)
				{
					sleep_ms(500);
					auto l = lock(s.lock_primitive());
					signal_state = 2;
					s(2);
				}
			);

			mutexed<int> state;
			token t = s.connect([&](int i) { state.set(i); });
			TS_ASSERT_EQUALS(state.get(), 1);
			sleep_ms(1000);
			TS_ASSERT_EQUALS(state.get(), 2);
			t.reset();
			TS_ASSERT_EQUALS(state.get(), 2);
		}

		{
			int signal_state = 1;
			signal<void(int), exception_handling::default_, threading::default_, state_populating::populator_and_withdrawer> s(std::make_pair([](h_type h){ h(1); }, [](h_type h){ h(3); }));

			thread th(
				[&](const std::atomic<bool>& alive)
				{
					sleep_ms(500);
					auto l = lock(s.lock_primitive());
					signal_state = 2;
					s(2);
				}
			);

			mutexed<int> state;
			token t = s.connect([&](int i) { state.set(i); });
			TS_ASSERT_EQUALS(state.get(), 1);
			sleep_ms(1000);
			TS_ASSERT_EQUALS(state.get(), 2);
			t.reset();
			TS_ASSERT_EQUALS(state.get(), 3);
		}

		{
			int signal_state = 0;
			signal<void(int), exception_handling::default_, threading::default_, state_populating::none> s;
			thread th(
				[&](const std::atomic<bool>& alive)
				{
					sleep_ms(500);
					auto l = lock(s.lock_primitive());
					signal_state = 2;
					s(2);
				}
			);

			mutexed<int> state;
			token t = s.connect([&](int i) { state.set(i); });
			TS_ASSERT_EQUALS(state.get(), 0);
			sleep_ms(1000);
			TS_ASSERT_EQUALS(state.get(), 2);
			t.reset();
			TS_ASSERT_EQUALS(state.get(), 2);
		}
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
