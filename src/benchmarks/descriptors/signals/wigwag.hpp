#ifndef SRC_BENCHMARKS_DESCRIPTORS_SIGNALS_WIGWAG_HPP
#define SRC_BENCHMARKS_DESCRIPTORS_SIGNALS_WIGWAG_HPP


#include <wigwag/signal.hpp>


namespace descriptors {
namespace signals {
namespace wigwag
{

	using namespace ::wigwag;

	template < typename Signature_ >
	using ui_signal = signal<Signature_, exception_handling::none, threading::none, state_populating::none, life_assurance::none>;


	struct regular
	{
		using SignalType = signal<void()>;
		using HandlerType = std::function<void()>;
		using ConnectionType = token;

		static HandlerType MakeHandler() { return []{}; }
		static std::string GetName() { return "wigwag"; }
	};


	struct ui
	{
		using SignalType = ui_signal<void()>;
		using HandlerType = std::function<void()>;
		using ConnectionType = token;

		static HandlerType MakeHandler() { return []{}; }
		static std::string GetName() { return "wigwag_ui"; }
	};

}}}

#endif
