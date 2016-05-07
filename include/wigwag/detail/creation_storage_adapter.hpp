#ifndef WIGWAG_DETAIL_CREATION_STORAGE_ADAPTER_HPP
#define WIGWAG_DETAIL_CREATION_STORAGE_ADAPTER_HPP


#include <utility>


namespace wigwag {
namespace detail
{

	template < typename Storage_ >
	class creation_storage_adapter
	{
	private:
		Storage_		_s;

	public:
		creation_storage_adapter()
			: _s()
		{ }

		template < typename T_, typename... Args_ >
		void create(Args_&&... args)
		{ _s.template create<T_>(std::forward<Args_>(args)...); }

		explicit operator bool() const
		{ return _s.constructed(); }

		auto get_ptr() const -> decltype(_s.get_ptr())
		{ return _s.get_ptr(); }

		auto operator -> () const -> decltype(&*get_ptr())
		{ return &*get_ptr(); }
	};

}}

#endif
