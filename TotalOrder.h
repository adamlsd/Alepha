static_assert( __cplusplus > 2020'00 );

#pragma once

#include <compare>

#include "Concepts.h"

namespace Alepha::inline Cavorite  ::detail::  total_order
{
	inline namespace exports
	{
		using TotalOrder= std::strong_ordering;

		template< typename T >
		concept TotalOrderable=
		requires( const T &lhs, const T &rhs )
		{
			{ lhs <=> rhs } -> SameAs< TotalOrder >;
		};
	}
}

namespace Alepha::Cavorite::inline exports::inline total_order
{
	using namespace detail::total_order::exports;
}
