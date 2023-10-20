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

		struct PopStack{};
	}

	struct exports::StackableStreambuf
		: virtual public std::streambuf
	{
		public:
			std::streambuf *underlying;

			~StackableStreambuf() override;

			// Children must be created by `new`.
			explicit StackableStreambuf( std::ostream &host );

			auto out() const { return std::ostream{ underlying }; }

			virtual void writeChar( char ch )= 0;
			virtual void drain()= 0;

			int overflow( const int ch ) override;

			std::streamsize xsputn( const char *data, std::streamsize amt ) override;
	};

	template< typename T >
	struct exports::PushStack
		: T
	{
		using T::T;
	};

	template< typename T >
	std::ostream &
	operator << ( std::ostream &os, PushStack< T > &&params )
	{
		build_streambuf( os, std::move( params ) );
		return os;
	}

	inline namespace impl
	{
		std::ostream & operator << ( std::ostream &os, PopStack );
	}
}

namespace Alepha::Hydrogen::Utility::inline exports::inline stackable_streambuf
{
	using namespace detail::stackable_streambuf::exports;
}
