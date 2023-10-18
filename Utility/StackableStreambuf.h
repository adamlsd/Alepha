static_assert( __cplusplus > 2020'00 );

#pragma once

#include <Alepha/Alepha.h>

#include <ostream>
#include <exception>
#include <stdexcept>

namespace Alepha::Hydrogen::Utility  ::detail::  stackable_streambuf
{
	inline namespace exports
	{
		struct StackableStreambuf;

		template< typename > struct PushStack;

		template< typename > struct PopStack;
	}

	inline const auto index= std::ios::xalloc();

	inline namespace impl
	{
		void releaseStack( std::ios_base &ios );
		bool releaseTop( std::ostream &os );
		
		void iosCallback( const std::ios_base::event event, std::ios_base &ios, const int idx );
	}

	struct exports::StackableStreambuf
		: virtual public std::streambuf
	{
		public:
			std::streambuf *underlying;

			~StackableStreambuf() {}

			// Children must be created by `new`.
			explicit
			StackableStreambuf( std::ostream &host )
				: underlying( host.rdbuf( this ) )
			{
				// TODO: Atomicity for this:
				if( not host.iword( index ) ) host.register_callback( iosCallback, index );
				host.iword( index )= 1;
			}

			std::ostream out() const { return std::ostream{ underlying }; }

			virtual void writeChar( const char ch )= 0;
			virtual void drain()= 0;

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

	inline bool
	impl::releaseTop( std::ostream &os )
	{
		auto *const streambuf= os.rdbuf();
		if( not streambuf ) return false;
		auto *const stacked= dynamic_cast< StackableStreambuf * >( streambuf );
		if( not stacked ) return false;

		stacked->drain();
		os.rdbuf( stacked->underlying );
		delete stacked;
		return true;
	}

	inline void
	impl::releaseStack( std::ios_base &ios )
	{
		auto &os= dynamic_cast< std::ostream & >( ios );

		while( releaseTop( os ) );
	}

	inline void
	impl::iosCallback( const std::ios_base::event event, std::ios_base &ios, const int idx )
	{
		if( index != idx ) throw std::logic_error{ "Wrong index." };

		if( event == std::ios_base::erase_event ) releaseStack( ios );
	}

	template< typename T >
	struct exports::PushStack
		: T
	{
		using T::T;
	};

	template< typename T >
	struct exports::PopStack {};

	template< typename T >
	std::ostream &
	operator << ( std::ostream &os, PushStack< T > &&params )
	{
		build_streambuf( os, std::move( params ) );
		return os;
	}

	template< typename T >
	std::ostream &
	operator << ( std::ostream &os, PopStack< T > )
	{
		if( not releaseTop( os ) ) throw std::logic_error( "OStream has no stacked streambufs!" );
		return os;
	}

}

namespace Alepha::Hydrogen::Utility::inline exports::inline stackable_streambuf
{
	using namespace detail::stackable_streambuf::exports;
}
