static_assert( __cplusplus > 2020'00 );

#pragma once

#include <Alepha/Alepha.h>

#include <type_traits>
#include <optional>

namespace Alepha::Hydrogen::Meta
{
	inline namespace exports { inline namespace type_traits {} }

	namespace detail::type_traits::is_optional
	{
		inline namespace exports
		{
			template< typename T >
			struct is_optional : std::false_type {};

			template< typename T, typename ... Args >
			struct is_optional< std::optional< T, Args... > > : std::true_type {};

			template< typename T >
			constexpr bool is_optional_v= is_optional< T >::value;
		} 
	}

	namespace exports::type_traits
	{
		using namespace detail::type_traits::is_optional::exports;
	}

}
