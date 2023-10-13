static_assert( __cplusplus > 2020 );

#include "word_wrap.h"

#include <cassert>

#include <tuple>
#include <iostream>
#include <iterator>
#include <sstream>

#include "evaluation_helpers.h"

namespace Alepha::Cavorite  ::detail::  word_wrap
{
	namespace
	{
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
		#if NEVER
		auto putWord= [width, nextLineOffset]( std::string &&word, std::string &line, const std::size_t lineLength )
		{
			std::ostringstream out;
			const auto rv= applyWordToLine( width, nextLineOffset, lineLength, std::move( word ), out );
			line+= std::move( out ).str();
			return rv;
		};

		std::string result;
		std::string word;
		std::size_t lineLength= 0;
		for( const char ch: text )
		{
			if( ch == '\n' )
			{
				const auto prev= lineLength;
				const auto size= word.size();
				lineLength= putWord( std::move( word ), result, lineLength );
				word.clear();
				result+= '\n';
				if( lineLength == prev + size )
				{
					std::fill_n( back_inserter( result ), nextLineOffset, ' ' );
					lineLength= nextLineOffset;
				}
				else lineLength= 0;
			}
			else if( ch == ' ' )
			{
				lineLength= putWord( std::move( word ), result, lineLength );
				word.clear();
				if( lineLength < width )
				{
					result+= ' ';
					lineLength++;
				}
			}
			else word+= ch;
		}
		if( not word.empty() ) std::ignore= putWord( std::move( word ), result, lineLength );
		return result;
		#else
		std::ostringstream oss;

		WordWrapStreambuf buf;
		buf.maximumWidth= width;
		buf.nextLineOffset= nextLineOffset;
		buf.underlying= static_cast< std::ostream & >( oss ).rdbuf();
		static_cast< std::ostream & >( oss ).rdbuf( &buf );

		oss << text;
		buf.drain();

		auto rv= std::move( oss ).str();
		return rv;
		#endif
	}
}
