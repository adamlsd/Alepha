static_assert( __cplusplus > 2020'00 );

#pragma once

#include <Alepha/Alepha.h>

#include <Alepha/Meta/is_pair.h>
#include <Alepha/Meta/is_tuple.h>

namespace Alepha::Hydrogen::Meta
{
	inline namespace exports { inline namespace type_traits {} }

	namespace detail::type_traits::is_product_type
	{
		inline namespace exports
		{
			template< typename T >
			constexpr bool is_product_type_v= is_tuple_v< T > or is_pair_v< T >;

			template< typename T >
			struct is_product_type : std::bool_constant< is_product_type_v< T > > {};
		}
	}

	namespace exports::type_traits
	{
		using namespace detail::type_traits::is_product_type::exports;
	}
}
