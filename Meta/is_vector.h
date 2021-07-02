static_assert( __cplusplus > 201700, "C++17 Required" );

#pragma once

#include <Alepha/Alepha.h>

#include <type_traits>
#include <vector>

namespace Alepha::Hydrogen::Meta
{
	inline namespace exports { inline namespace type_traits {} }

	namespace detail::type_traits::is_vector
	{
		inline namespace exports
		{
			template< typename T >
			struct is_vector : std::false_type {};

			template< typename T, typename ... Args >
			struct is_vector< std::vector< T, Args... > > : std::true_type {};

			template< typename T >
			constexpr bool is_vector_v= is_vector< T >::value;
		} 
	}

	namespace exports::type_traits
	{
		using namespace detail::type_traits::is_vector::exports;
	}
}

