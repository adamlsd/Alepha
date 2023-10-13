static_assert( __cplusplus > 2020'00 );

#pragma once

#include <cstddef>

#include <string>
#include <streambuf>

namespace Alepha::inline Cavorite  ::detail::  word_wrap
{
	inline namespace exports
	{
		std::string wordWrap( const std::string &text, std::size_t width, std::size_t nextLineOffset= 0 );

		class WordWrapStreambuf;
	}

	class exports::WordWrapStreambuf
		: public std::streambuf
	{
		public:
			std::streambuf *underlying= nullptr;

			std::size_t maximumWidth= 0;
			std::size_t nextLineOffset= 0;
			std::size_t currentLineLength= 0;

			std::string currentWord;

			void writeChar( const char ch );

			void drain();

		public:
			int
			overflow( const int ch ) override
			{
				if( ch == EOF ) throw std::logic_error( "EOF!" );
				writeChar( ch );

				return 1;
			}

			std::streamsize
			xsputn( const char *const data, const std::streamsize amt ) override
			{
				for( std::streamsize i= 0; i< amt; ++i ) overflow( data[ i ] );
				return amt;
			}
	};
}

namespace Alepha::Cavorite::inline exports::inline word_wrap
{
	using namespace detail::word_wrap::exports;
}
