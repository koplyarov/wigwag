#ifndef TEST_API_V1_TEST_HPP
#define TEST_API_V1_TEST_HPP

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
#include <wigwag/threadless_task_executor.hpp>
#include <wigwag/token_pool.hpp>

#include <cxxtest/TestSuite.h>

#include <chrono>
#include <iostream>
#include <thread>

#include <utils/mutexed.hpp>
#include <utils/profiler.hpp>
#include <utils/thread.hpp>


using namespace wigwag;
using namespace std::chrono;


class wigwag_api_v1_test : public CxxTest::TestSuite
{
public:
	static void test__exception_handling__default()
	{
		signal<void()> s;

		{
			token t = s.connect([&] { throw std::runtime_error("Test exception"); });
			TS_ASSERT_THROWS(s(), std::runtime_error);
		}

		{
			token t = s.connect([&] { throw 0; });
			TS_ASSERT_THROWS_ANYTHING(s());
		}
	}

	static void test__exception_handling__none()
	{
		signal<void(), exception_handling::none> s;

		{
			token t = s.connect([&] { throw std::runtime_error("Test exception"); });
			TS_ASSERT_THROWS(s(), std::runtime_error);
		}

		{
			token t = s.connect([&] { throw 0; });
			TS_ASSERT_THROWS_ANYTHING(s());
		}
	}


	static void test__exception_handling__print_to_stderr()
	{
		signal<void(), exception_handling::print_to_stderr> s;

		token_pool tp;
		tp += s.connect([&] { throw std::runtime_error("Test exception"); });
		tp += s.connect([&] { throw 0; });
		TS_ASSERT_THROWS_ANYTHING(s());

		std::shared_ptr<task_executor> worker = std::make_shared<basic_thread_task_executor<exception_handling::print_to_stderr> >();
		worker->add_task([]{ throw std::runtime_error("Test exception"); });
		thread::sleep(300);
	}

	////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	static void test__state_populating__default()
	{
		using h_type = const std::function<void(int)>&;

		int signal_state = 1;
		signal<void(int)> s([&](h_type h){ h(signal_state); } );
		thread th(
			[&](const std::atomic<bool>& alive)
			{
				thread::sleep(500);
				auto l = lock(s.lock_primitive());
				signal_state = 2;
				s(2);
			}
		);

		mutexed<int> state;
		token t = s.connect([&](int i) { state.set(i); });
		TS_ASSERT_EQUALS(state.get(), 1);
		thread::sleep(1000);
		TS_ASSERT_EQUALS(state.get(), 2);
		t.reset();
		TS_ASSERT_EQUALS(state.get(), 2);
	}

	static void test__state_populating__populator_only()
	{
		using h_type = const std::function<void(int)>&;

		int signal_state = 1;
		signal<void(int), exception_handling::default_, threading::default_, state_populating::populator_only> s([&](h_type h){ h(signal_state); } );
		thread th(
			[&](const std::atomic<bool>& alive)
			{
				thread::sleep(500);
				auto l = lock(s.lock_primitive());
				signal_state = 2;
				s(2);
			}
		);

		mutexed<int> state;
		token t = s.connect([&](int i) { state.set(i); });
		TS_ASSERT_EQUALS(state.get(), 1);
		thread::sleep(1000);
		TS_ASSERT_EQUALS(state.get(), 2);
		t.reset();
		TS_ASSERT_EQUALS(state.get(), 2);
	}

	static void test__state_populating__populator_and_withdrawer()
	{
		using h_type = const std::function<void(int)>&;

		int signal_state = 1;
		signal<void(int), exception_handling::default_, threading::default_, state_populating::populator_and_withdrawer> s(std::make_pair([](h_type h){ h(1); }, [](h_type h){ h(3); }));

		thread th(
			[&](const std::atomic<bool>& alive)
			{
				thread::sleep(500);
				auto l = lock(s.lock_primitive());
				signal_state = 2;
				s(2);
			}
		);

		mutexed<int> state;
		token t = s.connect([&](int i) { state.set(i); });
		TS_ASSERT_EQUALS(state.get(), 1);
		thread::sleep(1000);
		TS_ASSERT_EQUALS(state.get(), 2);
		t.reset();
		TS_ASSERT_EQUALS(state.get(), 3);
	}

	static void test__state_populating__none()
	{
		int signal_state = 0;
		signal<void(int), exception_handling::default_, threading::default_, state_populating::none> s;
		thread th(
			[&](const std::atomic<bool>& alive)
			{
				thread::sleep(500);
				auto l = lock(s.lock_primitive());
				signal_state = 2;
				s(2);
			}
		);

		mutexed<int> state;
		token t = s.connect([&](int i) { state.set(i); });
		TS_ASSERT_EQUALS(state.get(), 0);
		thread::sleep(1000);
		TS_ASSERT_EQUALS(state.get(), 2);
		t.reset();
		TS_ASSERT_EQUALS(state.get(), 2);
	}

	////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	static void test__life_assurance__default()
	{ do__test__life_assurance__common<signal<void()>>(); }

	static void test__life_assurance__intrusive_life_tokens()
	{ do__test__life_assurance__common<signal<void(), exception_handling::default_, threading::default_, state_populating::default_, life_assurance::intrusive_life_tokens>>(); }

	static void test__life_assurance__life_tokens()
	{ do__test__life_assurance__common<signal<void(), exception_handling::default_, threading::default_, state_populating::default_, life_assurance::life_tokens>>(); }

	template < typename Signal_ >
	static void do__test__life_assurance__common()
	{
		Signal_ s;

		thread t([&](const std::atomic<bool>& alive) { while (alive) { s(); thread::sleep(100); } });

		{
			mutexed<bool> handler_invoked(false);
			token t = s.connect([&]{ thread::sleep(1000); handler_invoked.set(true); });
			token other_t = s.connect([]{ thread::sleep(1000); });
			thread::sleep(300);
			profiler p;
			t.reset();
			auto disconnect_time = duration_cast<milliseconds>(p.reset()).count();
			TS_ASSERT(handler_invoked.get());
			TS_ASSERT_LESS_THAN_EQUALS(600, disconnect_time);
			TS_ASSERT_LESS_THAN_EQUALS(disconnect_time, 1200);
		}

		{
			mutexed<bool> handler_invoked(false);
			token other_t = s.connect([]{ thread::sleep(1000); });
			token t = s.connect([&]{ thread::sleep(1000); handler_invoked.set(true); });
			thread::sleep(300);
			profiler p;
			t.reset();
			auto disconnect_time = duration_cast<milliseconds>(p.reset()).count();
			TS_ASSERT(!handler_invoked.get());
			TS_ASSERT_LESS_THAN_EQUALS(disconnect_time, 100);
		}

		{
			mutexed<bool> handler_invoked(false);
			std::shared_ptr<task_executor> worker = std::make_shared<thread_task_executor>();
			token t = s.connect([&]{ thread::sleep(1000); handler_invoked.set(true); });
			token other_t = s.connect([]{ thread::sleep(1000); });
			thread::sleep(300);
			profiler p;
			t.reset();
			auto disconnect_time = duration_cast<milliseconds>(p.reset()).count();
			TS_ASSERT(handler_invoked.get());
			TS_ASSERT_LESS_THAN_EQUALS(600, disconnect_time);
			TS_ASSERT_LESS_THAN_EQUALS(disconnect_time, 1200);
		}

		{
			mutexed<bool> handler_invoked(false);
			std::shared_ptr<task_executor> worker = std::make_shared<thread_task_executor>();
			token other_t = s.connect([]{ thread::sleep(1000); });
			token t = s.connect([&]{ thread::sleep(1000); handler_invoked.set(true); });
			thread::sleep(300);
			profiler p;
			t.reset();
			auto disconnect_time = duration_cast<milliseconds>(p.reset()).count();
			TS_ASSERT(!handler_invoked.get());
			TS_ASSERT_LESS_THAN_EQUALS(disconnect_time, 100);
		}
	}

	static void test__life_assurance__single_threaded()
	{
		std::shared_ptr<threadless_task_executor> worker = std::make_shared<threadless_task_executor>();
		signal<void(), exception_handling::default_, threading::default_, state_populating::default_, life_assurance::single_threaded> s;

		{
			mutexed<bool> handler_invoked(false);
			token t = s.connect(worker, [&]{ handler_invoked.set(true); });
			s();
			worker->process_tasks();
			t.reset();
			TS_ASSERT(handler_invoked.get());
		}

		{
			mutexed<bool> handler_invoked(false);
			token t = s.connect(worker, [&]{ handler_invoked.set(true); });
			s();
			t.reset();
			worker->process_tasks();
			TS_ASSERT(!handler_invoked.get());
		}
	}

	static void test__life_assurance__none()
	{
		signal<void(), exception_handling::default_, threading::default_, state_populating::default_, life_assurance::none> s;

		bool handler_invoked = false;
		token t = s.connect([&]{ handler_invoked  = true; });
		s();
		t.reset();
		TS_ASSERT(handler_invoked);
	}

	////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

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

		thread::sleep(500);

		std::lock_guard<std::mutex> l(m);
		TS_ASSERT_DIFFERS(handler_thread_id, std::thread::id());
		TS_ASSERT_DIFFERS(handler_thread_id, std::this_thread::get_id());
	}
};

#endif
