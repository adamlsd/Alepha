static_assert( __cplusplus > 2020 );

#include "word_wrap.h"

#include <cassert>

#include <tuple>
#include <iostream>
#include <iterator>
#include <sstream>
#include <memory>

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

		struct WordWrapStreambuf
			: public std::streambuf
		{
			public:
				std::unique_ptr< std::streambuf > ownership;
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

	namespace
	{
		const auto wrapperIndex= std::ios_base::xalloc();

		void
		releaseWrapper( std::ios_base &ios )
		{
			auto *const streambuf= static_cast< WordWrapStreambuf * >( ios.pword( wrapperIndex ) );
			streambuf->drain();
			delete streambuf;
			ios.pword( wrapperIndex )= nullptr;
		}

		void
		wordwrapCallback( const std::ios_base::event event, std::ios_base &ios, const int idx ) noexcept
		{
			#pragma GCC diagnostic push
			#pragma GCC diagnostic ignored "-Wterminate"

			if( wrapperIndex != idx ) throw std::logic_error( "Must only work with the word wrap index." );

			if( not ios.pword( wrapperIndex ) ) return;

			if( event == std::ios_base::erase_event ) releaseWrapper( ios );
			else if( event == std::ios_base::imbue_event ) {}
			else if( event == std::ios_base::copyfmt_event ) throw std::runtime_error{ "Can't copy?" };

			#pragma GCC diagnostic pop
		}
	}

	std::ostream &
	impl::operator << ( std::ostream &os, EndWrap_t )
	{
		releaseWrapper( os );
		return os;
	}

	std::ostream &
	impl::operator << ( std::ostream &os, StartWrap args )
	{
		auto buf= std::make_unique< WordWrapStreambuf >();
		buf->maximumWidth= args.width;
		buf->nextLineOffset= args.nextLineOffset;
		buf->underlying= os.rdbuf( buf.get() );
		auto &state= os.iword( wrapperIndex );
		if( not state )
		{
			state= 1;
			os.register_callback( wordwrapCallback, wrapperIndex );
		}
		os.pword( wrapperIndex )= buf.release();

		return os;
	}
}
