static_assert( __cplusplus > 201700, "C++17 Required" );

#pragma once

#include <Alepha/Alepha.h>

#include <functional>

#include <Alepha/Meta/Container/vector.h>
#include <Alepha/Meta/functional.h>

namespace Alepha::Hydrogen::Meta
{
	inline namespace exports { inline namespace find_algos {} }

	namespace detail::find_algos
	{
		inline namespace exports
		{
			template< typename Iter, typename Predicate >
			constexpr bool
			find_if( const Iter first, const Iter last, Predicate pred )
			{
				for( Iter pos= first; pos != last; ++pos )
				{
					if( pred( *pos ) ) return true;
				}
				return false;
			}

			template< typename Iter, typename Value >
			constexpr bool
			find( const Iter first, const Iter last, const Value value )
			{
				return find_if( first, last, Meta::bind1st( std::equal_to{}, value ) );
			}
		}
	}

	namespace exports::find_algos
	{
		using namespace detail::find_algos::exports;
	}
}
