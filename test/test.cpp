// Copyright (c) 2016, Dmitry Koplyarov <koplyarov.da@gmail.com>
//
// Permission to use, copy, modify, and/or distribute this software for any purpose with or without fee is hereby granted,
// provided that the above copyright notice and this permission notice appear in all copies.
//
// THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS.
// IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS,
// WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.


#include <wigwag/signal.hpp>

#include <chrono>
#include <iostream>
#include <thread>


using namespace wigwag;


template < typename Signature_ >
using threadless_signal = basic_signal<Signature_, signal_policies::threading::threadless, signal_policies::handlers_storage::shared_list, signal_policies::life_assurance::none>;


static void signal_handler(int i)
{
	std::cout << "signal_handler: " << i << std::endl;
	std::this_thread::sleep_for(std::chrono::seconds(5));
}


int main()
{
	try
	{
		signal<void(int)> s;
		std::atomic<bool> alive;

		std::thread t(
			[&]()
			{
				int i = 0;
				while (alive)
				{
					s(++i);
					std::this_thread::sleep_for(std::chrono::milliseconds(300));
				}
			}
		);

		{
			token t = s.connect(&signal_handler);
			std::this_thread::sleep_for(std::chrono::seconds(3));
		}

		alive = false;
		t.join();
	}
	catch (const std::exception& ex)
	{
		std::cerr << "Uncaught exception: " << ex.what();
	}

	return 0;
}
