#ifndef EXAMPLES_COMMON_LOGGER_HPP
#define EXAMPLES_COMMON_LOGGER_HPP

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


enum class LogLevel
{
    Info,
    Error
};


class Logger
{
public:
    class LoggerWriter
    {
    private:
        LogLevel            _logLevel;
        std::stringstream   _stream;
        bool                _moved;

    public:
        LoggerWriter(LogLevel logLevel)
            : _logLevel(logLevel), _moved(false)
        { }

        LoggerWriter(LoggerWriter&& other)
            : _logLevel(other._logLevel), _moved(false)
        {
            _stream << other._stream.str();
            other._moved = true;
        }

        ~LoggerWriter()
        {
            if (!_moved)
                Logger::Write(_logLevel, _stream.str());
        }

        template < typename T_ >
        LoggerWriter& operator << (T_&& val)
        {
            _stream << val;
            return *this;
        }
    };

private:
    static std::mutex       s_mutex;

public:
    static LoggerWriter Log(LogLevel logLevel)
    { return LoggerWriter(logLevel); }

private:
    static void Write(LogLevel logLevel, std::string str);
};

#endif
