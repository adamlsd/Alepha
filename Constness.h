static_assert( __cplusplus > 2020'00 );

#include <type_traits>

#include "meta.h"

namespace Alepha::inline Cavorite  ::detail::  constness
{
	inline namespace exports
	{
		/*!
		 * Conditional constness primitive.
		 *
		 * C++17 and beyond have many conditional attributes on functions.  Conditional `noexcept`,
		 * conditional `explicit`, and even conditional instantiation.  The `noexcept` and `explicit`
		 * are conditional on a boolean condition.  This enum permits making templates which
		 * are conditionally const, depending upon instantiation.
		 *
		 * Simple example:
		 *
		 * ```
		 * template< Constness constness >
		 * struct IntWrapper
		 * {
		 *	private:
		 *		int variable;
		 *
		 *  public:
		 *		maybe_const_t< int &, constness > // This is sort of like if one could write `const( constness ) int &`
		 *		view() { return variable; }
		 *
		 *		const int &
		 *		view() const { return variable; }
		 * };
		 *
		 * using MutableIntWrapper= IntWrapper< Mutable >;
		 * using ConstIntWrapper= IntWrapper< Const >;
		 * ```
		 *
		 * Now `Constness< Mutable >` has a member `view` which returns `int &` and `Constness< Const >` has a member
		 * `view` which returns `const int &`.  This facility can be useful in implementing pairs of `const`/non-`const`
		 * iterators, and other gadgets.
		 */
		enum Constness : bool
		{
			Const= true,
			Mutable= false,
		};


		/*!
		 * Apply the `Constness` requested to the specified type.
		 *
		 * If the type isn't `const`, but `Constness` is set, then
		 * it evaluates to `const T`.
		 */
		template< typename Type, Constness > struct maybe_const;

		template< typename Type >
		struct maybe_const< Type, Const >
		{
			using type= std::add_const_t< Type >;
		};

		template< typename Type >
		struct maybe_const< Type, Mutable >
		{
			using type= Type;
		};

		template< typename Type >
		struct maybe_const< Type &, Const >
		{
			using type= std::add_const_t< Type > &;
		};

		template< typename Type >
		struct maybe_const< Type &, Mutable >
		{
			using type= Type &;
		};

		/*!
		 * Conditionally make `Type` `const`.
		 */
		template< typename Type, Constness constness >
		using maybe_const_t= typename maybe_const< Type, constness >::type;

		/*!
		 * Conditionally make `Type` into `const Type *`.
		 */
		template< typename Type, Constness constness >
		using maybe_const_ptr_t= std::add_pointer_t< maybe_const_t< Type, constness >::type;

		/*!
		 * Conditionally call `std::as_const`.
		 *
		 * Sometimes `std::as_const` is appropriate to call, and sometimes it isn't.
		 * In some cases, one might want to have type deduction driven by an expression
		 * where, in a `const` "branch" `std::as_const` is called, but in a non-`const`
		 * branch, it isn't.  This facilitates that:
		 *
		 * ```
		 * template< Constness constness >
		 * struct IntWrapper
		 * {
		 *  private:
		 *		int variable;
		 *
		 *  public:
		 *		// `std::as_const` will be called when `constness` is true,
		 *		// and will be skipped otherwise.  This permits the right
		 * 		// client overload to be called
		 * 		template< typename Function >
		 * 		decltype( auto )
		 * 		apply( Function func )
		 * 		{
		 *			return func( maybe_as_const< constness >( variable ) );
		 * 		}
		 * };
		 * ```
		 */
		template< Constness constness, typename T >
		constexpr decltype( auto )
		maybe_as_const( T &t ) noexcept
		{
			if constexpr( constness ) return std::as_const( t );
			else return t;
		}

		template< Constness constness, typename T >
		constexpr decltype( auto )
		maybe_as_const( const T &t ) noexcept
		{
			if constexpr( constness ) return std::as_const( t );
			else static_assert( dep_value< false, T > );
		}

		template< Constness constness, typename T >
		void maybe_as_const( T && )= delete;
	}
}

namespace Alepha::Cavorite::inline exports::inline constness
{
	using detail::constness::exports;
}
