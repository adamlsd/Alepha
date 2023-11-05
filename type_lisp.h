static_assert( __cplusplus > 2020'00 );

#pragma once

#include <Alepha/Alepha.h>

#include <type_traits>

namespace Alepha::Hydrogen  ::detail::  type_lisp_m
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
	constexpr bool is_type_list_v= false;

	template< typename ... Args >
	constexpr bool is_type_list_v< TypeList< Args... > >{ true };

	namespace exports
	{
		template< typename Type >
		concept TypeListType= is_type_list_v< Type >;
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

	template< typename Element, typename List >
	struct cons;

	template< typename Element, typename ... Members >
	struct cons< Element, TypeList< Members... > >
	{
		using type= TypeList< Element, Members... >;
	};

	namespace exports
	{
		template< typename Element, typename List >
		using cons_t= typename cons< Element, List >::type;
	}

	template< typename Tuple >
	struct list_from_tuple;

	namespace exports
	{
		template< typename Tuple >
		using list_from_tuple_t= typename list_from_tuple< Tuple >::type;
	}

	template< typename ... Members >
	struct list_from_tuple< std::tuple< Members... > >
	{
		using type= TypeList< Members... >;
	};
}

namespace Alepha::Hydrogen::inline exports::inline type_lisp_m
{
	using namespace detail::type_lisp_m::exports;
}
	
