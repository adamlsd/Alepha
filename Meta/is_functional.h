static_assert( __cplusplus > 2020'00 );

#pragma once

#include <Alepha/Alepha.h>

#include <functional>

namespace Alepha::Hydrogen::Meta
{
	inline namespace exports { inline namespace type_traits {} }

	namespace detail::type_traits::is_functional
	{
		inline namespace exports {}

		template< typename T, typename= void >
		struct is_functional : std::false_type {};

		template< typename T >
		struct is_functional< T, std::void_t< decltype( std::function< std::declval< T >() > ) > > : std::true_type {};

		namespace exports
		{
			template< typename T >
			constexpr bool is_functional_v= is_functional< T >::value;

			template< typename T >
			struct is_functional : std::bool_constant< bool, is_functional_v< T > > {};
		}
	}

	namespace exports::type_traits
	{
		using namespace detail::type_traits::is_functional::exports;
	}
}
