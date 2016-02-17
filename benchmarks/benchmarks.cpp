// Copyright (c) 2016, Dmitry Koplyarov <koplyarov.da@gmail.com>
//
// Permission to use, copy, modify, and/or distribute this software for any purpose with or without fee is hereby granted,
// provided that the above copyright notice and this permission notice appear in all copies.
//
// THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS.
// IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS,
// WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.


#include <wigwag/signal.hpp>

#include <boost/make_shared.hpp>
#include <boost/program_options.hpp>
#include <boost/signals2.hpp>
#include <boost/thread.hpp>

#include <chrono>
#include <iostream>
#include <thread>

#include <benchmarks/markers.hpp>
#include <benchmarks/storage_for.hpp>
#include <benchmarks/thread_priority.hpp>
#include <test/profiler.hpp>


using namespace wigwag;


std::function<void()> g_empty_handler([]{});


template < typename T_ >
void create(int64_t n)
{
	storage_for<T_> *v = new storage_for<T_>[n];

	{
		operation_profiler op("creating");
		for (int64_t i = 0; i < n; ++i)
			new(&v[i].obj) T_();
	}

	measure_memory();

	{
		operation_profiler op("destroying");
		for (int64_t i = 0; i < n; ++i)
			v[i].obj.~T_();
	}

	delete[] v;
}


template < typename T_ >
void create_lock_unlock(int64_t n)
{
	storage_for<T_> *v = new storage_for<T_>[n];

	{
		operation_profiler op("creating");
		for (int64_t i = 0; i < n; ++i)
			new(&v[i].obj) T_();
	}

	measure_memory();

	{
		operation_profiler op("locking");
		for (int64_t i = 0; i < n; ++i)
			v[i].obj.lock();
	}

	{
		operation_profiler op("unlocking");
		for (int64_t i = 0; i < n; ++i)
			v[i].obj.unlock();
	}

	{
		operation_profiler op("destroying");
		for (int64_t i = 0; i < n; ++i)
			v[i].obj.~T_();
	}

	delete[] v;
}


template < typename Signal_, typename Connection_ >
void connect(int64_t n)
{
	Signal_ s;
	storage_for<Connection_> *v = new storage_for<Connection_>[n];

	{
		operation_profiler op("connecting");
		for (int64_t i = 0; i < n; ++i)
			new(&v[i].obj) Connection_(s.connect(g_empty_handler));
	}

	{
		operation_profiler op("invoking");
		s();
	}

	measure_memory();
}


template < typename Signal_, typename Connection_ >
void connect_tracked(int64_t n)
{
	Signal_ s;
	storage_for<Connection_> *v = new storage_for<Connection_>[n];

	boost::shared_ptr<std::string> tracked(new std::string);

	{
		operation_profiler op("connecting");
		for (int64_t i = 0; i < n; ++i)
		{
			typename Signal_::slot_type slot(g_empty_handler);
			slot.track(tracked);
			new(&v[i].obj) Connection_(s.connect(slot));
		}
	}

	{
		operation_profiler op("invoking");
		s();
	}

	measure_memory();
}


template < typename Signal_, typename Connection_ >
void invoke(int64_t n)
{
	Signal_ s;
	Connection_ c = s.connect(g_empty_handler);

	{
		operation_profiler op("invoking");
		for (int64_t i = 0; i < n; ++i)
			s();
	}
}


template < typename Signal_, typename Connection_ >
void invoke_tracked(int64_t n)
{
	Signal_ s;
	typename Signal_::slot_type slot(g_empty_handler);
	slot.track(boost::make_shared<std::string>());
	Connection_ c = s.connect(slot);

	{
		operation_profiler op("invoking");
		for (int64_t i = 0; i < n; ++i)
			s();
	}
}


template < typename Signature_ >
using ui_signal = signal<Signature_, exception_handling::none, threading::none, state_populating::none, life_assurance::none>;

template < typename Signature_ >
using ui_signal2 = signal<Signature_, exception_handling::none, threading::none, state_populating::none, life_assurance::life_tokens>;


struct cmdline_exception : public std::runtime_error
{ cmdline_exception(const std::string& msg) : std::runtime_error(msg) { } };


int main(int argc, char* argv[])
{
	try
	{
		set_max_thread_priority();

		std::string task, obj;
		int64_t count = 0;

		boost::program_options::options_description desc;
		desc.add_options()
			("help,h", "Show help")
			("task,t", boost::program_options::value<std::string>(&task), "Set task: create, connect")
			("obj,o", boost::program_options::value<std::string>(&obj), "Set object: signal, ui_signal, boost_signal2")
			("count,c", boost::program_options::value<int64_t>(&count), "Set iterations count")
			;

		boost::program_options::parsed_options parsed = boost::program_options::command_line_parser(argc, argv).options(desc).allow_unregistered().run();

		boost::program_options::variables_map vm;
		boost::program_options::store(parsed, vm);
		boost::program_options::notify(vm);

		if (vm.count("help"))
			throw cmdline_exception(boost::lexical_cast<std::string>(desc));
		if (vm.count("count") == 0)
			throw cmdline_exception("count option is required");
		if (vm.count("obj") == 0)
			throw cmdline_exception("obj option is required");


		if (task == "create")
		{
			if (obj == "mutex")
				create<std::mutex>(count);
			else if (obj == "recursive_mutex")
				create<std::recursive_mutex>(count);
			else if (obj == "condition_variable")
				create<std::condition_variable>(count);

			else if (obj == "boost_mutex")
				create<boost::mutex>(count);
			else if (obj == "boost_recursive_mutex")
				create<boost::recursive_mutex>(count);
			else if (obj == "boost_condition_variable")
				create<boost::condition_variable>(count);

			else if (obj == "life_token")
				create<life_token>(count);

			else if (obj == "signal")
				create<signal<void()>>(count);
			else if (obj == "ui_signal")
				create<ui_signal<void()>>(count);
			else if (obj == "ui_signal_life_tokens")
				create<ui_signal2<void()>>(count);

			else if (obj == "boost_signal2")
				create<boost::signals2::signal<void()>>(count);

			else
				throw cmdline_exception("obj " + obj + " not supported");
		}
		else if (task == "create_lock_unlock")
		{
			if (obj == "mutex")
				create_lock_unlock<std::mutex>(count);
			else if (obj == "recursive_mutex")
				create_lock_unlock<std::recursive_mutex>(count);

			else if (obj == "boost_mutex")
				create_lock_unlock<boost::mutex>(count);
			else if (obj == "boost_recursive_mutex")
				create_lock_unlock<boost::recursive_mutex>(count);

			else
				throw cmdline_exception("obj " + obj + " not supported");
		}
		else if (task == "connect")
		{
			if (obj == "signal")
				connect<signal<void()>, token>(count);
			else if (obj == "ui_signal")
				connect<ui_signal<void()>, token>(count);
			else if (obj == "ui_signal_life_tokens")
				connect<ui_signal2<void()>, token>(count);

			else if (obj == "boost_signal2")
				connect<boost::signals2::signal<void()>, boost::signals2::connection>(count);

			else
				throw cmdline_exception("obj " + obj + " not supported");
		}
		else if (task == "connect_tracked")
		{
			if (obj == "boost_signal2")
				connect_tracked<boost::signals2::signal<void()>, boost::signals2::connection>(count);
			else
				throw cmdline_exception("obj " + obj + " not supported");
		}
		else if (task == "invoke")
		{
			if (obj == "signal")
				invoke<signal<void()>, token>(count);
			else if (obj == "ui_signal")
				invoke<ui_signal<void()>, token>(count);
			else if (obj == "ui_signal_life_tokens")
				invoke<ui_signal2<void()>, token>(count);

			else if (obj == "boost_signal2")
				invoke<boost::signals2::signal<void()>, boost::signals2::connection>(count);

			else
				throw cmdline_exception("obj " + obj + " not supported");
		}
		else if (task == "invoke_tracked")
		{
			if (obj == "boost_signal2")
				invoke_tracked<boost::signals2::signal<void()>, boost::signals2::connection>(count);
			else
				throw cmdline_exception("obj " + obj + " not supported");
		}
		else
			std::cerr << "Task not specified!" << std::endl;
	}
	catch (const cmdline_exception& ex)
	{
		std::cerr << ex.what() << std::endl;
		return 1;
	}
	catch (const std::exception& ex)
	{
		std::cerr << "Uncaught exception: " << ex.what() << std::endl;
		return 1;
	}
	return 0;
}
