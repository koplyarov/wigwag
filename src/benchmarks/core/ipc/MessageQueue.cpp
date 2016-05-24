// Copyright (c) 2016, Dmitry Koplyarov <koplyarov.da@gmail.com>
//
// Permission to use, copy, modify, and/or distribute this software for any purpose with or without fee is hereby granted,
// provided that the above copyright notice and this permission notice appear in all copies.
//
// THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS.
// IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS,
// WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.


#include <benchmarks/core/ipc/MessageQueue.hpp>

#include <boost/archive/text_iarchive.hpp>
#include <boost/archive/text_oarchive.hpp>
#include <boost/serialization/serialization.hpp>

#include <sstream>


BOOST_CLASS_EXPORT_IMPLEMENT(benchmarks::MessageBase);
BOOST_CLASS_EXPORT_IMPLEMENT(benchmarks::IterationsCountMessage);
BOOST_CLASS_EXPORT_IMPLEMENT(benchmarks::BenchmarkResultMessage);
BOOST_CLASS_EXPORT_IMPLEMENT(benchmarks::ExceptionMessage);
namespace benchmarks
{

	using namespace boost::interprocess;


	std::shared_ptr<MessageBase> MessageQueue::ReceiveMessageBase()
	{
		std::array<char, MaxMessageSize> buf;

		message_queue::size_type msg_size = 0;
		unsigned int priority = 0;
		_queue.receive(buf.data(), buf.size(), msg_size, priority);

		std::stringstream s(std::string(buf.begin(), buf.begin() + msg_size));
		boost::archive::text_iarchive ar(s);

		MessageBase* result = nullptr;
		ar >> result;

		return std::shared_ptr<MessageBase>(result);
	}


	void MessageQueue::SendMessage(const std::shared_ptr<MessageBase>& message)
	{
		std::stringstream s;
		boost::archive::text_oarchive ar(s);
		MessageBase* message_ptr = message.get();
		ar << BOOST_SERIALIZATION_NVP(message_ptr);

		std::string str(s.str());

		_queue.send(str.data(), str.size(), 0);
	}


	void MessageQueue::Remove(const std::string& name)
	{ message_queue::remove(name.c_str()); }

}
