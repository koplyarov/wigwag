#ifndef SRC_BENCHMARKS_DESCRIPTORS_SIGNALS_SIGCPP_HPP
#define SRC_BENCHMARKS_DESCRIPTORS_SIGNALS_SIGCPP_HPP

#if WIGWAG_BENCHMARKS_SIGCPP2

#include <sigc++/sigc++.h>


namespace descriptors {
namespace signals {
namespace sigcpp
{

	class ScopedConnection
	{
	private:
		sigc::connection		_connection;

	public:
		ScopedConnection(sigc::connection c) : _connection(std::move(c)) { }
		~ScopedConnection() { _connection.disconnect(); }

		ScopedConnection(const ScopedConnection&) = delete;
		ScopedConnection& operator = (const ScopedConnection&) = delete;
	};


	struct Regular
	{
		using SignalType = sigc::signal<void>;
		using HandlerType = std::function<void()>;
		using ConnectionType = ScopedConnection;

		static HandlerType MakeHandler() { return []{}; }
		static std::string GetName() { return "sigcpp"; }
	};

}}}

#endif

#endif
