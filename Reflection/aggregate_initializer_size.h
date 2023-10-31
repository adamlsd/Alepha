static_assert( __cplusplus > 2020'00 );

#pragma once

#include <Alepha/Alepha.h>

#include <tuple>
#include <utility>
#include <type_traits>

#include <Alepha/Reflection/detail/config.h>

namespace Alepha::Hydrogen::Reflection  ::detail:: aggregate_initializer_size_m
{
	inline namespace exports {}


	// Basic methodology here: I probe the number of arguments that an object can be constructed with.  I don't
	// care what they actually are, as there's no easy way to hoist out what they are.  However, for an aggregate,
	// the largest number of items one can initialize it with is related to the number of items that it contains.
	// Therefore as long as the inspection is performed on aggregates and yields the maximum number it finds, then
	// it will always yield a valid initialization pack count.  Other information is then used to winnow that
	// down to arrive at a member count.

	// The basic adaptable argument.  Because it pretends to be anything, it can be used as a parameter in invoking
	// any initialization method.
	struct argument
	{
		template< typename T > constexpr operator T ();
	};

	// The first step is to just start it all off with a blank sequence and walk forward from there.
	// The default arguments cause it to start with the blank sequence, even if it doesn't match this
	// case in the specialization selection.
	template< typename T, typename seq= std::index_sequence<>, typename= void, typename= std::enable_if_t< std::is_aggregate_v< T > > >
	struct init_count_impl
		// When this base case is reached, the size of the sequence is the argument count.
		: std::integral_constant< std::size_t, seq::size() > {};

	// This expansion case always matches when an initializer of the number of elements in the sequence is syntactically
	// valid.  It also recurses, thus exploring the whole initializer set.  There is one fewer value in the sequence set
	// than we use to initialize so that when SFINAE gives up, it defers to the base case above, thus having the right
	// count.
	template< typename T, std::size_t ... values >
	struct init_count_impl
	<
		T,
		std::index_sequence< values... >,
		std::void_t< decltype( T{ ( values, std::declval< argument >() )..., std::declval< argument >() } ) >,
		void
	>
		// Descend and take the next element in the sequence.
		: init_count_impl< T, std::index_sequence< values..., sizeof...( values ) > > {};

	namespace exports
	{
		template< typename T >
		constexpr std::size_t aggregate_initializer_size_v= init_count_impl< T >::value;

		template< typename T >
		using aggregate_initializer_size= typename init_count_impl< T >::type;
	}
}

namespace Alepha::Hydrogen::Reflection::inline exports::inline aggregate_initializer_size_m
{
	using namespace detail::aggregate_initializer_size_m::exports;
}
