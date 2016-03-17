// Copyright (c) 2016, Dmitry Koplyarov <koplyarov.da@gmail.com>
//
// Permission to use, copy, modify, and/or distribute this software for any purpose with or without fee is hereby granted,
// provided that the above copyright notice and this permission notice appear in all copies.
//
// THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS.
// IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS,
// WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.


#include <wigwag/life_token.hpp>
#include <wigwag/signal.hpp>

#include <boost/core/typeinfo.hpp>
#include <boost/make_shared.hpp>
#include <boost/program_options.hpp>
#include <boost/signals2.hpp>
#include <boost/thread.hpp>

#include <chrono>
#include <iostream>
#include <thread>

#include <benchmarks/markers.hpp>
#include <utils/storage_for.hpp>
#include <utils/thread_priority.hpp>
#include <utils/profiler.hpp>

#if WIGWAG_BENCHMARKS_SIGCPP2
#	include <sigc++/sigc++.h>
#endif

#if WIGWAG_BENCHMARKS_QT5
#	include <QObject>
#endif


using namespace wigwag;


std::function<void()> g_empty_handler([]{});


#if WIGWAG_BENCHMARKS_QT5
class SignalOwner : public QObject
{
    Q_OBJECT

public:
    SignalOwner() { }

    void invokeSignal() { emit testSignal(); }

signals:
    void testSignal();
};

class SlotOwner : public QObject
{
    Q_OBJECT

public:
    SlotOwner() { }

public slots:
    void testSlot() { }
};

#include <benchmarks.moc>
#endif


template < typename Connection_ >
class scoped_connection
{
private:
	Connection_		_connection;
	bool			_moved;

public:
	scoped_connection(Connection_ c)
		: _connection(std::move(c)), _moved(false)
	{ }

	scoped_connection(scoped_connection&& other)
		: _connection(std::move(other._connection)), _moved(false)
	{ other._moved = true; }

	~scoped_connection()
	{
		if (!_moved)
			_connection.disconnect();
	}

	scoped_connection(const scoped_connection&) = delete;
	scoped_connection& operator = (const scoped_connection&) = delete;
};


template < typename T_ >
void create(int64_t n)
{
	storage_for<T_> *v = new storage_for<T_>[(size_t)n];

	{
		operation_profiler op("creating", n);
		for (int64_t i = 0; i < n; ++i)
			new(&v[i].obj) T_();
	}

	measure_memory("object", n);

	{
		operation_profiler op("destroying", n);
		for (int64_t i = 0; i < n; ++i)
			v[i].obj.~T_();
	}

	delete[] v;
}


template < typename T_ >
void create_lock_unlock(int64_t n)
{
	storage_for<T_> *l = new storage_for<T_>[(size_t)n];

	{
		operation_profiler op("creating", n);
		for (int64_t i = 0; i < n; ++i)
			new(&l[i].obj) T_();
	}

	measure_memory("object", n);

	{
		operation_profiler op("locking", n);
		for (int64_t i = 0; i < n; ++i)
			l[i].obj.lock();
	}

	{
		operation_profiler op("unlocking", n);
		for (int64_t i = 0; i < n; ++i)
			l[i].obj.unlock();
	}

	{
		operation_profiler op("destroying", n);
		for (int64_t i = 0; i < n; ++i)
			l[i].obj.~T_();
	}

	delete[] l;
}


template < typename T_, typename Functor_ >
void create_function(int64_t n, const Functor_& f)
{
	storage_for<T_> *v = new storage_for<T_>[(size_t)n];

	{
		operation_profiler op("creating", n);
		for (int64_t i = 0; i < n; ++i)
			new(&v[i].obj) T_(f);
	}

	measure_memory("object", n);

	{
		operation_profiler op("destroying", n);
		for (int64_t i = 0; i < n; ++i)
			v[i].obj.~T_();
	}

	delete[] v;
}


template < typename Signal_, typename Connection_ >
void connect_invoke(int64_t num_slots, int64_t num_calls)
{
	Signal_ s;
	storage_for<Connection_> *c = new storage_for<Connection_>[(size_t)num_slots];

	{
		operation_profiler op("connecting", num_slots);
		for (int64_t i = 0; i < num_slots; ++i)
			new(&c[i].obj) Connection_(s.connect(g_empty_handler));
	}

	measure_memory("connection", num_slots);

	{
		operation_profiler op("invoking", num_slots * num_calls);
		for (int64_t i = 0; i < num_calls; ++i)
			s();
	}

	{
		operation_profiler op("disconnecting", num_slots);
		for (int64_t i = 0; i < num_slots; ++i)
			c[i].obj.~Connection_();
	}

	delete[] c;
}


template < typename Signal_, typename Connection_ >
void connect_invoke_tracking(int64_t num_slots, int64_t num_calls)
{
	Signal_ s;
	storage_for<Connection_> *c = new storage_for<Connection_>[(size_t)num_slots];

	boost::shared_ptr<std::string> tracked(new std::string);

	{
		operation_profiler op("connecting", num_slots);
		for (int64_t i = 0; i < num_slots; ++i)
		{
			typename Signal_::slot_type slot(g_empty_handler);
			slot.track(tracked);
			new(&c[i].obj) Connection_(s.connect(slot));
		}
	}

	measure_memory("connection", num_slots);

	{
		operation_profiler op("invoking", num_slots * num_calls);
		for (int64_t i = 0; i < num_calls; ++i)
			s();
	}

	{
		operation_profiler op("disconnecting", num_slots);
		for (int64_t i = 0; i < num_slots; ++i)
			c[i].obj.~Connection_();
	}

	delete[] c;
}


template < typename Signal_, typename Connection_ >
void connect_disconnect(int64_t num_slots, int64_t num_iterations)
{
	Signal_ *s = new Signal_[(size_t)num_iterations];
	storage_for<Connection_> *c = new storage_for<Connection_>[(size_t)(num_slots * num_iterations)];

	{
		operation_profiler op("connecting", num_slots * num_iterations);
		for (int64_t j = 0; j < num_iterations; ++j)
			for (int64_t i = 0; i < num_slots; ++i)
				new(&c[i + j * num_slots].obj) Connection_(s[j].connect(g_empty_handler));
	}

	{
		operation_profiler op("disconnecting", num_slots * num_iterations);
		for (int64_t i = 0; i < num_slots * num_iterations; ++i)
			c[i].obj.~Connection_();
	}

	delete[] c;
}


template < typename Signal_, typename Connection_ >
void connect_disconnect_tracking(int64_t num_slots, int64_t num_iterations)
{
	Signal_ *s = new Signal_[(size_t)num_iterations];
	storage_for<Connection_> *c = new storage_for<Connection_>[(size_t)(num_slots * num_iterations)];

	boost::shared_ptr<std::string> tracked(new std::string);

	{
		operation_profiler op("connecting", num_slots * num_iterations);
		for (int64_t j = 0; j < num_iterations; ++j)
			for (int64_t i = 0; i < num_slots; ++i)
			{
				typename Signal_::slot_type slot(g_empty_handler);
				slot.track(tracked);
				new(&c[i + j * num_slots].obj) Connection_(s[j].connect(slot));
			}
	}

	{
		operation_profiler op("disconnecting", num_slots * num_iterations);
		for (int64_t i = 0; i < num_slots * num_iterations; ++i)
			c[i].obj.~Connection_();
	}

	delete[] c;
}


#if WIGWAG_BENCHMARKS_QT5
void connect_invoke_qt5(int64_t num_slots, int64_t num_calls)
{
	SignalOwner s;
	SlotOwner *h = new SlotOwner[(size_t)num_slots];
	QMetaObject::Connection *c = new QMetaObject::Connection[(size_t)(num_slots)];

	{
		operation_profiler op("connecting", num_slots);
		for (int64_t i = 0; i < num_slots; ++i)
			c[i] = QObject::connect(&s, SIGNAL(testSignal()), &h[i], SLOT(testSlot()));
	}

	measure_memory("connection", num_slots);

	{
		operation_profiler op("invoking", num_slots * num_calls);
		for (int64_t i = 0; i < num_calls; ++i)
			s.invokeSignal();
	}

	{
		operation_profiler op("disconnecting", num_slots);
		for (int64_t i = 0; i < num_slots; ++i)
			QObject::disconnect(c[i]);
	}

	delete[] h;
}

void connect_disconnect_qt5(int64_t num_slots, int64_t num_iterations)
{
	SignalOwner *s = new SignalOwner[(size_t)num_iterations];
	SlotOwner *h = new SlotOwner[(size_t)(num_slots * num_iterations)];
	QMetaObject::Connection *c = new QMetaObject::Connection[(size_t)(num_slots * num_iterations)];

	{
		operation_profiler op("connecting", num_slots * num_iterations);
		for (int64_t j = 0; j < num_iterations; ++j)
			for (int64_t i = 0; i < num_slots; ++i)
				c[i + j * num_slots] = QObject::connect(&s[j], SIGNAL(testSignal()), &h[i + j * num_slots], SLOT(testSlot()));
	}

	{
		operation_profiler op("disconnecting a single slot", num_slots * num_iterations);
		for (int64_t i = 0; i < num_slots * num_iterations; ++i)
			QObject::disconnect(c[i]);
	}

	delete[] h;
}
#endif


template < typename Signature_ >
using ui_signal = signal<Signature_, exception_handling::none, threading::none, state_populating::none, life_assurance::none>;


struct cmdline_exception : public std::runtime_error
{ cmdline_exception(const std::string& msg) : std::runtime_error(msg) { } };


int main(int argc, char* argv[])
{
	try
	{
		set_max_thread_priority();

		std::string task, obj;
		int64_t count = 0, secondary_count = 0;

		boost::program_options::options_description desc;
		desc.add_options()
			("help", "Show help")
			("task", boost::program_options::value<std::string>(&task), "Set task: create, connect")
			("obj", boost::program_options::value<std::string>(&obj), "Set object: signal, ui_signal, boost_signal2")
			("count", boost::program_options::value<int64_t>(&count), "Set iterations count")
			("secondary-count", boost::program_options::value<int64_t>(&secondary_count), "Set iterations count")
			;

		boost::program_options::parsed_options parsed = boost::program_options::command_line_parser(argc, argv).options(desc).allow_unregistered().run();

		boost::program_options::variables_map vm;
		boost::program_options::store(parsed, vm);
		boost::program_options::notify(vm);

		if (vm.count("help"))
			throw cmdline_exception(boost::lexical_cast<std::string>(desc));
		if (vm.count("task") == 0)
			throw cmdline_exception("obj option is required");
		if (vm.count("obj") == 0)
			throw cmdline_exception("obj option is required");
		if (vm.count("count") == 0)
			throw cmdline_exception("count option is required");

		if (task == "create")
		{
			if (obj == "std_mutex")
				create<std::mutex>(count);
			else if (obj == "std_recursive_mutex")
				create<std::recursive_mutex>(count);
			else if (obj == "std_condition_variable")
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

			else if (obj == "boost_signal2")
				create<boost::signals2::signal<void()>>(count);

			else if (obj == "sigc_signal")
#if WIGWAG_BENCHMARKS_SIGCPP2
				create<sigc::signal<void>>(count);
#else
				throw cmdline_exception("wigwag_benchmarks were built without sigc++ support");
#endif

			else if (obj == "qt5_signal")
#if WIGWAG_BENCHMARKS_QT5
				create<SignalOwner>(count);
#else
				throw cmdline_exception("wigwag_benchmarks were built without Qt5 support");
#endif

			else
				throw cmdline_exception("obj " + obj + " not supported");
		}
		else if (task == "create_lock_unlock")
		{
			if (obj == "std_mutex")
				create_lock_unlock<std::mutex>(count);
			else if (obj == "std_recursive_mutex")
				create_lock_unlock<std::recursive_mutex>(count);

			else if (obj == "boost_mutex")
				create_lock_unlock<boost::mutex>(count);
			else if (obj == "boost_recursive_mutex")
				create_lock_unlock<boost::recursive_mutex>(count);

			else
				throw cmdline_exception("obj " + obj + " not supported");
		}
		else if (task == "create_function")
		{
			if (obj == "std_function")
				create_function<std::function<void()>>(count, []{});

			else if (obj == "boost_function")
				create_function<boost::function<void()>>(count, []{});

			else
				throw cmdline_exception("obj " + obj + " not supported");
		}
		else if (task == "connect_invoke")
		{
			if (vm.count("secondary-count") == 0)
				throw cmdline_exception("secondary-count option is required");

			if (obj == "signal")
				connect_invoke<signal<void()>, token>(count, secondary_count);
			else if (obj == "ui_signal")
				connect_invoke<ui_signal<void()>, token>(count, secondary_count);

			else if (obj == "boost_signal2")
				connect_invoke<boost::signals2::signal<void()>, boost::signals2::scoped_connection>(count, secondary_count);

			else if (obj == "sigc_signal")
#if WIGWAG_BENCHMARKS_SIGCPP2
				connect_invoke<sigc::signal<void>, scoped_connection<sigc::connection>>(count, secondary_count);
#else
				throw cmdline_exception("wigwag_benchmarks were built without sigc++ support");
#endif

			else if (obj == "qt5_signal")
#if WIGWAG_BENCHMARKS_QT5
				connect_invoke_qt5(count, secondary_count);
#else
				throw cmdline_exception("wigwag_benchmarks were built without Qt5 support");
#endif

			else
				throw cmdline_exception("obj " + obj + " not supported");
		}
		else if (task == "connect_invoke_tracking")
		{
			if (vm.count("secondary-count") == 0)
				throw cmdline_exception("secondary-count option is required");

			if (obj == "boost_signal2")
				connect_invoke_tracking<boost::signals2::signal<void()>, boost::signals2::scoped_connection>(count, secondary_count);
			else
				throw cmdline_exception("obj " + obj + " not supported");
		}
		else if (task == "connect_disconnect")
		{
			if (vm.count("secondary-count") == 0)
				throw cmdline_exception("secondary-count option is required");

			if (obj == "signal")
				connect_disconnect<signal<void()>, token>(count, secondary_count);
			else if (obj == "ui_signal")
				connect_disconnect<ui_signal<void()>, token>(count, secondary_count);

			else if (obj == "boost_signal2")
				connect_disconnect<boost::signals2::signal<void()>, boost::signals2::scoped_connection>(count, secondary_count);

			else if (obj == "sigc_signal")
#if WIGWAG_BENCHMARKS_SIGCPP2
				connect_disconnect<sigc::signal<void>, scoped_connection<sigc::connection>>(count, secondary_count);
#else
				throw cmdline_exception("wigwag_benchmarks were built without sigc++ support");
#endif

			else if (obj == "qt5_signal")
#if WIGWAG_BENCHMARKS_QT5
				connect_disconnect_qt5(count, secondary_count);
#else
				throw cmdline_exception("wigwag_benchmarks were built without Qt5 support");
#endif

			else
				throw cmdline_exception("obj " + obj + " not supported");
		}
		else if (task == "connect_disconnect_tracking")
		{
			if (vm.count("secondary-count") == 0)
				throw cmdline_exception("secondary-count option is required");

			if (obj == "boost_signal2")
				connect_disconnect_tracking<boost::signals2::signal<void()>, boost::signals2::scoped_connection>(count, secondary_count);
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
