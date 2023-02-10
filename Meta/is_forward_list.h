static_assert( __cplusplus > 2020'00 );

#pragma once

#include <Alepha/Alepha.h>

#include <type_traits>
#include <forward_list>

namespace Alepha::Hydrogen::Meta
{
	inline namespace exports { inline namespace type_traits {} }

	namespace detail::type_traits::is_forward_list
	{
		inline namespace exports
		{
			template< typename T >
			struct is_forward_list : std::false_type {};

			template< typename T, typename ... Args >
			struct is_forward_list< std::forward_list< T, Args... > > : std::true_type {};

			template< typename T >
			constexpr bool is_forward_list_v= is_forward_list< T >::value;
		} 
	}

	namespace exports::type_traits
	{
		using namespace detail::type_traits::is_forward_list::exports;
	}
}

