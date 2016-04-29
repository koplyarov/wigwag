#ifndef BENCHMARKS_CORE_IPC_MESSAGEQUEUE_HPP
#define BENCHMARKS_CORE_IPC_MESSAGEQUEUE_HPP

// Copyright (c) 2016, Dmitry Koplyarov <koplyarov.da@gmail.com>
//
// Permission to use, copy, modify, and/or distribute this software for any purpose with or without fee is hereby granted,
// provided that the above copyright notice and this permission notice appear in all copies.
//
// THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS.
// IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS,
// WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.


#include <benchmarks/core/ipc/Messages.hpp>

#include <boost/interprocess/ipc/message_queue.hpp>

#include <memory>


namespace benchmarks
{

	struct RemoteException : public std::runtime_error
	{ RemoteException(const std::string& msg) : std::runtime_error(msg) { } };


	class MessageQueue
	{
	private:
		boost::interprocess::message_queue	_queue;

	public:
		MessageQueue(const std::string& name);

		template < typename MessageType >
		std::shared_ptr<MessageType> ReceiveMessage()
		{
			auto m = ReceiveMessageBase();

			auto result = std::dynamic_pointer_cast<MessageType>(m);
			if (result)
				return result;

			auto ex = dynamic_cast<ExceptionMessage*>(m.get());
			if (ex)
				throw RemoteException(ex->GetMessage());

			throw std::runtime_error("Unexpected message type!");
		}

		void SendMessage(const std::shared_ptr<MessageBase>& message);

		static void Remove(const std::string& name);

	private:
		std::shared_ptr<MessageBase> ReceiveMessageBase();
	};

}


#endif
