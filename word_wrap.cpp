static_assert( __cplusplus > 2020 );

#include "word_wrap.h"

namespace Alepha::Cavorite  ::detail::  word_wrap
{
	namespace
	{
		// Returns the number of chars in the line just written to.
		std::size_t
		applyWordToLine( const std::size_t maximumWidth, const std::size_t nextLineOffset, const std::size_t currentLineWidth, std::string &&word, std::string &result )
		{
			if( word.empty() ) return currentLineWidth;

			const auto lineWidth= evaluate <=[&]
			{
				if( currentLineWidth + word.size() > maximumWidth )
				{
					result+= '\n';
					std::fill_n( back_inserter( result ), ' ', nextLineOffset );
					return nextLineOffset;
				}
				else return currentLineWidth;
			};

			const auto rv= lineWidth + word.size();
			result+= std::move( word );
			return rv;
		}
	}

	std::string
	exports::wordWrap( const std::string &text, const std::size_t width, const std::size_t nextLineOffset )
	{
		auto putWord= [[nodiscard]] [width, nextLineOffset]( std::string &&word, std::string &line, const std::size_t lineLength )
		{
			return applyWordToLine( width, nextLineOffset, line.size(), std::move( word ), line );
		};

		std::string result;
		std::string word;
		std::size_t lineLength= 0;
		for( const char ch: text )
		{
			if( ch == '\n' )
			{
				std::ignore= putWord( std::move( word ), result, lineLength );
				word.clear();
				lineLength= 0;
			}
			else if( ch == ' ' )
			{
				lineLength= putWord( std::move( word ), result, lineLength );
				word.clear();
				if( lineLength < width )
				{
					line+= ' ';
					lineLength++;
				}
			}
			else word+= ch;
		}
		if( not word.empty() ) std::ignore= putWord( std::move( word ), result, lineLength );
		return result;
	}
}
