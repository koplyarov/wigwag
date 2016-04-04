#ifndef SRC_BENCHMARKS_ADAPTERS_SIGCPP_HPP
#define SRC_BENCHMARKS_ADAPTERS_SIGCPP_HPP

namespace sigcpp_adapters
{

	template < typename Connection_ >
	class scoped_connection
	{
	private:
		Connection_		_connection;
		bool			_moved;

	public:
		scoped_connection(Connection_ c)
			: _connection(std::move(c)), _moved(false)
		{ }

		scoped_connection(scoped_connection&& other)
			: _connection(std::move(other._connection)), _moved(false)
		{ other._moved = true; }

		~scoped_connection()
		{
			if (!_moved)
				_connection.disconnect();
		}

		scoped_connection(const scoped_connection&) = delete;
		scoped_connection& operator = (const scoped_connection&) = delete;
	};


}

#endif
