static_assert( __cplusplus > 2020'00 );

#pragma once

#include <Alepha/Alepha.h>

#include <type_traits>

#include <Alepha/Meta/is_list.h>
#include <Alepha/Meta/is_forward_list.h>
#include <Alepha/Meta/is_vector.h>
#include <Alepha/Meta/is_deque.h>
#include <Alepha/Meta/is_string.h>

namespace Alepha::Hydrogen::Meta
{
	inline namespace exports { inline namespace type_traits {} }

	namespace detail::type_traits::is_sequence
	{
		inline namespace exports
		{
			template< typename T >
			constexpr bool is_sequence_v= is_list_v< T > or is_forward_list_v< T > or is_deque_v< T > or is_string_v< T > or is_vector_v< T >;

			template< typename T >
			struct is_sequence : std::bool_constant< is_sequence_v< T > > {};
		} 
	}

	namespace exports::type_traits
	{
		using namespace detail::type_traits::is_sequence::exports;
	}
}

