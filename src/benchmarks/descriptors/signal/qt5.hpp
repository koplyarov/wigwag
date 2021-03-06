#ifndef SRC_BENCHMARKS_DESCRIPTORS_SIGNALS_QT5_HPP
#define SRC_BENCHMARKS_DESCRIPTORS_SIGNALS_QT5_HPP

#if WIGWAG_BENCHMARKS_QT5

#define QT_NO_KEYWORDS
#include <QObject>


namespace descriptors {
namespace signal {
namespace qt5
{

	class SlotOwner : public QObject
	{
		Q_OBJECT

	public:
		SlotOwner() { }
		SlotOwner(const SlotOwner& other) { }

	public Q_SLOTS:
		void testSlot() { }
	};


	class SlotWrapper
	{
	private:
		SlotOwner		_slotOwner;
		const char*		_slotName;

	public:
		SlotWrapper(const SlotOwner& slotOwner, const char* slotName)
			: _slotOwner(slotOwner), _slotName(slotName)
		{ }

		const SlotOwner* getSlotOwner() const { return &_slotOwner; }
		const char* getSlotName() const { return _slotName; }
	};


	class SignalConnectionWrapper
	{
	private:
		QMetaObject::Connection		_c;

	public:
		SignalConnectionWrapper(const QMetaObject::Connection c) : _c(c) { }
		~SignalConnectionWrapper() { QObject::disconnect(_c); }

		SignalConnectionWrapper(const SignalConnectionWrapper&) = delete;
		SignalConnectionWrapper& operator = (const SignalConnectionWrapper&) = delete;
	};


	class SignalOwner : public QObject
	{
		Q_OBJECT

	public:
		SignalOwner() { }

		void invokeSignal() { Q_EMIT testSignal(); }

		QMetaObject::Connection connect(const SlotWrapper& slotWrapper) const
		{ return QObject::connect(this, SIGNAL(testSignal()), slotWrapper.getSlotOwner(), slotWrapper.getSlotName()); }

		void operator () ()
		{ Q_EMIT testSignal(); }

	Q_SIGNALS:
		void testSignal();
	};


	struct Regular
	{
		using SignalType = SignalOwner;
		using HandlerType = SlotWrapper;
		using ConnectionType = SignalConnectionWrapper;

		static HandlerType MakeHandler() { return SlotWrapper(SlotOwner(), SLOT(testSlot())); }
		static std::string GetName() { return "qt5"; }
	};

}}}

#endif

#endif
