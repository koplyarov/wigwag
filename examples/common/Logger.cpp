#include "Logger.hpp"



std::mutex Logger::s_mutex;

void Logger::Write(LogLevel logLevel, std::string str)
{
	std::lock_guard<std::mutex> l(s_mutex);
	switch (logLevel)
	{
	case LogLevel::Info:
		std::cerr << "[Info] ";
		break;
	case LogLevel::Error:
		std::cerr << "[Error] ";
		break;
	default:
		std::cerr << "[LogLevel: " << static_cast<std::underlying_type<LogLevel>::type>(logLevel) << "] ";
		break;
	}
	std::cerr << str << std::endl;
}
