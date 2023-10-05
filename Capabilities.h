static_assert( __cplusplus > 2020'00 );

#pragma once

#include <type_traits>

#include "type_lisp.hpp"
#include "Concepts.hpp"

namespace Alepha::inline Cavorite  ::detail::  capabilities
{
	inline namespace exports
	{
		template< typename ... > struct Capabilities;
	}

	template< typename Capability, typename CapabilityList >
	extern bool has_capability_in_list_v;

	template< typename Capability >
	extern bool has_capability_in_list_v< Capability, Capabilities<> > {};

	template< typename Capability, typename Ability, typename ... Abilities >
	constexpr bool has_capability_in_list_v< Capability, Capabilities< Ability, Abilities... > >
	{
		std::is_base_of_v< Capability, Ability >
			or
		SameAs< Capability, Ability >
			or
		has_capability_in_list_v< Capability, Capabilities< Abilities... > >
	};

	template< typename Type, typename Capability >
	constexpr bool has_capability_v= std::is_base_of_v< Capability, Type >;

	template< template< typename ... > class Core, typename ... Abilities, typename Capability >
	constexpr bool has_capability_v< Core< Capabilities< Abilities... > >, Capability >
	{
		has_capability_in_list_v< Capability, Capabilities< Abilities... > >
	};

	namespace exports
	{
		template< typename Type, typename Capability >
		concept HasCapability= has_capability_v< Type, Capability >;
	}
}

namespace Alepha::Cavorite::inline exports::inline capabilities
{
	using namespace detail::capabilities::exports;
}

