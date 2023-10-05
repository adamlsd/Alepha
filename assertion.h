static_assert( __cplusplus > 2020'00 );

#pragma once

#include <cstdlib>

#include <exception>

namespace Alepha::inline Cavorite  ::detail::  assertion_function
{
	inline namespace exports {}

	namespace C
	{
		constexpr bool ndebug= false
		#ifdef NDEBUG
		or true
		#endif
		;

		constexpr bool debugMode= false or not ndebug;
	}

	// A non-macro variation on assertions.

	namespace exports
	{
		inline void
		assertion( const bool b )
		{
			if constexpr( C::debugMode )
			{
				if( not b ) std::abort();
			}

			(void) b;
		}
	}
}

namespace Alepha::Cavorite::inline exports::inline assertion_function
{
	using namespace detail::exports::assertion_function;
}
