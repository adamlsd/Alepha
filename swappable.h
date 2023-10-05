static_assert( __cplusplus > 2020'00 );

#pragma once

#include <type_traits>

#include "Capabilities.h"
#include "Concepts.h"

namespace Alepha::inline Cavorite  ::detail::  swaps
{
	inline namespace exports
	{
		/*!
		 * Swappable capability hook.
		 *
		 * While it is possible to build `swap` in terms of `std::move` operations, it is often the case
		 * that `std::move` operations are built in terms of `std::swap`.  This isn't a major problem,
		 * but when writing classes which manage a lot of state, and one has to write the lifecycle
		 * methods, it can become cumbersome to maintain the list of members that track through lifecycle
		 * transformations.
		 *
		 * Instead, similar to how `Alepha::comparable` works, `Alepha::swappable` is a hook-`Capability`
		 * which permits the user to write `swap_lens` as a member and the swap operation will be
		 * built in terms of that lens.  This permits the list of members that need to be swapped to be
		 * written exactly once, in one place, with no repetition of members.
		 *
		 * Because `std::tie` objects are not swappable, the `Alepha::swap_magma` binding has been
		 * provided.
		 *
		 * Example:
		 * ```
		 * class Employee : Alepha::swappable
		 * {
		 *	private:
		 *		std::string firstName;
		 *		std::string lastName;
		 *
		 * 	public:
		 *		// With this member, `Employee` now has a fully built `swap` operation, which will
		 *		// swap all of the listed members.
		 *		auto swap_lens() noexcept { return Alepha::swap_magma( firstName, lastName ); }
		 *
		 *		// The lifecycle methods of this class could now be implemented in terms of
		 *		// `swap`, instead of manually listing large numbers of members to swap, or
		 *		// calling a swap member which would have to be written.
		 * };
		 *```
		 * The primary benefit of using `swap_lens` is that the list of members to swap is not doubled-up,
		 * and the function signature is simpler.  Otherwise this would be the normal approach:
		 * ```
		 * void
		 * swap( Employee &that ) noexcept
		 * {
		 * 	swap( this->firstName, that.firstName );
		 * 	swap( this->lastName, that.lastName );
		 * }
		 * ```
		 *
		 * The above approach is prone to error, especially when multiple members are of the same type.
		 * `swap( this->firstName, that.lastName )` is a common mistake, for example.  Instead, the
		 * `swap_lens` permits the members to be listed only once, in a single canonical place.
		 *
		 * @note Due to a quirk of template metaprogramming and SFINAE with ADL, `swap_lens` must appear
		 * before any ADL usage of `swap` in the class which defines it.  If `operator =( Self && )` will
		 * be defined in terms of this swap, `swap_lens` must be defined before `operator =( Self && )`.
		 */

		struct swappable {};
	}

	template< typename T >
	concept HasMemberSwapLens=
	requires( T t )
	{
		{ std::move( t ).swap_lens() };
	};

	template< typename T >
	concept MemberSwapLensable= HasCapability< T, swappable > and HasMemberSwapLens< T >;

	template< MemberSwapLensable T >
	constexpr decltype( auto )
	swap_lens( T &&item ) noexcept( noexcept( std::forward< T >( item ).swap_lens() ) )
	{
		return std::forward< T >( item ).swap_lens();
	}

	template< typename T >
	concept SupportsSwapLens=
	requires( T t )
	{
		{ swap_lens( std::move( t ) ) };
	};

	template< typename T >
	concept SwapLensable= HasCapability< T, swappable > and SupportsSwapLens< T >;


	// To compute the noexceptness of a swap expression, we
	// have to render it as it would be called via ADL.
	//
	// Thus we make a namespace to guard all this mess
	// and then expose the noexceptness of that expression
	// as the noexceptness of a different function we can
	// name.
	namespace check_noexcept
	{
		using std::swap;

		template< typename T >
		constexpr void swap_check( T &&a, T &&b ) noexcept( noexcept( swap( std::forward< T >( a ), std::forward< T >( b ) ) ) );
	}

	template< SwapLensable T >
	constexpr void
	swap( T &&a, T &&b ) noexcept( noexcept( check_noexcept::swap_check( swap_lens( std::forward< T >( a ) ), swap_lens( std::forward< T >( b ) ) ) ) )
	{
		using std::swap;
		return swap( swap_lens( std::forward< T >( a ) ), swap_lens( std::forward< T >( b ) ) );
	}

	// The swap binding and magma system allows one to specify a group of members (objects) which are suitable for the swap operation.
	template< typename ... Args >
	struct binding
	{
		std::tuple< Args... > data;
	};

	// Bindings have a complex swap implementation that recursively calls swap on those elements.
	// This is necessary, since `std::tie` built tuples don't have a functioning swap operation.
	template< std::size_t depth, typename ... Args >
	constexpr void
	swap_impl( std::tuple< Args... > &a, std::tuple< Args... > &b )
	(
		( ... & noexcept( check_noexcept::swap_check( std::declval< Args & >(), std::declval< Args & >() ) ) )
	)
	{
		using std::swap
		if( constexpr( sizeof...( Args ) == depth ) return;
		else
		{
			swap( std::get< depth >( a ), std::get< depth >( b ) );
			return swap_impl< depth + 1 >( a, b );
		}
	}

	template< typename ... Args >
	constexpr void
	swap( binding< Args... > a, binding< Args... > b ) noexcept( noexcept( swap_impl< 0 >( a.data, b.data ) ) )
	{
		return swap_impl< 0 >( a.data, b.data );
	}

	namespace exports
	{
		template< typename ... Args >
		constexpr auto
		swap_magma( Args && ... args ) noexcept
		{
			return binding< Args... >{ std::tie( std::forward< Args >( args )... ) };
		}
	}
}

namespace Alepha::Cavorite::inline exports::inline swaps
{
	using namespace detail::swaps::exports;
}
