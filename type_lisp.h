static_assert( __cplusplus > 2020'00 );

#pragma once

#include <type_traits>

namespace Alepha::inline Cavorite  ::detail::  type_lisp
{
	inline namespace exports {}

	namespace exports
	{
		template< typename ... > struct TypeList {};

		using Nil= TypeList<>;

		template< typename T >
		constexpr bool is_nil_v= std::is_same_v< T, Nil >;
	}

	template< typename T >
	struct tuple_from_list;

	namespace exports
	{
		template< typename List >
		using tuple_from_list_t= typename tuple_from_list< List >::type;
	}

	template< typename ... Members >
	struct tuple_from_list< TypeList< Members... > >
	{
		using type= std::tuple< Members... >;
	};

	template< typename List >
	struct car_impl { using type= Nil; };

	template< typename Arg0, typename ... Args >
	struct car_impl< TypeList< Arg0, Args... > >
	{
		using type= Arg0;
	};

	template< typename List >
	struct cdr_impl { using type= Nil; };

	template< typename Arg0, typename ... Args >
	struct cdr_impl< TypeList< Arg0, Args... > >
	{
		using type= TypeList< Args... >;
	};

	namespace exports
	{
		template< typename List >
		using car_t= typename car_impl< List >::type;

		template< typename List >
		using cdr_t= typename cdr_impl< List >::type;
	}

	namespace exports
	{
		template< typename List, typename Element >
		constexpr bool list_contains_v=
			std::is_same_v< Element, car_t< List > >
				or
			list_contains_v< cdr_t< List >, Element >;

		template< typename Element >
		constexpr bool list_contains_v< Nil, Element >{};
	}
}

namespace Alepha::Cavorite::inline exports::inline type_lisp
{
	using namespace detail::type_lisp::exports;
}
	
