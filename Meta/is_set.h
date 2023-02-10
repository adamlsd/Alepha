static_assert( __cplusplus > 2020'00 );

#pragma once

#include <Alepha/Alepha.h>

#include <type_traits>
#include <set>

namespace Alepha::Hydrogen::Meta
{
	inline namespace exports { inline namespace type_traits {} }

	namespace detail::type_traits::is_set
	{
		inline namespace exports
		{
			template< typename T >
			struct is_set : std::false_type {};

			template< typename T, typename ... Args >
			struct is_set< std::set< T, Args... > > : std::true_type {};

			template< typename T >
			constexpr bool is_set_v= is_set< T >::value;
		} 
	}

	namespace exports::type_traits
	{
		using namespace detail::type_traits::is_set::exports;
	}
}

