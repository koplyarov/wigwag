#ifndef SRC_BENCHMARKS_ADAPTERS_BOOST_HPP
#define SRC_BENCHMARKS_ADAPTERS_BOOST_HPP


namespace boost_adapters
{

	template < typename Signature_, typename TrackablePtr_ >
	class tracking_slot_wrapper
	{
		using func_type = std::function<Signature_>;
		using slot_type = boost::signals2::slot<Signature_>;

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
		static void do_track(Slot_& s, const boost::shared_ptr<T_>& t)
		{ s.track(t); }

		template < typename Slot_, typename T_ >
		static void do_track(Slot_& s, const T_& t, int dummy = 0)
		{ s.track_foreign(t); }
	};


	template < typename Signature_, typename TrackablePtr_ >
	tracking_slot_wrapper<Signature_, TrackablePtr_> make_tracking_slot_wrapper(const std::function<Signature_>& f, const TrackablePtr_& t)
	{ return tracking_slot_wrapper<Signature_, TrackablePtr_>(f, t); }


}


#endif
