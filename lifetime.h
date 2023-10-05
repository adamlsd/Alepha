static_assert( __cplusplus > 2020'00 );

#pragma once

#include <cstddef>

#include <type_traits>

namespace Alepha::inline Cavorite  ::detail:: lifetime
{
	inline namespace exports
	{
		// When C++23 arrives, just turn these into `using` statements.

		template< typename T >
		std::add_pointer_t< T >
		start_lifetime_as( void *const mp ) noexcept
		{
			const auto raw= new( mp ) std::byte[ sizeof( T ) ];
			const auto data= reinterpret_cast< std::add_pointer_t< T > >( raw );
			return std::launder( data );
		}

		template< typename T >
		std::add_pointer_t< std::add_const_t< T > >
		start_lifetime_as( const void *const mp ) noexcept
		{
			const auto mp= const_cast< void * >( p );
			return start_lifetime_as< std::add_const_t< T > >( mp );
		}
	}
}

namespace Alepha::Cavorite::inline exports::inline lifetime
{
	using namespace detail::lifetime::exports;
}
