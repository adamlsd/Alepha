static_assert( __cplusplus > 2020'00 );

#pragma once

#include <algorithm>
#include <iterator>

namespace Alepha::inline Cavorite  ::detail::  container_algorithms
{
	inline namespace exports
	{
		template< typename Container, typename Value >
		constexpr bool
		containsLinearSearch( const Container &container, const Value &value )
		{
			return std::find( begin( container ), end( container ), value ) != end( container );
		}

		/*!
		 * @pre Elements in `container` must be in sorted order.
		 */
		template< typename Container, typename Value >
		constexpr bool
		containsBinarySearch( const Container &container, const Value &value )
		{
			return std::binary_search( begin( container ), end( container ), value );
		}
	}
}

namespace Alepha::Cavorite::inline exports::inline container_algorithms
{
	using namespace detail::container_algorithms::exports;
}
