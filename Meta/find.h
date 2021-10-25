static_assert( __cplusplus > 201700, "C++17 Required" );

#pragma once

#include <Alepha/Alepha.h>

#include <tuple>

#include <Alepha/Meta/list.h>
#include <Alepha/Meta/functional.h>

namespace Alepha::Hydrogen::Meta
{
	inline namespace exports { inline namespace find_algos {} }

	namespace detail::find_algos
	{
		inline namespace exports
		{
			template< typename Predicate, typename Tuple >
			struct find_if;

			template< typename Predicate, typename First, typename ... Elements >
			struct find_if< Predicate, list< First, Elements... > >
				: std::conditional_t< Meta::call< Predicate, First >::value, std::true_type, find_if< Predicate, list< Elements... > > >::type {};

			template< typename Predicate >
			struct find_if< Predicate, list<> > : std::false_type {};

			template< typename Predicate, typename List >
			constexpr bool find_if_v= find_if< Predicate, List >::value;
		}

		namespace exports
		{
			template< typename Key, typename Argument >
			constexpr bool find_v= find_if_v< Meta::bind1st< std::is_same, Key >, Argument >;
		}
	}

	namespace exports::find_algos
	{
		using namespace detail::find_algos::exports;
	}
}
