// Copyright (c) 2016, Dmitry Koplyarov <koplyarov.da@gmail.com>
//
// Permission to use, copy, modify, and/or distribute this software for any purpose with or without fee is hereby granted,
// provided that the above copyright notice and this permission notice appear in all copies.
//
// THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS.
// IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS,
// WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.


#include <wigwag/life_token.hpp>
#include <wigwag/listenable.hpp>
#include <wigwag/signal.hpp>
#include <wigwag/thread_task_executor.hpp>
#include <wigwag/threadless_task_executor.hpp>
#include <wigwag/token_pool.hpp>

#include <gtest/gtest.h>

#include <chrono>
#include <iostream>
#include <thread>

#include <test/utils/mutexed.hpp>
#include <test/utils/profiler.hpp>
#include <test/utils/thread.hpp>


#if  defined(_MSC_VER) && _MSC_VER < 1900
#   define HAS_STD_FUNCTION_MOVE_BUG 1
#else
#   define HAS_STD_FUNCTION_MOVE_BUG 0
#endif


using namespace std::chrono;


class test_listener
{
private:
    std::function<void()>       _f_impl;
    std::function<void(int)>    _g_impl;

public:
    test_listener(const std::function<void()>& f_impl, const std::function<void(int)>& g_impl)
        : _f_impl(f_impl), _g_impl(g_impl)
    { }

    virtual void f() const { _f_impl(); }
    virtual void g(int i) { _g_impl(i); }
};

class copy_ctor_counter
{
private:
    std::atomic<int>&       _counter;
    bool                    _movedFrom;

public:
    copy_ctor_counter(std::atomic<int>& counter)
        : _counter(counter), _movedFrom(false)
    { }

    copy_ctor_counter(const copy_ctor_counter& other)
        : _counter(other._counter), _movedFrom(false)
    { ++_counter; }

    copy_ctor_counter(copy_ctor_counter&& other)
        : _counter(other._counter), _movedFrom(false)
    { other._movedFrom = true; }

    copy_ctor_counter& operator = (const copy_ctor_counter&) = delete;

    void operator() () const
    {
        if (_movedFrom)
            throw std::runtime_error("Accessing invalid copy_ctor_counter!");
    }
};

template < typename Signal_ >
static void do_test_life_assurance_common()
{
    {
        std::shared_ptr<wigwag::task_executor> worker = std::make_shared<wigwag::thread_task_executor>();
        worker->add_task([]{ wigwag::thread::sleep(200); });
        wigwag::token_pool tp;
        Signal_ s;
        tp += s.connect(worker, []{ wigwag::thread::sleep(200); });
        s();
    }

    {
        std::shared_ptr<wigwag::task_executor> worker = std::make_shared<wigwag::thread_task_executor>();
        worker->add_task([]{ wigwag::thread::sleep(200); });
        Signal_ s;
        wigwag::token_pool tp;
        tp += s.connect(worker, []{ wigwag::thread::sleep(200); });
        s();
    }

    {
        std::shared_ptr<wigwag::task_executor> worker = std::make_shared<wigwag::thread_task_executor>();
        wigwag::token_pool tp;
        Signal_ s;
        tp += s.connect(worker, []{ });
        s();
        wigwag::thread::sleep(200);
    }

    {
        std::shared_ptr<wigwag::task_executor> worker = std::make_shared<wigwag::thread_task_executor>();
        Signal_ s;
        wigwag::token_pool tp;
        tp += s.connect(worker, []{ });
        s();
        wigwag::thread::sleep(200);
    }

    {
        Signal_ s;
        wigwag::thread th([&](const std::atomic<bool>& alive) { while (alive) { s(); wigwag::thread::sleep(100); } });
        wigwag::mutexed<bool> handler_invoked(false);
        wigwag::token t = s.connect([&]{ wigwag::thread::sleep(1000); handler_invoked.set(true); });
        wigwag::token other_t = s.connect([]{ wigwag::thread::sleep(1000); });
        wigwag::thread::sleep(300);
        wigwag::profiler p;
        t.reset();
        auto disconnect_time = duration_cast<milliseconds>(p.reset()).count();
        ASSERT_TRUE(handler_invoked.get());
        ASSERT_LE(600, disconnect_time);
        ASSERT_LE(disconnect_time, 1200);
    }

    {
        Signal_ s;
        wigwag::thread th([&](const std::atomic<bool>& alive) { while (alive) { s(); wigwag::thread::sleep(100); } });
        wigwag::mutexed<bool> handler_invoked(false);
        wigwag::token other_t = s.connect([]{ wigwag::thread::sleep(1000); });
        wigwag::token t = s.connect([&]{ wigwag::thread::sleep(1000); handler_invoked.set(true); });
        wigwag::thread::sleep(300);
        wigwag::profiler p;
        t.reset();
        auto disconnect_time = duration_cast<milliseconds>(p.reset()).count();
        ASSERT_FALSE(handler_invoked.get());
        ASSERT_LE(disconnect_time, 100);
    }

    {
        Signal_ s;
        wigwag::thread th([&](const std::atomic<bool>& alive) { while (alive) { s(); wigwag::thread::sleep(100); } });
        wigwag::mutexed<bool> handler_invoked(false);
        std::shared_ptr<wigwag::task_executor> worker = std::make_shared<wigwag::thread_task_executor>();
        wigwag::token t = s.connect(worker, [&]{ wigwag::thread::sleep(1000); handler_invoked.set(true); });
        wigwag::token other_t = s.connect(worker, []{ wigwag::thread::sleep(1000); });
        wigwag::thread::sleep(300);
        wigwag::profiler p;
        t.reset();
        auto disconnect_time = duration_cast<milliseconds>(p.reset()).count();
        ASSERT_TRUE(handler_invoked.get());
        ASSERT_LE(600, disconnect_time);
        ASSERT_LE(disconnect_time, 1200);
    }

    {
        Signal_ s;
        wigwag::thread th([&](const std::atomic<bool>& alive) { while (alive) { s(); wigwag::thread::sleep(100); } });
        wigwag::mutexed<bool> handler_invoked(false);
        std::shared_ptr<wigwag::task_executor> worker = std::make_shared<wigwag::thread_task_executor>();
        wigwag::token other_t = s.connect(worker, []{ wigwag::thread::sleep(1000); });
        wigwag::token t = s.connect(worker, [&]{ wigwag::thread::sleep(1000); handler_invoked.set(true); });
        wigwag::thread::sleep(300);
        wigwag::profiler p;
        t.reset();
        auto disconnect_time = duration_cast<milliseconds>(p.reset()).count();
        ASSERT_FALSE(handler_invoked.get());
        ASSERT_LE(disconnect_time, 100);
    }
}

// ============================================================================
// Basic signal/listenable tests
// ============================================================================

TEST(WigwagApiV1, Signals)
{
    wigwag::signal<void(int)> s;
    std::shared_ptr<wigwag::task_executor> worker = std::make_shared<wigwag::thread_task_executor>();

    std::mutex m;
    int value = 0;

    wigwag::token t0 = s.connect([&](int i) { auto l = wigwag::lock(m); value += i; });
    s(1);
    { auto l = wigwag::lock(m); ASSERT_EQ(value, 1); }

    wigwag::token t1 = s.connect([&](int i) { auto l = wigwag::lock(m); value += 10 * i; });
    s(3);
    { auto l = wigwag::lock(m); ASSERT_EQ(value, 34); }

    t0.reset();
    s(5);
    { auto l = wigwag::lock(m); ASSERT_EQ(value, 84); }

    wigwag::token t2 = s.connect(worker, [&](int i) { auto l = wigwag::lock(m); value -= i; });
    s(7);
    wigwag::thread::sleep(100);
    { auto l = wigwag::lock(m); ASSERT_EQ(value, 147); }
}

TEST(WigwagApiV1, SignalConnector)
{
    wigwag::signal<void(int)> s;
    wigwag::signal_connector<void(int)> c = s.connector();
    std::shared_ptr<wigwag::task_executor> worker = std::make_shared<wigwag::thread_task_executor>();

    std::mutex m;
    int value = 0;

    wigwag::token t0 = c.connect([&](int i) { auto l = wigwag::lock(m); value += i; });
    s(1);
    { auto l = wigwag::lock(m); ASSERT_EQ(value, 1); }

    wigwag::token t1 = c.connect([&](int i) { auto l = wigwag::lock(m); value += 10 * i; });
    s(3);
    { auto l = wigwag::lock(m); ASSERT_EQ(value, 34); }

    t0.reset();
    s(5);
    { auto l = wigwag::lock(m); ASSERT_EQ(value, 84); }

    wigwag::token t2 = c.connect(worker, [&](int i) { auto l = wigwag::lock(m); value -= i; });
    s(7);
    wigwag::thread::sleep(100);
    { auto l = wigwag::lock(m); ASSERT_EQ(value, 147); }
}

TEST(WigwagApiV1, Listenable)
{
    wigwag::listenable<test_listener> l;

    int f_value = 0, g_value = 0;

    wigwag::token t0 = l.connect(test_listener([&] { ++f_value; }, [&](int i) { g_value += i; }));
    l.invoke(std::bind(&test_listener::f, std::placeholders::_1));
    ASSERT_EQ(f_value, 1);
    ASSERT_EQ(g_value, 0);
    l.invoke(std::bind(&test_listener::g, std::placeholders::_1, 32));
    ASSERT_EQ(f_value, 1);
    ASSERT_EQ(g_value, 32);

    wigwag::token t1 = l.connect(test_listener([&] { f_value += 10; }, [&](int i) { g_value += i * 2; }));
    l.invoke([](const test_listener& f) { f.f(); });
    ASSERT_EQ(f_value, 12);
    ASSERT_EQ(g_value, 32);
    l.invoke([](test_listener& f) { f.g(15); });
    ASSERT_EQ(f_value, 12);
    ASSERT_EQ(g_value, 77);

    t0.reset();
    l.invoke([](const test_listener& f) { f.f(); });
    ASSERT_EQ(f_value, 22);
    ASSERT_EQ(g_value, 77);
    l.invoke([](test_listener& f) { f.g(5); });
    ASSERT_EQ(f_value, 22);
    ASSERT_EQ(g_value, 87);
}

// ============================================================================
// Token tests
// ============================================================================

TEST(WigwagApiV1, Token)
{
    wigwag::signal<void()> s;
    wigwag::token t;
    int counter = 0;
    t = s.connect([&] { ++counter; });
    s();
    ASSERT_EQ(counter, 1);
    t = s.connect([&] { counter += 100; });
    s();
    ASSERT_EQ(counter, 101);
}

TEST(WigwagApiV1, TokenPool)
{
    wigwag::signal<void()> s;
    wigwag::token_pool tp;
    int counter = 0;
    tp += s.connect([&] { ++counter; });
    s();
    ASSERT_EQ(counter, 1);
    tp.add_token(s.connect([&] { counter += 100; }));
    s();
    ASSERT_EQ(counter, 102);
    tp.release();
    s();
    ASSERT_EQ(counter, 102);
}

// ============================================================================
// Connect/disconnect from handler tests
// ============================================================================

TEST(WigwagApiV1, ConnectFromHandler)
{
    {
        wigwag::token_pool tp;
        wigwag::signal<void()> s;
        bool second_handler_invoked = false;

        tp += s.connect([&]{ tp += s.connect([&] { second_handler_invoked = true; }); });
        s();
        ASSERT_FALSE(second_handler_invoked);
        s();
        ASSERT_TRUE(second_handler_invoked);
    }
    {
        wigwag::token_pool tp;
        wigwag::listenable<test_listener> l;
        bool second_handler_invoked = false;

        tp += l.connect(test_listener([&] { tp += l.connect(test_listener([&] { second_handler_invoked = true; }, [](int){})); }, [](int){}));
        l.invoke([](const test_listener& f) { f.f(); });
        ASSERT_FALSE(second_handler_invoked);
        l.invoke([](const test_listener& f) { f.f(); });
        ASSERT_TRUE(second_handler_invoked);
    }
}

TEST(WigwagApiV1, DisconnectFromHandler)
{
    {
        wigwag::signal<void()> s;
        bool second_handler_invoked = false;

        std::unique_ptr<wigwag::token> t2;
        wigwag::token t1 = s.connect([&] { t2.reset(); });
        t2.reset(new wigwag::token(s.connect([&] { second_handler_invoked = true; })));

        s();
        ASSERT_FALSE(second_handler_invoked);
    }
    {
        wigwag::listenable<test_listener> l;
        bool second_handler_invoked = false;

        std::unique_ptr<wigwag::token> t2;
        wigwag::token t1 = l.connect(test_listener([&] { t2.reset(); }, [](int){}));
        t2.reset(new wigwag::token(l.connect(test_listener([&] { second_handler_invoked = true; }, [](int){}))));

        l.invoke([](const test_listener& f) { f.f(); });
        ASSERT_FALSE(second_handler_invoked);
    }
}

// ============================================================================
// Signal/handler attributes tests
// ============================================================================

TEST(WigwagApiV1, SignalAttributes)
{
    std::shared_ptr<wigwag::task_executor> worker = std::make_shared<wigwag::thread_task_executor>();
    wigwag::token_pool tp;

    {
        wigwag::signal<void()> s;
        ASSERT_NO_THROW(tp += s.connect([]{}));
        ASSERT_NO_THROW(tp += s.connect(worker, []{}));
    }

    {
        wigwag::signal<void()> s(wigwag::signal_attributes::none);
        ASSERT_NO_THROW(tp += s.connect([]{}));
        ASSERT_NO_THROW(tp += s.connect(worker, []{}));
    }

    {
        wigwag::signal<void()> s(wigwag::signal_attributes::connect_sync_only);
        ASSERT_NO_THROW(tp += s.connect([]{}));
        ASSERT_THROW((tp += s.connect(worker, []{})), std::runtime_error);
    }

    {
        wigwag::signal<void()> s(wigwag::signal_attributes::connect_async_only);
        ASSERT_THROW((tp += s.connect([]{})), std::runtime_error);
        ASSERT_NO_THROW(tp += s.connect(worker, []{}));
    }
}

TEST(WigwagApiV1, HandlerAttributes)
{
    using h_type = const std::function<void(int)>&;

    std::shared_ptr<wigwag::task_executor> worker = std::make_shared<wigwag::thread_task_executor>();
    wigwag::signal<void(int), wigwag::exception_handling::default_, wigwag::threading::default_, wigwag::state_populating::populator_and_withdrawer> s(std::make_pair([](h_type h){ h(1); }, [](h_type h){ h(3); }));

    {
        int sync_state = 0;
        wigwag::token t = s.connect([&](int i) { sync_state = i; }, wigwag::handler_attributes::none);
        ASSERT_EQ(sync_state, 1);
        s(2);
        ASSERT_EQ(sync_state, 2);
        t.reset();
        ASSERT_EQ(sync_state, 3);
    }

    {
        wigwag::mutexed<int> async_state(0);
        wigwag::token t = s.connect(worker, [&](int i) { async_state.set(i); }, wigwag::handler_attributes::none);
        wigwag::thread::sleep(100);
        ASSERT_EQ(async_state.get(), 1);
        s(2);
        wigwag::thread::sleep(100);
        ASSERT_EQ(async_state.get(), 2);
    }

    {
        int sync_state = 0;
        wigwag::token t = s.connect([&](int i) { sync_state = i; }, wigwag::handler_attributes::suppress_populator);
        ASSERT_EQ(sync_state, 0);
        s(2);
        ASSERT_EQ(sync_state, 2);
        t.reset();
        ASSERT_EQ(sync_state, 2);
    }
}

// ============================================================================
// Exception handling policy tests
// ============================================================================

TEST(WigwagApiV1, ExceptionHandlingDefault)
{
    wigwag::signal<void()> s;

    {
        wigwag::token t = s.connect([&] { throw std::runtime_error("Test exception"); });
        ASSERT_THROW(s(), std::runtime_error);
    }

    {
        wigwag::token t = s.connect([&] { throw 0; });
        ASSERT_ANY_THROW(s());
    }

    wigwag::listenable<test_listener> l;

    {
        wigwag::token t = l.connect(test_listener([&] { throw std::runtime_error("Test exception"); }, [](int){}));
        ASSERT_THROW(l.invoke([](const test_listener& l){ l.f(); }), std::runtime_error);
    }

    {
        wigwag::token t = l.connect(test_listener([]{}, [&] (int i) { throw i; }));
        ASSERT_ANY_THROW(l.invoke([](test_listener& l){ l.g(42); }));
    }
}

TEST(WigwagApiV1, ExceptionHandlingNone)
{
    wigwag::signal<void(), wigwag::exception_handling::none> s;

    {
        wigwag::token t = s.connect([&] { throw std::runtime_error("Test exception"); });
        ASSERT_THROW(s(), std::runtime_error);
    }

    {
        wigwag::token t = s.connect([&] { throw 0; });
        ASSERT_ANY_THROW(s());
    }

    wigwag::listenable<test_listener, wigwag::exception_handling::none> l;

    {
        wigwag::token t = l.connect(test_listener([&] { throw std::runtime_error("Test exception"); }, [](int){}));
        ASSERT_THROW(l.invoke([](const test_listener& l){ l.f(); }), std::runtime_error);
    }

    {
        wigwag::token t = l.connect(test_listener([]{}, [&] (int i) { throw i; }));
        ASSERT_ANY_THROW(l.invoke([](test_listener& l){ l.g(42); }));
    }
}

TEST(WigwagApiV1, ExceptionHandlingPrintToStderr)
{
    {
        wigwag::signal<void(), wigwag::exception_handling::print_to_stderr> s;

        wigwag::token_pool tp;
        tp += s.connect([&] { throw std::runtime_error("Test exception"); });
        ASSERT_NO_THROW(s());
        tp += s.connect([&] { throw 0; });
        ASSERT_ANY_THROW(s());

        std::shared_ptr<wigwag::basic_thread_task_executor<wigwag::exception_handling::print_to_stderr> > worker = std::make_shared<wigwag::basic_thread_task_executor<wigwag::exception_handling::print_to_stderr> >();
        worker->add_task([]{ throw std::runtime_error("Test exception"); });
        wigwag::thread::sleep(300);
    }

    {
        wigwag::listenable<test_listener, wigwag::exception_handling::print_to_stderr> l;

        wigwag::token_pool tp;
        tp += l.connect(test_listener([&] { throw std::runtime_error("Test exception"); }, [](int){}));
        tp += l.connect(test_listener([]{}, [&] (int i) { throw i; }));
        ASSERT_NO_THROW(l.invoke([](const test_listener& l){ l.f(); }));
        ASSERT_ANY_THROW(l.invoke([](test_listener& l){ l.g(42); }));
    }
}

// ============================================================================
// State populating policy tests
// ============================================================================

TEST(WigwagApiV1, StatePopulatingDefault)
{
    using h_type = const std::function<void(int)>&;

    int signal_state = 1;
    wigwag::signal<void(int)> s([&](h_type h){ h(signal_state); } );
    wigwag::thread th(
        [&](const std::atomic<bool>& alive)
        {
            wigwag::thread::sleep(500);
            auto l = wigwag::lock(s.lock_primitive());
            signal_state = 2;
            s(2);
        }
    );

    wigwag::mutexed<int> state;
    wigwag::token t = s.connect([&](int i) { state.set(i); });
    ASSERT_EQ(state.get(), 1);
    wigwag::thread::sleep(1000);
    ASSERT_EQ(state.get(), 2);
    t.reset();
    ASSERT_EQ(state.get(), 2);
}

TEST(WigwagApiV1, StatePopulatingPopulatorOnly)
{
    using h_type = const std::function<void(int)>&;

    int signal_state = 1;
    wigwag::signal<void(int), wigwag::exception_handling::default_, wigwag::threading::default_, wigwag::state_populating::populator_only> s([&](h_type h){ h(signal_state); } );
    wigwag::thread th(
        [&](const std::atomic<bool>& alive)
        {
            wigwag::thread::sleep(500);
            auto l = wigwag::lock(s.lock_primitive());
            signal_state = 2;
            s(2);
        }
    );

    wigwag::mutexed<int> state;
    wigwag::token t = s.connect([&](int i) { state.set(i); });
    ASSERT_EQ(state.get(), 1);
    wigwag::thread::sleep(1000);
    ASSERT_EQ(state.get(), 2);
    t.reset();
    ASSERT_EQ(state.get(), 2);
}

TEST(WigwagApiV1, StatePopulatingPopulatorAndWithdrawer)
{
    using h_type = const std::function<void(int)>&;

    int signal_state = 1;
    wigwag::signal<void(int), wigwag::exception_handling::default_, wigwag::threading::default_, wigwag::state_populating::populator_and_withdrawer> s(std::make_pair([](h_type h){ h(1); }, [](h_type h){ h(3); }));

    wigwag::thread th(
        [&](const std::atomic<bool>& alive)
        {
            wigwag::thread::sleep(500);
            auto l = wigwag::lock(s.lock_primitive());
            signal_state = 2;
            s(2);
        }
    );

    wigwag::mutexed<int> state;
    wigwag::token t = s.connect([&](int i) { state.set(i); });
    ASSERT_EQ(state.get(), 1);
    wigwag::thread::sleep(1000);
    ASSERT_EQ(state.get(), 2);
    t.reset();
    ASSERT_EQ(state.get(), 3);
}

TEST(WigwagApiV1, StatePopulatingNone)
{
    int signal_state = 0;
    wigwag::signal<void(int), wigwag::exception_handling::default_, wigwag::threading::default_, wigwag::state_populating::none> s;
    wigwag::thread th(
        [&](const std::atomic<bool>& alive)
        {
            wigwag::thread::sleep(500);
            auto l = wigwag::lock(s.lock_primitive());
            signal_state = 2;
            s(2);
        }
    );

    wigwag::mutexed<int> state;
    wigwag::token t = s.connect([&](int i) { state.set(i); });
    ASSERT_EQ(state.get(), 0);
    wigwag::thread::sleep(1000);
    ASSERT_EQ(state.get(), 2);
    t.reset();
    ASSERT_EQ(state.get(), 2);
}

// ============================================================================
// Life assurance policy tests
// ============================================================================

TEST(WigwagApiV1, LifeAssuranceDefault)
{ do_test_life_assurance_common<wigwag::signal<void()>>(); }

TEST(WigwagApiV1, LifeAssuranceIntrusiveLifeTokens)
{ do_test_life_assurance_common<wigwag::signal<void(), wigwag::exception_handling::default_, wigwag::threading::default_, wigwag::state_populating::default_, wigwag::life_assurance::intrusive_life_tokens>>(); }

TEST(WigwagApiV1, LifeAssuranceSingleThreaded)
{
    {
        std::shared_ptr<wigwag::threadless_task_executor> worker = std::make_shared<wigwag::threadless_task_executor>();
        wigwag::token_pool tp;
        wigwag::signal<void(), wigwag::exception_handling::default_, wigwag::threading::default_, wigwag::state_populating::default_, wigwag::life_assurance::single_threaded> s;
        tp += s.connect(worker, []{ });
        s();
    }

    {
        std::shared_ptr<wigwag::threadless_task_executor> worker = std::make_shared<wigwag::threadless_task_executor>();
        wigwag::signal<void(), wigwag::exception_handling::default_, wigwag::threading::default_, wigwag::state_populating::default_, wigwag::life_assurance::single_threaded> s;
        wigwag::token_pool tp;
        tp += s.connect(worker, []{ });
        s();
    }

    {
        std::shared_ptr<wigwag::threadless_task_executor> worker = std::make_shared<wigwag::threadless_task_executor>();
        wigwag::token_pool tp;
        wigwag::signal<void(), wigwag::exception_handling::default_, wigwag::threading::default_, wigwag::state_populating::default_, wigwag::life_assurance::single_threaded> s;
        tp += s.connect(worker, []{ });
        s();
        worker->process_tasks();
    }

    {
        std::shared_ptr<wigwag::threadless_task_executor> worker = std::make_shared<wigwag::threadless_task_executor>();
        wigwag::signal<void(), wigwag::exception_handling::default_, wigwag::threading::default_, wigwag::state_populating::default_, wigwag::life_assurance::single_threaded> s;
        wigwag::token_pool tp;
        tp += s.connect(worker, []{ });
        s();
        worker->process_tasks();
    }

    std::shared_ptr<wigwag::threadless_task_executor> worker = std::make_shared<wigwag::threadless_task_executor>();
    wigwag::signal<void(), wigwag::exception_handling::default_, wigwag::threading::default_, wigwag::state_populating::default_, wigwag::life_assurance::single_threaded> s;

    {
        wigwag::mutexed<bool> handler_invoked(false);
        wigwag::token t = s.connect(worker, [&]{ handler_invoked.set(true); });
        s();
        worker->process_tasks();
        t.reset();
        ASSERT_TRUE(handler_invoked.get());
    }

    {
        wigwag::mutexed<bool> handler_invoked(false);
        wigwag::token t = s.connect(worker, [&]{ handler_invoked.set(true); });
        s();
        t.reset();
        worker->process_tasks();
        ASSERT_FALSE(handler_invoked.get());
    }
}

TEST(WigwagApiV1, LifeAssuranceNone)
{
    wigwag::signal<void(), wigwag::exception_handling::default_, wigwag::threading::default_, wigwag::state_populating::default_, wigwag::life_assurance::none> s;

    bool handler_invoked = false;
    wigwag::token t = s.connect([&]{ handler_invoked  = true; });
    s();
    t.reset();
    ASSERT_TRUE(handler_invoked);
}

// ============================================================================
// Creation policy tests
// ============================================================================

TEST(WigwagApiV1, CreationAheadOfTime)
{
    {
        wigwag::signal<void(), wigwag::creation::ahead_of_time> s;
        s();
        wigwag::token t = s.connect([]{});
        s();
    }
    {
        wigwag::listenable<test_listener, wigwag::creation::ahead_of_time> l;
        l.invoke([](const test_listener& f) { f.f(); });
        wigwag::token t = l.connect(test_listener([] {}, [](int) {}));
        l.invoke([](const test_listener& f) { f.f(); });
    }
}

TEST(WigwagApiV1, CreationLazy)
{
    {
        wigwag::signal<void(), wigwag::creation::lazy> s;
        s();
        wigwag::token t = s.connect([]{});
        s();
    }
    {
        wigwag::listenable<test_listener, wigwag::creation::lazy> l;
        l.invoke([](const test_listener& f) { f.f(); });
        wigwag::token t = l.connect(test_listener([] {}, [](int) {}));
        l.invoke([](const test_listener& f) { f.f(); });
    }
}

// ============================================================================
// Life token tests
// ============================================================================

TEST(WigwagApiV1, LifeToken)
{
    {
        wigwag::life_token lt;

        wigwag::thread th(
            [&](const std::atomic<bool>& alive)
            {
                wigwag::life_token::execution_guard g(lt);
                if (g.is_alive())
                    wigwag::thread::sleep(300);
            });

        wigwag::thread::sleep(100);

        wigwag::profiler p;
        wigwag::life_token lt2(std::move(lt));
        auto move_time = duration_cast<milliseconds>(p.reset()).count();
        lt2.release();
        auto release_time = duration_cast<milliseconds>(p.reset()).count();
        ASSERT_LE(move_time, 100);
        ASSERT_LE(150, release_time);
    }

    {
        wigwag::life_token lt;

        wigwag::thread th(
            [&](const std::atomic<bool>& alive)
            {
                wigwag::thread::sleep(100);

                wigwag::life_token::execution_guard g(lt);
                if (g.is_alive())
                    wigwag::thread::sleep(300);
            });

        wigwag::profiler p;
        wigwag::life_token lt2(std::move(lt));
        auto move_time = duration_cast<milliseconds>(p.reset()).count();
        lt2.release();
        auto release_time = duration_cast<milliseconds>(p.reset()).count();
        ASSERT_LE(move_time, 100);
        ASSERT_LE(release_time, 100);
    }

    {
        wigwag::life_token lt;
        wigwag::life_token::checker lc(lt);

        wigwag::thread th(
            [&](const std::atomic<bool>& alive)
            {
                wigwag::life_token::execution_guard g(lc);
                if (g.is_alive())
                    wigwag::thread::sleep(300);
            });

        wigwag::thread::sleep(100);

        wigwag::profiler p;
        wigwag::life_token lt2(std::move(lt));
        auto move_time = duration_cast<milliseconds>(p.reset()).count();
        lt2.release();
        auto release_time = duration_cast<milliseconds>(p.reset()).count();
        ASSERT_LE(move_time, 100);
        ASSERT_LE(150, release_time);
    }

    {
        wigwag::life_token lt;
        wigwag::life_token::checker lc(lt);

        wigwag::thread th(
            [&](const std::atomic<bool>& alive)
            {
                wigwag::thread::sleep(100);

                wigwag::life_token::execution_guard g(lc);
                if (g.is_alive())
                    wigwag::thread::sleep(300);
            });

        wigwag::profiler p;
        wigwag::life_token lt2(std::move(lt));
        auto move_time = duration_cast<milliseconds>(p.reset()).count();
        lt2.release();
        auto release_time = duration_cast<milliseconds>(p.reset()).count();
        ASSERT_LE(move_time, 100);
        ASSERT_LE(release_time, 100);
    }
}

// ============================================================================
// Task executor tests
// ============================================================================

TEST(WigwagApiV1, TaskExecutors)
{
    {
        std::shared_ptr<wigwag::task_executor> worker = std::make_shared<wigwag::thread_task_executor>();

        wigwag::signal<void()> s;

        std::mutex m;
        std::thread::id handler_thread_id;

        wigwag::token t = s.connect(worker, [&]{
                std::lock_guard<std::mutex> l(m);
                handler_thread_id = std::this_thread::get_id();
            });

        s();

        wigwag::thread::sleep(500);

        std::lock_guard<std::mutex> l(m);
        ASSERT_NE(handler_thread_id, std::thread::id());
        ASSERT_NE(handler_thread_id, std::this_thread::get_id());
    }

    {
        std::shared_ptr<wigwag::task_executor> worker = std::make_shared<wigwag::thread_task_executor>();

        std::mutex m;
        int n = 0;

        for (int i = 0; i < 3; ++i)
            worker->add_task([&] {
                    wigwag::thread::sleep(200);
                    auto l = wigwag::lock(m);
                    ++n;
                });

        wigwag::profiler p;
        worker.reset();
        auto worker_dtor_time = duration_cast<milliseconds>(p.reset()).count();
        ASSERT_LE(500, worker_dtor_time);
        auto l = wigwag::lock(m);
        ASSERT_EQ(n, 3);
    }
}

// ============================================================================
// Copying tests
// ============================================================================

TEST(WigwagApiV1, TaskExecutorFunctionCopying)
{
#if !HAS_STD_FUNCTION_MOVE_BUG
    {
        std::atomic<int> counter(0);
        std::shared_ptr<wigwag::threadless_task_executor> worker = std::make_shared<wigwag::threadless_task_executor>();
        worker->add_task(copy_ctor_counter(counter));
        ASSERT_EQ(counter.load(), 0);
        worker->process_tasks();
        ASSERT_EQ(counter.load(), 0);
    }

    {
        std::atomic<int> counter(0);
        std::shared_ptr<wigwag::task_executor> worker = std::make_shared<wigwag::thread_task_executor>();
        worker->add_task([]{ wigwag::thread::sleep(100); });
        ASSERT_EQ(counter.load(), 0);
        wigwag::thread::sleep(200);
        ASSERT_EQ(counter.load(), 0);
    }
#endif
}

TEST(WigwagApiV1, SignalHandlerCopying)
{
#if !HAS_STD_FUNCTION_MOVE_BUG
    {
        wigwag::signal<void()> s;
        std::atomic<int> counter(0);

        wigwag::token t = s.connect(copy_ctor_counter(counter));
        ASSERT_EQ(counter.load(), 0);
        s();
        ASSERT_EQ(counter.load(), 0);
        t.reset();
        ASSERT_EQ(counter.load(), 0);
    }

    {
        std::shared_ptr<wigwag::threadless_task_executor> worker = std::make_shared<wigwag::threadless_task_executor>();
        wigwag::signal<void()> s;
        std::atomic<int> counter(0);

        wigwag::token t = s.connect(worker, copy_ctor_counter(counter), wigwag::handler_attributes::suppress_populator);
        ASSERT_EQ(counter.load(), 0);
        s();
        ASSERT_EQ(counter.load(), 1);
        worker->process_tasks();
        ASSERT_EQ(counter.load(), 1);
        t.reset();
        ASSERT_EQ(counter.load(), 1);
    }

    {
        std::shared_ptr<wigwag::threadless_task_executor> worker = std::make_shared<wigwag::threadless_task_executor>();
        wigwag::signal<void()> s([](const wigwag::signal<void()>::handler_type&) { });
        std::atomic<int> counter(0);

        wigwag::token t = s.connect(worker, copy_ctor_counter(counter));
        ASSERT_EQ(counter.load(), 1);
        worker->process_tasks();
        ASSERT_EQ(counter.load(), 1);
        s();
        ASSERT_EQ(counter.load(), 2);
        worker->process_tasks();
        ASSERT_EQ(counter.load(), 2);
        t.reset();
        ASSERT_EQ(counter.load(), 2);
    }
#endif
}

TEST(WigwagApiV1, SignalParametersCopying)
{
#if !HAS_STD_FUNCTION_MOVE_BUG
    {
        std::atomic<int> counter(0);
        copy_ctor_counter state(counter);
        wigwag::signal<void(const copy_ctor_counter&)> s([&](const std::function<void(const copy_ctor_counter&)>& h){ h(state); });

        wigwag::token t = s.connect([](const copy_ctor_counter& c) { });
        ASSERT_EQ(counter.load(), 0);
        s(copy_ctor_counter(counter));
        ASSERT_EQ(counter.load(), 0);
        t.reset();
        ASSERT_EQ(counter.load(), 0);
    }

    {
        std::shared_ptr<wigwag::threadless_task_executor> worker = std::make_shared<wigwag::threadless_task_executor>();
        std::atomic<int> counter(0);
        copy_ctor_counter state(counter);
        wigwag::signal<void(const copy_ctor_counter&)> s([&](const std::function<void(const copy_ctor_counter&)>& h){ h(state); });

        wigwag::token t = s.connect(worker, [](const copy_ctor_counter& c) { }, wigwag::handler_attributes::suppress_populator);
        ASSERT_EQ(counter.load(), 0);
        worker->process_tasks();
        ASSERT_EQ(counter.load(), 0);
        s(copy_ctor_counter(counter));
        ASSERT_EQ(counter.load(), 1);
        worker->process_tasks();
        ASSERT_EQ(counter.load(), 1);
        t.reset();
        ASSERT_EQ(counter.load(), 1);
    }

    {
        std::shared_ptr<wigwag::threadless_task_executor> worker = std::make_shared<wigwag::threadless_task_executor>();
        std::atomic<int> counter(0);
        copy_ctor_counter state(counter);
        wigwag::signal<void(const copy_ctor_counter&)> s([&](const std::function<void(const copy_ctor_counter&)>& h){ h(state); });

        wigwag::token t = s.connect(worker, [](const copy_ctor_counter& c) { });
        ASSERT_EQ(counter.load(), 1);
        worker->process_tasks();
        ASSERT_EQ(counter.load(), 1);
        s(copy_ctor_counter(counter));
        ASSERT_EQ(counter.load(), 2);
        worker->process_tasks();
        ASSERT_EQ(counter.load(), 2);
        t.reset();
        ASSERT_EQ(counter.load(), 2);
    }
#endif
}
