// Copyright (c) 2016, Dmitry Koplyarov <koplyarov.da@gmail.com>
//
// Permission to use, copy, modify, and/or distribute this software for any purpose with or without fee is hereby granted,
// provided that the above copyright notice and this permission notice appear in all copies.
//
// THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS.
// IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS,
// WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.

#include <benchmarks/core/utils/Logger.hpp>


namespace benchmarks
{

	NamedLogger::LoggerWriter::LoggerWriter(LogLevel logLevel, const std::string& loggerName)
		: _loggerLogLevel(Logger::GetLogLevel()), _logLevel(logLevel), _loggerName(&loggerName), _moved(false)
	{ }

	NamedLogger::LoggerWriter::LoggerWriter(LoggerWriter&& other)
		: _logLevel(other._logLevel), _moved(false)
	{
		_stream << other._stream.str();
		other._moved = true;
	}

	NamedLogger::LoggerWriter::~LoggerWriter()
	{
		if (!_moved && _loggerLogLevel <= _logLevel)
			Logger::Write(_logLevel, *_loggerName, _stream.str());
	}


	////////////////////////////////////////////////////////////////////////////////


	std::mutex Logger::s_mutex;
	LogLevel Logger::s_logLevel = LogLevel::Info;


	void Logger::SetLogLevel(LogLevel logLevel)
	{
		std::lock_guard<std::mutex> l(s_mutex);
		s_logLevel = logLevel;
	}

	LogLevel Logger::GetLogLevel()
	{
		std::lock_guard<std::mutex> l(s_mutex);
		return s_logLevel;
	}


	void Logger::Write(LogLevel logLevel, const std::string& loggerName, std::string str)
	{
		std::lock_guard<std::mutex> l(s_mutex);
		switch (logLevel)
		{
		case LogLevel::Debug:	std::cerr << "[Debug] "; break;
		case LogLevel::Info:	std::cerr << "[Info] "; break;
		case LogLevel::Warning:	std::cerr << "[Warning] "; break;
		case LogLevel::Error:	std::cerr << "[Error] "; break;
		default: std::cerr << "[LogLevel: " << static_cast<std::underlying_type<LogLevel>::type>(logLevel) << "] "; break;
		}
		std::cerr << "[" << loggerName << "] " << str << std::endl;
	}

}
