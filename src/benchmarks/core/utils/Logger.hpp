#ifndef BENCHMARKS_CORE_UTILS_LOGGER_HPP
#define BENCHMARKS_CORE_UTILS_LOGGER_HPP

// Copyright (c) 2016, Dmitry Koplyarov <koplyarov.da@gmail.com>
//
// Permission to use, copy, modify, and/or distribute this software for any purpose with or without fee is hereby granted,
// provided that the above copyright notice and this permission notice appear in all copies.
//
// THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS.
// IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS,
// WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.


#include <iostream>
#include <mutex>
#include <sstream>


namespace benchmarks
{

	enum class LogLevel
	{
		Debug,
		Info,
		Warning,
		Error
	};


	class NamedLogger;

	class Logger
	{
		friend class NamedLogger;

	private:
		static std::mutex       s_mutex;
		static LogLevel			s_logLevel;

	public:
		static void SetLogLevel(LogLevel logLevel);
		static LogLevel GetLogLevel();

	private:
		static void Write(LogLevel logLevel, const std::string& loggerName, std::string str);
	};


	namespace detail
	{
		template < typename T_, typename Enabler_ = std::string >
		struct ObjectLogger
		{ static void Log(std::stringstream& s, T_&& val) { s << val; } };

		template < typename T_ >
		struct ObjectLogger<T_, decltype(std::declval<T_>().ToString())>
		{ static void Log(std::stringstream& s, T_&& val) { s << val.ToString(); } };
	}

	class NamedLogger
	{
	public:
		class LoggerWriter
		{
		private:
			LogLevel            _loggerLogLevel;
			LogLevel            _logLevel;
			const std::string*	_loggerName;
			std::stringstream   _stream;
			bool                _moved;

		public:
			LoggerWriter(LogLevel logLevel, const std::string& loggerName);
			LoggerWriter(LoggerWriter&& other);
			~LoggerWriter();

			template < typename T_ >
			LoggerWriter& operator << (T_&& val)
			{
				if (_loggerLogLevel <= _logLevel)
					detail::ObjectLogger<T_>::Log(_stream, std::forward<T_>(val));
				return *this;
			}
		};

	private:
		std::string		_name;

	public:
		NamedLogger(std::string name)
			: _name(std::move(name))
		{ }

		LoggerWriter Debug() const { return LoggerWriter(LogLevel::Debug, _name); }
		LoggerWriter Info() const { return LoggerWriter(LogLevel::Info, _name); }
		LoggerWriter Warning() const { return LoggerWriter(LogLevel::Warning, _name); }
		LoggerWriter Error() const { return LoggerWriter(LogLevel::Error, _name); }
	};

#define BENCHMARKS_LOGGER(ClassName_) benchmarks::NamedLogger ClassName_::s_logger(#ClassName_)

}

#endif
