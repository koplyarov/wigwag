#ifndef SRC_BENCHMARKS_DESCRIPTORS_SIGNALS_SIGCPP_HPP
#define SRC_BENCHMARKS_DESCRIPTORS_SIGNALS_SIGCPP_HPP

#if WIGWAG_BENCHMARKS_SIGCPP2

#include <sigc++/sigc++.h>


namespace sigcpp_descriptors
{

	class scoped_connection
	{
	private:
		sigc::connection		_connection;

	public:
		scoped_connection(sigc::connection c) : _connection(std::move(c)) { }
		~scoped_connection() { _connection.disconnect(); }

		scoped_connection(const scoped_connection&) = delete;
		scoped_connection& operator = (const scoped_connection&) = delete;
	};


	struct regular
	{
		using signal_type = sigc::signal<void>;
		using handler_type = std::function<void()>;
		using connection_type = scoped_connection;

		static handler_type make_handler() { return []{}; }
		static std::string GetName() { return "sigcpp"; }
	};

}

#endif

#endif
