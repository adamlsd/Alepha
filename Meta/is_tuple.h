static_assert( __cplusplus > 201700, "C++17 Required" );

#pragma once

#include <Alepha/Alepha.h>

#include <type_traits>
#include <tuple>

namespace Alepha::Hydrogen::Meta
{
	inline namespace exports { inline namespace type_traits {} }

	namespace detail::type_traits::is_tuple
	{
		inline namespace exports
		{
			template< typename T >
			struct is_tuple : std::false_type {};

			template< typename ... Args >
			struct is_tuple< std::tuple< Args... > > : std::true_type {};

			template< typename T >
			constexpr bool is_tuple_v= is_tuple< T >::value;
		}
	}

	namespace exports::type_traits
	{
		using namespace detail::type_traits::is_tuple::exports;
	}
}
