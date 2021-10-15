static_assert( __cplusplus > 201700, "C++17 Required" );

#pragma once

#include <Alepha/Alepha.h>

#include <type_traits>
#include <utility>

namespace Alepha::Hydrogen::Meta
{
	inline namespace exports { inline namespace type_traits {} }

	namespace detail::type_traits::is_pair
	{
		inline namespace exports
		{
			template< typename T >
			struct is_pair : std::false_type {};

			template< typename ... Args >
			struct is_pair< std::pair< Args... > > : std::true_type {};

			template< typename T >
			constexpr bool is_pair_v= is_pair< T >::value;
		}
	}

	namespace exports::type_traits
	{
		using namespace detail::type_traits::is_pair::exports;
	}
}
