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


using namespace wigwag;


template < int >
class constructor_tag
{ };


class observable_int
{
private:
	int					_value;
	signal<void(int)>	_on_changed;

public:
	observable_int(constructor_tag<1>)
		: _value(0), _on_changed(std::bind(&observable_int::on_changed_populator, this, std::placeholders::_1))
	{ }

	observable_int(constructor_tag<2>)
		: _value(0), _on_changed([&](const std::function<void(int)>& h){ h(this->_value); })
	{ }

	observable_int(constructor_tag<3>)
		: _value(0), _on_changed([&](const std::function<void(int)>& h){ h(_value); })
	{ }


	signal_connector<void(int)> on_changed() const
	{ return _on_changed.connector(); }

	const signal<void(int)>& on_changed_ref() const
	{ return _on_changed; }


	void set_value(int value)
	{
		std::lock_guard<decltype(_on_changed.lock_primitive())> l(_on_changed.lock_primitive());
		_value = value;
		_on_changed(_value);
	}

private:
	void on_changed_populator(const std::function<void(int)>& handler)
	{ handler(_value); }
};


class int_observer
{
private:
	mutable std::mutex				_mutex { };
	int								_value { };
	token_pool						_tokens { };
	std::shared_ptr<task_executor>	_worker { std::make_shared<thread_task_executor>() };

public:
	int_observer(const observable_int& i)
	{
		_tokens += i.on_changed().connect(_worker, std::bind(&int_observer::int_changed_async_handler, this, std::placeholders::_1));
		_tokens += i.on_changed().connect(std::bind(&int_observer::int_changed_sync_handler, this, std::placeholders::_1));
	}

private:
	void int_changed_async_handler(int i)
	{
		std::lock_guard<std::mutex> l(_mutex);
		_value = i;
	}

	void int_changed_sync_handler(int)
	{ }
};


class crazy_signals
{
public:
	signal<void(const std::function<void(int)>& f)>			on_func;
	signal<void(std::string& s)>							on_string_ref;

	void test_connect()
	{
		std::shared_ptr<task_executor>	worker = std::make_shared<thread_task_executor>();
		token_pool tp;

		tp += on_func.connect(std::bind(&crazy_signals::func_handler, this, std::placeholders::_1));
		tp += on_string_ref.connect(std::bind(&crazy_signals::string_ref_handler, this, std::placeholders::_1));

		tp += on_func.connect(worker, std::bind(&crazy_signals::func_handler, this, std::placeholders::_1));
		tp += on_string_ref.connect(worker, std::bind(&crazy_signals::string_ref_handler, this, std::placeholders::_1));
	}

	void test_invoke()
	{
		std::string s;

		on_func([](int){ });
		on_func(std::bind([](const std::string&, int){ }, "qwe", std::placeholders::_1));
		on_string_ref(s);
		on_string_ref(std::ref(s));
	}

private:
	void func_handler(const std::function<void(int)>& f) { f(42); }
	void string_ref_handler(std::string& s) { s = "qwe"; }
};


class instantiations_test
{
public:
	signal<void(), exception_handling::none> s1;
	signal<void(), threading::shared_recursive_mutex> s2;
	signal<void(), life_assurance::none, state_populating::none> s3;
	signal<void(), threading::shared_recursive_mutex, creation::lazy> s4;

	listenable<std::function<void()>, exception_handling::none> l1;
	listenable<std::function<void()>, threading::shared_recursive_mutex> l2;
	listenable<std::function<void()>, life_assurance::none, state_populating::none> l3;

	instantiations_test()
		:	s1(),
			s2(std::make_shared<std::recursive_mutex>()),
			s3(),
			s4(std::make_shared<std::recursive_mutex>()),
			l1(),
			l2(std::make_shared<std::recursive_mutex>()),
			l3()
	{ }

	void f()
	{
		s1.connect([]{});
		s2.connect([]{});
		s3.connect([]{});
		s4.connect([]{});
		l1.connect([]{});
		l2.connect([]{});
		l3.connect([]{});
	}

	void f() const
	{
		s1();
		s2();
		s3();
		s4();
		l1.invoke([](const std::function<void()>& f){ f(); });
		l2.invoke([](const std::function<void()>& f){ f(); });
		l3.invoke([](const std::function<void()>& f){ f(); });
	}
};
