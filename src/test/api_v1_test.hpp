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


#include <wigwag/listenable.hpp>
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
private:
	class test_listener
	{
	private:
		std::function<void()>		_f_impl;
		std::function<void(int)>	_g_impl;

	public:
		test_listener(const std::function<void()>& f_impl, const std::function<void(int)>& g_impl)
			: _f_impl(f_impl), _g_impl(g_impl)
		{ }

		virtual void f() const { _f_impl(); }
		virtual void g(int i) const { _g_impl(i); }
	};

public:
	static void test_signals()
	{
		signal<void(int)> s;
		std::shared_ptr<task_executor> worker = std::make_shared<thread_task_executor>();

		std::mutex m;
		int value = 0;

		token t0 = s.connect([&](int i) { auto l = lock(m); value += i; });
		s(1);
		{ auto l = lock(m); TS_ASSERT_EQUALS(value, 1); }

		token t1 = s.connect([&](int i) { auto l = lock(m); value += 10 * i; });
		s(3);
		{ auto l = lock(m); TS_ASSERT_EQUALS(value, 34); }

		t0.reset();
		s(5);
		{ auto l = lock(m); TS_ASSERT_EQUALS(value, 84); }

		token t2 = s.connect(worker, [&](int i) { auto l = lock(m); value -= i; });
		s(7);
		thread::sleep(100);
		{ auto l = lock(m); TS_ASSERT_EQUALS(value, 147); }
	}

	static void test_listenable()
	{
		listenable<test_listener> l;

		int f_value = 0, g_value = 0;

		token t0 = l.connect(test_listener([&] { ++f_value; }, [&](int i) { g_value += i; }));
		l.invoke(std::bind(&test_listener::f, std::placeholders::_1));
		TS_ASSERT_EQUALS(f_value, 1);
		TS_ASSERT_EQUALS(g_value, 0);
		l.invoke(std::bind(&test_listener::g, std::placeholders::_1, 32));
		TS_ASSERT_EQUALS(f_value, 1);
		TS_ASSERT_EQUALS(g_value, 32);

		token t1 = l.connect(test_listener([&] { f_value += 10; }, [&](int i) { g_value += i * 2; }));
		l.invoke([](const test_listener& f) { f.f(); });
		TS_ASSERT_EQUALS(f_value, 12);
		TS_ASSERT_EQUALS(g_value, 32);
		l.invoke([](const test_listener& f) { f.g(15); });
		TS_ASSERT_EQUALS(f_value, 12);
		TS_ASSERT_EQUALS(g_value, 77);

		t0.reset();
		l.invoke([](const test_listener& f) { f.f(); });
		TS_ASSERT_EQUALS(f_value, 22);
		TS_ASSERT_EQUALS(g_value, 77);
		l.invoke([](const test_listener& f) { f.g(5); });
		TS_ASSERT_EQUALS(f_value, 22);
		TS_ASSERT_EQUALS(g_value, 87);
	}

	////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	static void test_token()
	{
		signal<void()> s;
		token t;
		int counter = 0;
		t = s.connect([&] { ++counter; });
		s();
		TS_ASSERT_EQUALS(counter, 1);
		t = s.connect([&] { counter += 100; });
		s();
		TS_ASSERT_EQUALS(counter, 101);
	}

	////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	static void test_connect_from_handler()
	{
		{
			token_pool tp;
			signal<void()> s;
			bool second_handler_invoked = false;

			tp += s.connect([&]{ tp += s.connect([&] { second_handler_invoked = true; }); });
			s();
			TS_ASSERT(!second_handler_invoked);
			s();
			TS_ASSERT(second_handler_invoked);
		}
		{
			token_pool tp;
			listenable<test_listener> l;
			bool second_handler_invoked = false;

			tp += l.connect(test_listener([&] { tp += l.connect(test_listener([&] { second_handler_invoked = true; }, [](int){})); }, [](int){}));
			l.invoke([](const test_listener& f) { f.f(); });
			TS_ASSERT(!second_handler_invoked);
			l.invoke([](const test_listener& f) { f.f(); });
			TS_ASSERT(second_handler_invoked);
		}
	}

	static void test_disconnect_from_handler()
	{
		{
			signal<void()> s;
			bool second_handler_invoked = false;

			std::unique_ptr<token> t2;
			token t1 = s.connect([&] { t2.reset(); });
			t2.reset(new token(s.connect([&] { second_handler_invoked = true; })));

			s();
			TS_ASSERT(!second_handler_invoked);
		}
		{
			listenable<test_listener> l;
			bool second_handler_invoked = false;

			std::unique_ptr<token> t2;
			token t1 = l.connect(test_listener([&] { t2.reset(); }, [](int){}));
			t2.reset(new token(l.connect(test_listener([&] { second_handler_invoked = true; }, [](int){}))));

			l.invoke([](const test_listener& f) { f.f(); });
			TS_ASSERT(!second_handler_invoked);
		}
	}

	////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	static void test_signal_attributes()
	{
		std::shared_ptr<task_executor> worker = std::make_shared<thread_task_executor>();
		token_pool tp;

		{
			signal<void()> s;
			TS_ASSERT_THROWS_NOTHING(tp += s.connect([]{}));
			TS_ASSERT_THROWS_NOTHING(tp += s.connect(worker, []{}));
		}

		{
			signal<void()> s(signal_attributes::none);
			TS_ASSERT_THROWS_NOTHING(tp += s.connect([]{}));
			TS_ASSERT_THROWS_NOTHING(tp += s.connect(worker, []{}));
		}

		{
			signal<void()> s(signal_attributes::connect_sync_only);
			TS_ASSERT_THROWS_NOTHING(tp += s.connect([]{}));
			TS_ASSERT_THROWS((tp += s.connect(worker, []{})), std::runtime_error);
		}

		{
			signal<void()> s(signal_attributes::connect_async_only);
			TS_ASSERT_THROWS((tp += s.connect([]{})), std::runtime_error);
			TS_ASSERT_THROWS_NOTHING(tp += s.connect(worker, []{}));
		}
	}

	////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	static void test_handler_attributes()
	{
		using h_type = const std::function<void(int)>&;

		std::shared_ptr<task_executor> worker = std::make_shared<thread_task_executor>();
		signal<void(int), exception_handling::default_, threading::default_, state_populating::populator_and_withdrawer> s(std::make_pair([](h_type h){ h(1); }, [](h_type h){ h(3); }));

		{
			int sync_state = 0;
			token t = s.connect([&](int i) { sync_state = i; }, handler_attributes::none);
			TS_ASSERT_EQUALS(sync_state, 1);
			s(2);
			TS_ASSERT_EQUALS(sync_state, 2);
			t.reset();
			TS_ASSERT_EQUALS(sync_state, 3);
		}

		{
			mutexed<int> async_state(0);
			token t = s.connect(worker, [&](int i) { async_state.set(i); }, handler_attributes::none);
			thread::sleep(100);
			TS_ASSERT_EQUALS(async_state.get(), 1);
			s(2);
			thread::sleep(100);
			TS_ASSERT_EQUALS(async_state.get(), 2);
		}

		{
			int sync_state = 0;
			token t = s.connect([&](int i) { sync_state = i; }, handler_attributes::suppress_populator);
			TS_ASSERT_EQUALS(sync_state, 0);
			s(2);
			TS_ASSERT_EQUALS(sync_state, 2);
			t.reset();
			TS_ASSERT_EQUALS(sync_state, 2);
		}
	}

	////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

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

		listenable<test_listener> l;

		{
			token t = l.connect(test_listener([&] { throw std::runtime_error("Test exception"); }, [](int){}));
			TS_ASSERT_THROWS(l.invoke([](const test_listener& l){ l.f(); }), std::runtime_error);
		}

		{
			token t = l.connect(test_listener([]{}, [&] (int i) { throw i; }));
			TS_ASSERT_THROWS_ANYTHING(l.invoke([](const test_listener& l){ l.g(42); }));
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

		listenable<test_listener, exception_handling::none> l;

		{
			token t = l.connect(test_listener([&] { throw std::runtime_error("Test exception"); }, [](int){}));
			TS_ASSERT_THROWS(l.invoke([](const test_listener& l){ l.f(); }), std::runtime_error);
		}

		{
			token t = l.connect(test_listener([]{}, [&] (int i) { throw i; }));
			TS_ASSERT_THROWS_ANYTHING(l.invoke([](const test_listener& l){ l.g(42); }));
		}
	}


	static void test__exception_handling__print_to_stderr()
	{
		{
			signal<void(), exception_handling::print_to_stderr> s;

			token_pool tp;
			tp += s.connect([&] { throw std::runtime_error("Test exception"); });
			TS_ASSERT_THROWS_NOTHING(s());
			tp += s.connect([&] { throw 0; });
			TS_ASSERT_THROWS_ANYTHING(s());

			std::shared_ptr<task_executor> worker = std::make_shared<basic_thread_task_executor<exception_handling::print_to_stderr> >();
			worker->add_task([]{ throw std::runtime_error("Test exception"); });
			thread::sleep(300);
		}

		{
			listenable<test_listener, exception_handling::print_to_stderr> l;

			token_pool tp;
			tp += l.connect(test_listener([&] { throw std::runtime_error("Test exception"); }, [](int){}));
			tp += l.connect(test_listener([]{}, [&] (int i) { throw i; }));
			TS_ASSERT_THROWS_NOTHING(l.invoke([](const test_listener& l){ l.f(); }));
			TS_ASSERT_THROWS_ANYTHING(l.invoke([](const test_listener& l){ l.g(42); }));
		}
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
		{
			std::shared_ptr<task_executor> worker = std::make_shared<thread_task_executor>();
			worker->add_task([]{ thread::sleep(200); });
			token_pool tp;
			Signal_ s;
			tp += s.connect(worker, []{ thread::sleep(200); });
			s();
		}

		{
			std::shared_ptr<task_executor> worker = std::make_shared<thread_task_executor>();
			worker->add_task([]{ thread::sleep(200); });
			Signal_ s;
			token_pool tp;
			tp += s.connect(worker, []{ thread::sleep(200); });
			s();
		}

		{
			std::shared_ptr<task_executor> worker = std::make_shared<thread_task_executor>();
			token_pool tp;
			Signal_ s;
			tp += s.connect(worker, []{ });
			s();
			thread::sleep(200);
		}

		{
			std::shared_ptr<task_executor> worker = std::make_shared<thread_task_executor>();
			Signal_ s;
			token_pool tp;
			tp += s.connect(worker, []{ });
			s();
			thread::sleep(200);
		}

		{
			Signal_ s;
			thread th([&](const std::atomic<bool>& alive) { while (alive) { s(); thread::sleep(100); } });
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
			Signal_ s;
			thread th([&](const std::atomic<bool>& alive) { while (alive) { s(); thread::sleep(100); } });
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
			Signal_ s;
			thread th([&](const std::atomic<bool>& alive) { while (alive) { s(); thread::sleep(100); } });
			mutexed<bool> handler_invoked(false);
			std::shared_ptr<task_executor> worker = std::make_shared<thread_task_executor>();
			token t = s.connect(worker, [&]{ thread::sleep(1000); handler_invoked.set(true); });
			token other_t = s.connect(worker, []{ thread::sleep(1000); });
			thread::sleep(300);
			profiler p;
			t.reset();
			auto disconnect_time = duration_cast<milliseconds>(p.reset()).count();
			TS_ASSERT(handler_invoked.get());
			TS_ASSERT_LESS_THAN_EQUALS(600, disconnect_time);
			TS_ASSERT_LESS_THAN_EQUALS(disconnect_time, 1200);
		}

		{
			Signal_ s;
			thread th([&](const std::atomic<bool>& alive) { while (alive) { s(); thread::sleep(100); } });
			mutexed<bool> handler_invoked(false);
			std::shared_ptr<task_executor> worker = std::make_shared<thread_task_executor>();
			token other_t = s.connect(worker, []{ thread::sleep(1000); });
			token t = s.connect(worker, [&]{ thread::sleep(1000); handler_invoked.set(true); });
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
		{
			std::shared_ptr<threadless_task_executor> worker = std::make_shared<threadless_task_executor>();
			token_pool tp;
			signal<void(), exception_handling::default_, threading::default_, state_populating::default_, life_assurance::single_threaded> s;
			tp += s.connect(worker, []{ });
			s();
		}

		{
			std::shared_ptr<threadless_task_executor> worker = std::make_shared<threadless_task_executor>();
			signal<void(), exception_handling::default_, threading::default_, state_populating::default_, life_assurance::single_threaded> s;
			token_pool tp;
			tp += s.connect(worker, []{ });
			s();
		}

		{
			std::shared_ptr<threadless_task_executor> worker = std::make_shared<threadless_task_executor>();
			token_pool tp;
			signal<void(), exception_handling::default_, threading::default_, state_populating::default_, life_assurance::single_threaded> s;
			tp += s.connect(worker, []{ });
			s();
			worker->process_tasks();
		}

		{
			std::shared_ptr<threadless_task_executor> worker = std::make_shared<threadless_task_executor>();
			signal<void(), exception_handling::default_, threading::default_, state_populating::default_, life_assurance::single_threaded> s;
			token_pool tp;
			tp += s.connect(worker, []{ });
			s();
			worker->process_tasks();
		}

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

	static void test_life_token()
	{
		{
			life_token lt;

			thread th(
				[&](const std::atomic<bool>& alive)
				{
					life_token::execution_guard g(lt);
					if (g.is_alive())
						thread::sleep(300);
				});

			thread::sleep(100);

			profiler p;
			lt.release();
			auto release_time = duration_cast<milliseconds>(p.reset()).count();
			TS_ASSERT_LESS_THAN_EQUALS(150, release_time);
		}

		{
			life_token lt;

			thread th(
				[&](const std::atomic<bool>& alive)
				{
					thread::sleep(100);

					life_token::execution_guard g(lt);
					if (g.is_alive())
						thread::sleep(300);
				});

			profiler p;
			lt.release();
			auto release_time = duration_cast<milliseconds>(p.reset()).count();
			TS_ASSERT_LESS_THAN_EQUALS(release_time, 100);
		}

		{
			life_token lt;
			life_token::checker lc(lt);

			thread th(
				[&](const std::atomic<bool>& alive)
				{
					life_token::execution_guard g(lc);
					if (g.is_alive())
						thread::sleep(300);
				});

			thread::sleep(100);

			profiler p;
			lt.release();
			auto release_time = duration_cast<milliseconds>(p.reset()).count();
			TS_ASSERT_LESS_THAN_EQUALS(150, release_time);
		}

		{
			life_token lt;
			life_token::checker lc(lt);

			thread th(
				[&](const std::atomic<bool>& alive)
				{
					thread::sleep(100);

					life_token::execution_guard g(lc);
					if (g.is_alive())
						thread::sleep(300);
				});

			profiler p;
			lt.release();
			auto release_time = duration_cast<milliseconds>(p.reset()).count();
			TS_ASSERT_LESS_THAN_EQUALS(release_time, 100);
		}
	}

	////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	static void test_task_executors()
	{
		{
			std::shared_ptr<task_executor> worker = std::make_shared<thread_task_executor>();

			signal<void()> s;

			std::mutex m;
			std::thread::id handler_thread_id;

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

		{
			std::shared_ptr<task_executor> worker = std::make_shared<thread_task_executor>();

			std::mutex m;
			int n = 0;

			for (int i = 0; i < 3; ++i)
				worker->add_task([&] {
						thread::sleep(200);
						auto l = lock(m);
						++n;
					});

			profiler p;
			worker.reset();
			auto worker_dtor_time = duration_cast<milliseconds>(p.reset()).count();
			TS_ASSERT_LESS_THAN_EQUALS(500, worker_dtor_time);
			auto l = lock(m);
			TS_ASSERT_EQUALS(n, 3);
		}
	}
};

#endif
