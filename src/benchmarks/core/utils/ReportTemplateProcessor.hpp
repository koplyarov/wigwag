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


#include <benchmarks/core/detail/MeasurementId.hpp>

#include <boost/phoenix/phoenix.hpp>
#include <boost/spirit/include/qi.hpp>


namespace benchmarks
{
	class ReportTemplateProcessor
	{
	private:
		template < typename Iterator_, typename Printer_, typename MacroProcessor_ >
		struct ReportGrammar : boost::spirit::qi::grammar<Iterator_>
		{
			Printer_											_printer;
			MacroProcessor_										_macroProcessor;
			boost::spirit::qi::rule<Iterator_>					_text, _rawTextBlock, _macro;
			boost::spirit::qi::rule<Iterator_, MeasurementId()>	_measurementId;
			boost::spirit::qi::rule<Iterator_, BenchmarkId()>	_benchmarkId;
			boost::spirit::qi::rule<Iterator_, std::string()>	_measurementLocalId;
			boost::spirit::qi::rule<Iterator_, std::string()>	_identifier;

			ReportGrammar(const Printer_& printer, const MacroProcessor_& macroProcessor)
				: ReportGrammar::base_type(_text), _printer(printer), _macroProcessor(macroProcessor)
			{
				using namespace boost::spirit::qi;
				using namespace boost::phoenix;

				_text					= _rawTextBlock >> -( _macro >> _text );
				_rawTextBlock			= *( !lit('$') >> char_[_printer] );

				_macro					= lit("${") >> (_measurementId >> -(lit('-') >> _measurementId))[ bind(_macroProcessor, _1, _2) ] >> lit('}');

				_measurementId			= (_benchmarkId >> lit("[") >> _measurementLocalId >> lit(']'))[ _val = construct<MeasurementId>(_1, _2) ];
				_benchmarkId			= (_identifier >> '.' >> _identifier >> '.' >> _identifier)[ _val = construct<BenchmarkId>(_1, _2, _3) ];
				_measurementLocalId		= _identifier[ _val = _1 ];

				_identifier			= as_string[(alpha | char_('_')) >> *(alnum | char_('_'))];
			}
		};

	public:
		template < typename Iterator_, typename Printer_, typename MacroProcessor_ >
		static void Process(Iterator_ b, Iterator_ e, const Printer_& printer, const MacroProcessor_& macroProcessor)
		{
			ReportGrammar<Iterator_, Printer_, MacroProcessor_> g(printer, macroProcessor);
			if (!boost::spirit::qi::parse(b, e, g) || b != e)
				throw std::runtime_error("Could not parse report template!");
		}
	};

}


#endif
