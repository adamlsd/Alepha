static_assert( __cplusplus > 2020'00 );

#pragma once

#include <Alepha/Alepha.h>

#include <type_traits>

#include <Alepha/Stud/type_traits.h>

#include <Alepha/Meta/pair_decay.h>
#include <Alepha/Meta/tuple_decay.h>

namespace Alepha::Hydrogen::Meta
{
	inline namespace exports { inline namespace type_traits {} }

	namespace detail::type_traits::product_type_decay
	{
		inline namespace exports
		{
			template< typename T > struct product_type_decay;

			template< typename ... Args >
			struct product_type_decay< std::pair< Args... > > : pair_decay< std::pair< Args... > > {};

			template< typename ... Args >
			struct product_type_decay< std::tuple< Args... > > : tuple_decay< std::tuple< Args... > > {};

			template< typename T >
			using product_type_decay_t= typename product_type_decay< T >::type;
		}
	}

	namespace exports::type_traits
	{
		using namespace detail::type_traits::product_type_decay::exports;
	}
}

