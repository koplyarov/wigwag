#ifndef BENCHMARKS_CORE_UTILS_REPORTTEMPLATEPROCESSOR_HPP
#define BENCHMARKS_CORE_UTILS_REPORTTEMPLATEPROCESSOR_HPP

// Copyright (c) 2016, Dmitry Koplyarov <koplyarov.da@gmail.com>
//
// Permission to use, copy, modify, and/or distribute this software for any purpose with or without fee is hereby granted,
// provided that the above copyright notice and this permission notice appear in all copies.
//
// THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS.
// IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS,
// WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.


#include <benchmarks/core/BenchmarkId.hpp>

#include <boost/phoenix/phoenix.hpp>
#include <boost/spirit/include/qi.hpp>


namespace benchmarks
{
	class ReportTemplateProcessor
	{
	private:
		struct MakeBenchmarkId
		{
			template < typename T1_, typename T2_, typename T3_ >
			BenchmarkId operator () (T1_&& p1, T2_&& p2, T3_&& p3) const
			{ return BenchmarkId(std::string(p1.begin(), p1.end()), std::string(p2.begin(), p2.end()), std::string(p3.begin(), p3.end())); }
		};

		template < typename Printer_, typename MacroProcessor_, typename Iterator_ >
		struct ReportGrammar : boost::spirit::qi::grammar<Iterator_>
		{
			boost::spirit::qi::rule<Iterator_>	text, raw_text_block, macro;

			ReportGrammar()
				: ReportGrammar::base_type(text)
			{
				namespace qi = boost::spirit::qi;
				namespace phx = boost::phoenix;

				text				= raw_text_block >> -( macro >> text );
				raw_text_block		= *( !qi::lit('$') >> qi::char_[Printer_()] );
				macro				= qi::lit("${") >> (+qi::alnum >> '.' >> +qi::alnum >> '.' >> +qi::alnum)[phx::bind(MacroProcessor_(), phx::bind(MakeBenchmarkId(), qi::_1, qi::_2, qi::_3))] >> qi::lit("}");
			}
		};

	public:
		template < typename Printer_, typename MacroProcessor_, typename Iterator_ >
		static bool Process(Iterator_ b, Iterator_ e)
		{
			ReportGrammar<Printer_, MacroProcessor_, Iterator_> g;
			return boost::spirit::qi::parse(b, e, g) && b == e;
		}
	};

}


#endif
