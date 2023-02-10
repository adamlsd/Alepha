static_assert( __cplusplus > 2020'00 );

#pragma once

#include <Alepha/Alepha.h>

#include <type_traits>

#include <Alepha/Stud/type_traits.h>

namespace Alepha::Hydrogen::Meta
{
	inline namespace exports { inline namespace type_traits {} }

	namespace detail::type_traits::tuple_decay_trait
	{
		inline namespace exports
		{
			template< typename T > struct tuple_decay;

			template< typename ... Args >
			struct tuple_decay< std::tuple< Args... > >
			{
				using type= std::tuple< Stud::decay_t< Args >... >;
			};

			template< typename T >
			using tuple_decay_t= typename tuple_decay< T >::type;
		}
	}

	namespace exports::type_traits::inline tuple_decay_trait
	{
		using namespace detail::type_traits::tuple_decay_trait::exports;
	}
}
