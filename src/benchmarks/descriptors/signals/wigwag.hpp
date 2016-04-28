#ifndef SRC_BENCHMARKS_DESCRIPTORS_SIGNALS_WIGWAG_HPP
#define SRC_BENCHMARKS_DESCRIPTORS_SIGNALS_WIGWAG_HPP


#include <wigwag/signal.hpp>


namespace wigwag_descriptors
{

	template < typename Signature_ >
	using ui_signal = wigwag::signal<Signature_, wigwag::exception_handling::none, wigwag::threading::none, wigwag::state_populating::none, wigwag::life_assurance::none>;


	struct regular
	{
		using signal_type = wigwag::signal<void()>;
		using handler_type = std::function<void()>;
		using connection_type = wigwag::token;

		static handler_type make_handler() { return []{}; }
		static std::string GetName() { return "wigwag"; }
	};


	struct ui
	{
		using signal_type = ui_signal<void()>;
		using handler_type = std::function<void()>;
		using connection_type = wigwag::token;

		static handler_type make_handler() { return []{}; }
		static std::string GetName() { return "wigwag_ui"; }
	};

}

#endif
