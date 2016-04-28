#ifndef SRC_BENCHMARKS_DESCRIPTORS_SIGNALS_BOOST_HPP
#define SRC_BENCHMARKS_DESCRIPTORS_SIGNALS_BOOST_HPP


#include <boost/make_shared.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/signals2.hpp>


namespace descriptors {
namespace signals {
namespace boost
{
	using namespace ::boost;


	template < typename Signature_, typename TrackablePtr_ >
	class TrackingSlotWrapper
	{
		using FuncType = std::function<Signature_>;
		using SlotType = signals2::slot<Signature_>;

	private:
		FuncType		_f;
		TrackablePtr_	_t;

	public:
		TrackingSlotWrapper(const FuncType& f, const TrackablePtr_& t)
			: _f(f), _t(t)
		{ }

		operator SlotType() const
		{
			SlotType result(_f);
			DoTrack(result, _t);
			return result;
		}

	private:
		template < typename Slot_, typename T_ >
		static void DoTrack(Slot_& s, const shared_ptr<T_>& t)
		{ s.track(t); }

		template < typename Slot_, typename T_ >
		static void DoTrack(Slot_& s, const T_& t, int dummy = 0)
		{ s.track_foreign(t); }
	};


	struct Tracking
	{
		using SignalType = signals2::signal<void()>;
		using HandlerType = TrackingSlotWrapper<void(), shared_ptr<std::string>>;
		using ConnectionType = signals2::scoped_connection;

		static HandlerType MakeHandler() { return HandlerType([]{}, make_shared<std::string>()); }
		static std::string GetName() { return "boost_tracking"; }
	};

	struct Regular
	{
		using SignalType = signals2::signal<void()>;
		using HandlerType = std::function<void()>;
		using ConnectionType = signals2::scoped_connection;

		static HandlerType MakeHandler() { return []{}; }
		static std::string GetName() { return "boost"; }
	};


}}}


#endif
