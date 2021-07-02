static_assert( __cplusplus > 201700, "C++17 Required" );

#pragma once

#include <Alepha/Alepha.h>

#include <type_traits>
#include <string>

namespace Alepha::Hydrogen::Meta
{
	inline namespace exports { inline namespace type_traits {} }

	namespace detail::type_traits::is_string
	{
		inline namespace exports
		{
			template< typename T >
			struct is_string : std::false_type {};

			template< typename T, typename ... Args >
			struct is_string< std::basic_string< T, Args... > > : std::true_type {};

			template< typename T >
			constexpr bool is_string_v= is_string< T >::value;
		} 
	}

	namespace exports::type_traits
	{
		using namespace detail::type_traits::is_string::exports;
	}
}

