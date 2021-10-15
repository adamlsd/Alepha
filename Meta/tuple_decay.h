static_assert( __cplusplus > 201700, "C++17 Required" );

#pragma once

#include <Alepha/Alepha.h>

#include <type_traits>

#include <Alepha/Stud/type_traits.h>

namespace Alepha::Hydrogen::Meta
{
	inline namespace exports { inline namespace type_traits {} }

	namespace detail::type_traits::tuple_decay
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

	namespace exports::type_traits
	{
		using namespace detail::type_traits::tuple_decay::exports;
	}
}
