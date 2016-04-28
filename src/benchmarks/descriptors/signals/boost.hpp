#ifndef SRC_BENCHMARKS_DESCRIPTORS_SIGNALS_BOOST_HPP
#define SRC_BENCHMARKS_DESCRIPTORS_SIGNALS_BOOST_HPP


#include <boost/signals2/signal.hpp>


namespace descriptors {
namespace signals {
namespace boost
{
	using namespace ::boost;


	template < typename Signature_, typename TrackablePtr_ >
	class tracking_slot_wrapper
	{
		using func_type = std::function<Signature_>;
		using slot_type = signals2::slot<Signature_>;

	private:
		func_type		_f;
		TrackablePtr_	_t;

	public:
		tracking_slot_wrapper(const func_type& f, const TrackablePtr_& t)
			: _f(f), _t(t)
		{ }

		operator slot_type() const
		{
			slot_type result(_f);
			do_track(result, _t);
			return result;
		}

	private:
		template < typename Slot_, typename T_ >
		static void do_track(Slot_& s, const shared_ptr<T_>& t)
		{ s.track(t); }

		template < typename Slot_, typename T_ >
		static void do_track(Slot_& s, const T_& t, int dummy = 0)
		{ s.track_foreign(t); }
	};


	template < typename Signature_, typename TrackablePtr_ >
	tracking_slot_wrapper<Signature_, TrackablePtr_> make_tracking_slot_wrapper(const std::function<Signature_>& f, const TrackablePtr_& t)
	{ return tracking_slot_wrapper<Signature_, TrackablePtr_>(f, t); }


	struct tracking
	{
		using signal_type = signals2::signal<void()>;
		using handler_type = tracking_slot_wrapper<void(), shared_ptr<std::string>>;
		using connection_type = signals2::scoped_connection;

		static handler_type make_handler() { return handler_type([]{}, make_shared<std::string>()); }
		static std::string GetName() { return "boost_tracking"; }
	};

	struct regular
	{
		using signal_type = signals2::signal<void()>;
		using handler_type = std::function<void()>;
		using connection_type = signals2::scoped_connection;

		static handler_type make_handler() { return []{}; }
		static std::string GetName() { return "boost"; }
	};


}}}


#endif
