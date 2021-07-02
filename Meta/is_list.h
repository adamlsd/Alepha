static_assert( __cplusplus > 201700, "C++17 Required" );

#pragma once

#include <Alepha/Alepha.h>

#include <type_traits>
#include <list>

namespace Alepha::Hydrogen::Meta
{
	inline namespace exports { inline namespace type_traits {} }

	namespace detail::type_traits::is_list
	{
		inline namespace exports
		{
			template< typename T >
			struct is_list : std::false_type {};

			template< typename T, typename ... Args >
			struct is_list< std::list< T, Args... > > : std::true_type {};

			template< typename T >
			constexpr bool is_list_v= is_list< T >::value;
		} 
	}

	namespace exports::type_traits
	{
		using namespace detail::type_traits::is_list::exports;
	}
}

