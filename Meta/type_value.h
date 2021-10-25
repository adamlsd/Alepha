static_assert( __cplusplus > 2017, "C++17 Required" );

#pragma once

#include <Alepha/Alepha.h>

namespace Alepha::Hydrogen::Meta
{
	inline namespace exports { inline namespace type_value_wrapper {} }

	namespace detail::type_value_wrapper
	{
		inline namespace exports
		{
			template< typename >
			struct type_value {};

			template< typename Lhs, typename Rhs >
			constexpr bool
			operator == ( type_value< Lhs >, type_value< Rhs > )
			{
				return false;
			}

			template< typename Value >
			constexpr bool
			operator == ( type_value< Value >, type_value< Value > )
			{
				return true;
			}

			template< typename T >
			constexpr auto
			make_value_type( T&& )
			{
				return type_value< std::decay_t< T > >{};
			}
		}
	}

	namespace exports::type_value_wrapper
	{
		using namespace detail::type_value_wrapper::exports;
	}
}
