static_assert( __cplusplus > 2020'00 );

#pragma once

#include <Alepha/Alepha.h>

#include <ostream>
#include <exception>
#include <stdexcept>
#include <memory>
#include <stack>

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
		void iosCallback( const std::ios_base::event event, std::ios_base &ios, const int idx );
	}

	inline auto &
	getStack( std::ios_base &ios )
	{
		auto &ownership= reinterpret_cast< std::stack< std::unique_ptr< StackableStreambuf > > *& >( ios.pword( index ) );
		if( not ownership ) ownership= new std::decay_t< decltype( *ownership ) >{};

		return *ownership;
	}

	struct exports::StackableStreambuf
		: virtual public std::streambuf
	{
		public:
			std::streambuf *underlying;

			~StackableStreambuf() override {}

			// Children must be created by `new`.
			explicit
			StackableStreambuf( std::ostream &host )
				: underlying( host.rdbuf( this ) )
			{
				// TODO: Atomicity for this:
				if( not host.iword( index ) ) host.register_callback( iosCallback, index );
				host.iword( index )= 1;
				getStack( host ).emplace( this );
			}

			auto out() const { return std::ostream{ underlying }; }

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

	enum { token };

	inline bool
	releaseTop( std::ios_base &ios, decltype( token )= token )
	{
		auto &ownership= getStack( ios );

		// Since it's owned, delete happens at scope exit.
		const std::unique_ptr top= std::move( ownership.top() );
		ownership.pop();

		top->drain();
		return not ownership.empty();
	}

	inline bool
	releaseTop( std::ostream &os )
	{
		const auto *const current= dynamic_cast< StackableStreambuf * >( os.rdbuf() );
		if( not current ) return false;

		os.rdbuf( current->underlying );

		releaseTop( os, token );

		return true;
	}

	inline void
	releaseStack( std::ios_base &ios )
	{
		auto &os= dynamic_cast< std::ostream & >( ios );

		while( releaseTop( os ) );
	}

	inline void
	impl::iosCallback( const std::ios_base::event event, std::ios_base &ios, const int idx )
	{
		#pragma GCC diagnostic push
		#pragma GCC diagnostic ignored "-Wterminate"

		if( index != idx ) throw std::logic_error{ "Wrong index." };

		if( event == std::ios_base::erase_event ) releaseStack( ios );
		else if( event == std::ios_base::imbue_event ) {}
		else if( event == std::ios_base::copyfmt_event ) throw std::runtime_error{ "Can't copy?" };

		#pragma GCC diagnostic pop
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
