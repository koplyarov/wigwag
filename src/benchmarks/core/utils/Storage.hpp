#ifndef BENCHMARKS_CORE_UTILS_STORAGE_HPP
#define BENCHMARKS_CORE_UTILS_STORAGE_HPP

// Copyright (c) 2016, Dmitry Koplyarov <koplyarov.da@gmail.com>
//
// Permission to use, copy, modify, and/or distribute this software for any purpose with or without fee is hereby granted,
// provided that the above copyright notice and this permission notice appear in all copies.
//
// THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS.
// IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS,
// WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.


namespace benchmarks
{

	template < typename T_ >
	union Storage
	{
	private:
		T_		_obj;

	public:
		Storage() { }
		~Storage() { }

		template < typename... Args_ >
		void Construct(Args_&&... args)
		{ new(&_obj) T_(std::forward<Args_>(args)...); }

		void Destruct()
		{ _obj.~T_(); }

		T_* Ptr() { return &_obj; }
		const T_* Ptr() const { return &_obj; }
		T_& Ref() { return _obj; }
		const T_& Ref() const { return _obj; }

		T_& operator* () { return Ref(); }
		const T_& operator* () const { return Ref(); }

		T_* operator -> () { return Ptr(); }
		const T_* operator -> () const { return Ptr(); }
	};


	template < typename T_ >
	class StorageArray
	{
	private:
		Storage<T_>*		_arr;
		int64_t			_size;

	public:
		StorageArray(int64_t size)
			: _size(size)
		{ _arr = new Storage<T_>[size]; }

		~StorageArray()
		{ delete[] _arr; }


		void Construct()
		{
			for (int64_t i = 0; i < _size; ++i)
				_arr[i].Construct();
		}

		template < typename FunctorType_ >
		void Construct(const FunctorType_& f)
		{
			for (int64_t i = 0; i < _size; ++i)
				_arr[i].Construct(f());
		}

		void Destruct()
		{
			for (int64_t i = 0; i < _size; ++i)
				_arr[i].Destruct();
		}


		template < typename FunctorType_ >
		void ForEach(const FunctorType_& f)
		{
			for (int64_t i = 0; i < _size; ++i)
				f(_arr[i].Ref());
		}


		Storage<T_>& operator [] (int64_t i)
		{ return _arr[i]; }

		const Storage<T_>& operator [] (int64_t i) const
		{ return _arr[i]; }
	};

}

#endif
