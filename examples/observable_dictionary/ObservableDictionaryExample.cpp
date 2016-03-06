// Copyright (c) 2016, Dmitry Koplyarov <koplyarov.da@gmail.com>
//
// Permission to use, copy, modify, and/or distribute this software for any purpose with or without fee is hereby granted,
// provided that the above copyright notice and this permission notice appear in all copies.
//
// THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS.
// IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS,
// WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.


#include "../common/Logger.hpp"
#include "SortedObservableDictionary.hpp"

#include <wigwag/token_pool.hpp>

#include <random>


class DictListener : public virtual IObservableDictionaryListener<int, std::string>
{
public:
	virtual void OnItemAdded(const int& k, const std::string& v)
	{
		Logger::Log(LogLevel::Info) << "DictListener::OnItemAdded(" << k << ", " << v << ")";
	}

	virtual void OnItemRemoved(const int& k, const std::string& v)
	{
		Logger::Log(LogLevel::Info) << "DictListener::OnItemRemoved(" << k << ", " << v << ")";
	}
};


class Random
{
	std::random_device                       _rd;
	std::mt19937                             _mt;
	std::uniform_real_distribution<double>   _dist;

public:
	Random()
		: _mt(_rd()), _dist(1.0, 10.0)
	{ }

	std::string GenerateString(size_t size)
	{
		std::string s;
		for (size_t i = 0; i < size; ++i)
			s += 'a' + (int)_dist(_mt);
		return s;
	}
};


int main()
{
	using namespace std::this_thread;
	using namespace std::chrono;

	try
	{
		IObservableDictionaryPtr<int, std::string> dict = std::make_shared<SortedObservableDictionary<int, std::string>>();
		dict->Set(101, "qwe");
		dict->Set(102, "asd");

		wigwag::token_pool tp;
		tp += dict->AddListener(std::make_shared<DictListener>());
		tp += dict->OnChanged().connect([](CollectionOp op, int k, const std::string& v)
			{
				std::string op_str;
				switch (op)
				{
				case CollectionOp::ItemAdded:
					op_str = "ItemAdded";
					break;
				case CollectionOp::ItemRemoved:
					op_str = "ItemRemoved";
					break;
				default:
					op_str = "Unknown";
					break;
				}
				Logger::Log(LogLevel::Info) << "SignalHandler(" << op_str << ", " << k << ", " << v << ")";
			});

		Random r;

		std::atomic<bool> alive(true);
		std::thread t([&]()
			{
				int size = 5;
				int i = 0;
				while (alive)
				{
					sleep_for(seconds(1));
					Logger::Log(LogLevel::Info) << "=============";
					dict->Set(i, r.GenerateString(6));
					i = (i + 1) % size;
				}
			});

		sleep_for(seconds(10));
		alive = false;
		t.join();
	}
	catch (const std::exception& ex)
	{
		Logger::Log(LogLevel::Error) << "Uncaught exception: " << ex.what();
	}
}
