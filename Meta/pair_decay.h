static_assert( __cplusplus > 201700, "C++17 Required" );

#pragma once

#include <Alepha/Alepha.h>

#include <type_traits>

#include <Alepha/Stud/type_traits.h>

namespace Alepha::Hydrogen::Meta
{
	inline namespace exports { inline namespace type_traits {} }

	namespace detail::type_traits::pair_decay
	{
		inline namespace exports
		{
			template< typename T > struct pair_decay;

			template< typename First, typename Second >
			struct pair_decay< std::pair< First, Second > >
			{
				using type= std::pair< Stud::decay_t< First >, Stud::decay_t< Second > >;
			};

			template< typename T >
			using pair_decay_t= typename pair_decay< T >::type;
		}
	}

	namespace exports::type_traits
	{
		using namespace detail::type_traits::pair_decay::exports;
	}
}