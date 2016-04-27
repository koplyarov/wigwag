#ifndef SRC_BENCHMARKS_BENCHMARK_HPP
#define SRC_BENCHMARKS_BENCHMARK_HPP

// Copyright (c) 2016, Dmitry Koplyarov <koplyarov.da@gmail.com>
//
// Permission to use, copy, modify, and/or distribute this software for any purpose with or without fee is hereby granted,
// provided that the above copyright notice and this permission notice appear in all copies.
//
// THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS.
// IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS,
// WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.


#include <functional>
#include <map>
#include <sstream>
#include <string>
#include <vector>

#include <benchmarks/core/BenchmarkContext.hpp>


namespace benchmarks
{

	using SerializedParam = std::string;
	using SerializedParamsMap = std::map<std::string, SerializedParam>;
	using SerializedParamsPair = std::pair<std::string, SerializedParam>;

	namespace detail
	{
		template < typename T_ >
		struct BenchmarkParamDeserializer
		{
			static T_ Deserialize(const SerializedParam& serialized)
			{
				std::stringstream s(serialized);
				T_ result;
				s >> result;
				if (s.bad())
					throw std::runtime_error("Could not deserialize a parameter!");
				return result;
			}
		};

		template < typename... BenchmarkParams_ >
		struct BenchmarkInvoker;

		template < typename BenchmarkParamsHead_, typename... BenchmarkParamsTail_>
		struct BenchmarkInvoker<BenchmarkParamsHead_, BenchmarkParamsTail_...>
		{
			template < typename ParamNamesIter_, typename SerializedParamsMap_, typename Functor_, typename... ParamTypes_ >
			static void Invoke(ParamNamesIter_ paramNamesBegin, ParamNamesIter_ paramNamesEnd, const SerializedParamsMap_& serializedParams, const Functor_& f, ParamTypes_&&... params)
			{
				if (paramNamesBegin == paramNamesEnd)
					throw std::runtime_error("Parameters count mismatch!");

				auto param_name = *paramNamesBegin;
				auto it = serializedParams.find(param_name);
				if (it == serializedParams.end())
					throw std::invalid_argument("No argument named " + param_name + " provided!");

				BenchmarkParamsHead_ p = BenchmarkParamDeserializer<BenchmarkParamsHead_>::Deserialize(it->second);

				BenchmarkInvoker<BenchmarkParamsTail_...>::Invoke(std::next(paramNamesBegin), paramNamesEnd, serializedParams, f, std::forward<ParamTypes_>(params)..., p);
			}
		};

		template < >
		struct BenchmarkInvoker<>
		{
			template < typename ParamNamesIter_, typename Functor_, typename SerializedParamsMap_, typename... ParamTypes_ >
			static void Invoke(ParamNamesIter_ paramNamesBegin, ParamNamesIter_ paramNamesEnd, const SerializedParamsMap_& serializedParams, const Functor_& f, ParamTypes_&&... params)
			{
				if (paramNamesBegin != paramNamesEnd)
					throw std::runtime_error("Parameters count mismatch!");

				f(std::forward<ParamTypes_>(params)...);
			}
		};
	}


	struct IBenchmark
	{
		virtual ~IBenchmark() { }

		virtual std::string GetName() const = 0;
		virtual void Perform(BenchmarkContext& context, const SerializedParamsMap& serializedParams) const = 0;
	};
	using IBenchmarkPtr = std::shared_ptr<IBenchmark>;


	template < typename... BenchmarkParams_ >
	class Benchmark : public IBenchmark
	{
		using BenchmarkFunction = std::function<void(BenchmarkContext&, BenchmarkParams_...)>;

	private:
		std::string					_name;
		BenchmarkFunction			_benchmarkFunc;
		std::vector<std::string>	_orderedParamNames;

	public:
		Benchmark(std::string name, BenchmarkFunction benchmarkFunc, std::vector<std::string> orderedParamNames)
			: _name(std::move(name)), _benchmarkFunc(std::move(benchmarkFunc)), _orderedParamNames(std::move(orderedParamNames))
		{
			if (_orderedParamNames.size() != sizeof...(BenchmarkParams_))
				throw std::runtime_error("Parameters count mismatch: expected " + std::to_string(sizeof...(BenchmarkParams_)) + ", got " + std::to_string(_orderedParamNames.size()));
		}

		std::string GetName() const
		{ return _name; }

		void Perform(BenchmarkContext& context, const SerializedParamsMap& serializedParams) const
		{
			if (serializedParams.size() != _orderedParamNames.size())
				throw std::runtime_error("Parameters count mismatch!");

			detail::BenchmarkInvoker<BenchmarkParams_...>::Invoke(_orderedParamNames.cbegin(), _orderedParamNames.cend(), serializedParams, _benchmarkFunc, context);
		}
	};

}

#endif
