static_assert( __cplusplus > 2020'00 );

#pragma once

#include <Alepha/Alepha.h>

#include <ios>
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
		
		void iosCallback( const std::ios_base::event event, std::ios_base &ios, const int idx );
	}

	struct exports::StackableStreambuf
		: virtual public std::streambuf
	{
		public:
			std::streambuf *underlying;

			~StackableStreambuf() {}

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

	void
	impl::releaseStack( std::ios_base &ios )
	{
		auto &os= dynamic_cast< std::ostream & >( ios );
		auto *streambuf= os.rdbuf();

		while( auto *stacked= dynamic_cast< StackableStreambuf * >( streambuf ) )
		{
			streambuf= stacked->underlying;
			stacked->drain();
			os.rdbuf( stacked->underlying );
			delete stacked;
		}
	}

	inline void
	impl::iosCallback( const std::ios_base::event event, std::ios_base &ios, const int idx )
	{
		if( index != idx ) throw std::logic_error{ "Wrong index." };

		if( event == std::ios_base::erase_event ) releaseStack( ios );
	}

	template< typename T >
	struct exports::PushStack : T {};

	template< typename T >
	struct exports::PopStack {};

	template< typename T >
	std::ostream &
	operator << ( std::ostream &os, PushStack< T > &&params )
	{
		build_streambuf( os, params );
		return os;
	}

	template< typename T >
	std::ostream &
	operator << ( std::ostream &os, PopStack< T > )
	{
		releaseStack( os );
		return os;
	}

}
