static_assert( __cplusplus > 2020'00 );

#pragma once

#include <type_traits>
#include <tuple>
#include <vector>
#include <deque>
#include <list>
#include <array>
#include <optional>
#include <functional>
#include <string_view>

//#include "meta_matching.hpp"

#include "type_lisp.h"

namespace Alepha::Hydrogen  ::detail::  meta
{
	inline namespace exports
	{
		template< auto q, typename >
		constexpr decltype( q ) dependent_value= q;

		template< typename T >
		constexpr bool is_array_v= false;

		template< typename T, std::size_t sz >
		constexpr bool is_array_v< std::array< T, sz > >{ true };

		template< typename T, template< typename ... > class Ref >
		constexpr bool is_specialization_of_v= false;

		template< template< typename ... > class Ref, typename ... Args >
		constexpr bool is_specialization_of_v< Ref< Args... >, Ref >{ true };

		template< typename T, typename ... Member >
		constexpr bool is_specialized_on_v= false;

		template< template< typename ... > class Base, typename Member >
		constexpr bool is_specialized_on_v< Base< Member >, Member >{ true };

		template< typename T >
		concept is_functional_v=
		requires( const T &t )
		{
			{ std::function( t ) };
		};

		template< typename T >
		constexpr bool is_pair_v= is_specialization_of_v< T, std::pair >;

		template< typename T >
		constexpr bool is_tuple_v= is_specialization_of_v< T, std::tuple >;

		template< typename T >
		struct decay_tuple;

		template< typename ... Args >
		struct decay_tuple< std::tuple< Args... > >
		{
			using type= std::tuple< std::decay_t< Args >... >;
		};

		template< typename T >
		using decay_tuple_t= typename decay_tuple< T >::type;

		template< typename T >
		constexpr bool is_optional_v= is_specialization_of_v< T, std::optional >;
	}
}

namespace Alepha::Hydrogen::inline exports::inline meta
{
	using namespace detail::meta::exports;
}
