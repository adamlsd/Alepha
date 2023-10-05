static_assert( __cplusplus > 2020'00 );

#pragma once

#include <iostream>

#include <boost/noncopyable.hpp>

#include "Data.h"

namespace Alepha::inline Cavorite  ::detail::  blob_stream
{
	inline namespace exports
	{
		class BlobStreamBuf;
		class BlobStream;
	}

	class exports::BlobStreamBuf
		: public std::streambuf
	{
		private:
			const std::size_t bufferSize;
			std::size_t used= 0;
			Blob datum;

		public:
			explicit
			BlobStreamBuf( const std::size_t bufferSize )
				: bufferSize( bufferSize )
			{
				datum.setSize( 0 );
			}

			std::streamsize
			xsputn( const char *const data, const std::streamsize amt_ ) override
			{
				if( amt_ < 0 ) throw std::logic_error( "It should be impossible for `streamsize` to be negative in `xsputn`." );
				const std::size_t amt= amt_;
				const Buffer< Const > buffer{ data, amt };

				datum.combine( data, bufferSize );
				return amt;
			}

			int
			overflow( const int ch_ ) override
			{
				if( ch_ == EOF ) throw std::logic_error( "EOF detected in a data stream object." );
				const char ch= ch_;

				return xsputn( &ch, 1 );
			}

			Blob peel() { return datum.carveHead( datum.size() ); }
	};

	class exports::BlobStream
		: public std::ostream, boost::noncopyable
	{
		private:
			BlobStreamBuf buffer;

		public:
			explicit
			BlobStream( const std::size_t bufferSize )
				: buffer( bufferSize )
			{
				this->rdbuf( &buffer );
			}

			Blob peel() { return buffer.peel(); }
	};
}

namespace Alepha::Cavorite::inline exports::inline blob_stream
{
	using namespace detail::blob_stream::exports;
}
