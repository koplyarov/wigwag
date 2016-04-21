#ifndef BENCHMARKS_CORE_IPC_MESSAGES_HPP
#define BENCHMARKS_CORE_IPC_MESSAGES_HPP

// Copyright (c) 2016, Dmitry Koplyarov <koplyarov.da@gmail.com>
//
// Permission to use, copy, modify, and/or distribute this software for any purpose with or without fee is hereby granted,
// provided that the above copyright notice and this permission notice appear in all copies.
//
// THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS.
// IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS,
// WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.


#include <boost/serialization/base_object.hpp>
#include <boost/serialization/export.hpp>
#include <boost/serialization/map.hpp>
#include <boost/serialization/nvp.hpp>
#include <boost/serialization/serialization.hpp>

#include <map>
#include <string>


namespace benchmarks
{

	class MessageBase
	{
	public:
		virtual ~MessageBase() { }

		template<class Archive>
		void serialize(Archive &ar, const unsigned int version)
		{ }
	};


	class IterationsCountMessage : public MessageBase
	{
	private:
		int		_count;

	public:
		explicit IterationsCountMessage(int count = 0)
			: _count(count)
		{ }

		int GetCount() const { return _count; }

		template<class Archive>
		void serialize(Archive &ar, const unsigned int version)
		{ ar & BOOST_SERIALIZATION_BASE_OBJECT_NVP(benchmarks::MessageBase) & BOOST_SERIALIZATION_NVP(_count); }
	};


	class ResultsMessage : public MessageBase
	{
		using OperationTimesMap = std::map<std::string, double>	;
		using MemoryConsumptionMap = std::map<std::string, int64_t>;

	private:
		OperationTimesMap		_operationTimes;
		MemoryConsumptionMap	_memoryConsumption;

	public:
		ResultsMessage()
		{ }

		ResultsMessage(OperationTimesMap operationTimes, MemoryConsumptionMap memoryConsumption)
			: _operationTimes(std::move(operationTimes)), _memoryConsumption(std::move(memoryConsumption))
		{ }

		const OperationTimesMap& GetOperationTimes() const { return _operationTimes; }
		const MemoryConsumptionMap& GetMemoryConsumption() const { return _memoryConsumption; }

		template<class Archive>
		void serialize(Archive &ar, const unsigned int version)
		{ ar &BOOST_SERIALIZATION_BASE_OBJECT_NVP(benchmarks::MessageBase) & BOOST_SERIALIZATION_NVP(_operationTimes) & BOOST_SERIALIZATION_NVP(_memoryConsumption); }
	};


	class ExceptionMessage : public MessageBase
	{
	private:
		std::string		_message;

	public:
		explicit ExceptionMessage(std::string message = "")
			: _message(std::move(message))
		{ }

		std::string GetMessage() const { return _message; }

		template<class Archive>
		void serialize(Archive &ar, const unsigned int version)
		{ ar & BOOST_SERIALIZATION_BASE_OBJECT_NVP(benchmarks::MessageBase) & BOOST_SERIALIZATION_NVP(_message); }
	};


}


BOOST_CLASS_EXPORT_KEY(benchmarks::MessageBase);
BOOST_CLASS_EXPORT_KEY(benchmarks::IterationsCountMessage);
BOOST_CLASS_EXPORT_KEY(benchmarks::ResultsMessage);
BOOST_CLASS_EXPORT_KEY(benchmarks::ExceptionMessage);

#endif
