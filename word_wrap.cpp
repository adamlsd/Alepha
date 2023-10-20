static_assert( __cplusplus > 2020 );

#include "word_wrap.h"

#include <cassert>

#include <tuple>
#include <iostream>
#include <iterator>
#include <sstream>
#include <memory>

#include <Alepha/Utility/evaluation_helpers.h>

namespace Alepha::Cavorite  ::detail::  word_wrap
{
	namespace
	{
		using namespace Utility::exports::evaluation_helpers;

		// Returns the number of chars in the line just written to.
		std::size_t
		applyWordToLine( const std::size_t maximumWidth, const std::size_t nextLineOffset, const std::size_t currentLineWidth, std::string &&word, std::ostream &result )
		{
			if( word.empty() ) return currentLineWidth;

			const auto lineWidth= evaluate <=[&]
			{
				if( currentLineWidth + word.size() > maximumWidth )
				{
					result << "\n";
					std::fill_n( std::ostream_iterator< char >( result ), nextLineOffset, ' ' );
					return nextLineOffset;
				}
				else return currentLineWidth;
			};

			const auto rv= lineWidth + word.size();
			result << std::move( word );
			return rv;
		}

		struct WordWrapStreambuf
			: public Utility::StackableStreambuf
		{
			public:
				std::size_t maximumWidth= 0;
				std::size_t nextLineOffset= 0;
				std::size_t currentLineLength= 0;

				std::string currentWord;

				explicit
				WordWrapStreambuf( std::ostream &os, const std::size_t width, const std::size_t offset )
					: StackableStreambuf( os ), maximumWidth( width ), nextLineOffset( offset )
				{}

				void writeChar( const char ch ) override;

				void drain() override;
		};
	}

	void
	WordWrapStreambuf::writeChar( const char ch )
	{
		std::ostream outWrap{ underlying };
		if( ch == '\n' )
		{
			const auto prev= currentLineLength;
			const auto size= currentWord.size();
			currentLineLength= applyWordToLine( maximumWidth, nextLineOffset, currentLineLength, std::move( currentWord ), outWrap );
			currentWord.clear();
			outWrap << '\n';
			if( currentLineLength == prev + size )
			{
				std::fill_n( std::ostream_iterator< char >{ outWrap }, nextLineOffset, ' ' );
				currentLineLength= nextLineOffset;
			}
			else currentLineLength= 0;
		}
		else if( ch == ' ' )
		{
			currentLineLength= applyWordToLine( maximumWidth, nextLineOffset, currentLineLength, std::move( currentWord ), outWrap );
			currentWord.clear();
			if( currentLineLength < maximumWidth )
			{
				outWrap << ' ';
				++currentLineLength;
			}
		}
		else currentWord+= ch;
	}

	void
	WordWrapStreambuf::drain()
	{
		if( currentWord.empty() ) return;
		std::ostream outWrap{ underlying };
		applyWordToLine( maximumWidth, nextLineOffset, currentLineLength, std::move( currentWord ), outWrap );
	}

	std::string
	exports::wordWrap( const std::string &text, const std::size_t width, const std::size_t nextLineOffset )
	{
		std::ostringstream oss;

		oss << StartWrap{ width, nextLineOffset };
		oss << text;
		oss << EndWrap;

		auto rv= std::move( oss ).str();
		return rv;
	}

	void
	impl::build_streambuf( std::ostream &os, StartWrap &&args )
	{
		new WordWrapStreambuf( os, args.width, args.nextLineOffset );
	}
}
