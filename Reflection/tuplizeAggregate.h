static_assert( __cplusplus > 201700, "C++17 Required" );

#pragma once

#include <Alepha/Alepha.h>

#include <boost/preprocessor.hpp>

#include <Alepha/Reflection/detail/config.h>
#include <Alepha/Reflection/aggregate_members.h>

namespace Alepha::Hydrogen::Reflection
{
	inline namespace exports { inline namespace tuplize_aggregate {} }

	namespace detail::tuplize_aggregate
	{
		inline namespace exports {}

		template< typename T, typename= void >
		struct has_salient_members_constant : std::false_type {};

		template< typename T >
		struct has_salient_members_constant< T, std::void_t< decltype( T::salient_members ) > >
			: std::true_type {};

		template< typename T >
		constexpr bool has_salient_members_constant_v= has_salient_members_constant< T >::value;

		template< typename T >
		constexpr std::size_t
		compute_salient_members_count_impl()
		{
			if constexpr( has_salient_members_constant_v< T > ) return T::salient_members;
			else return aggregate_member_count_v< std::decay_t< T > >;
		}

		template< typename T >
		constexpr std::size_t compute_salient_members_count_v= compute_salient_members_count_impl< T >();

		namespace exports
		{
			/*!
			 * Deconstruct an aggregate object into a tie-based tuple pointing at its members.
			 *
			 * C++17's primary new reflection-oriented introduction is Structured Binding Declarations.
			 * What these let one do is to introduce a set of named variables that bind to each member
			 * of a (raw aggregate) `struct` in turn.  This leads to some very interesting forms of
			 * "reflection" about what a user defined type is made of.  Combined with the `std::is_aggregate`
			 * trait function and a way to determine the number of member objects, this provides a
			 * powerful new way to inspect any type.
			 *
			 * Structured Binding Declarations can also be used with arrays or types which implement a subset
			 * of the `std::tuple` interface.  Those cases are not as interesting.  We've always had the
			 * ability to inspect arrays via templates -- simple deduction operations work for that.  And
			 * C++11's `std::tuple`s are already inspectable by their nature and types which implement a tuple-like
			 * interface are also easily inspected by pre-C++17 means.
			 *
			 * The most important thing C++17 Structured Binding brings to the language is the ability to
			 * (at compiletime) programmatically inspect any structure's members -- to learn their types,
			 * and with a bit of special effort, to learn their offsets.  The names of those members are
			 * hidden, but their types are available, as is a way to work with all of them at once.  Any
			 * Structured Binding is sufficient to do this -- one need only give a new name for each member of
			 * the type.  `auto &[ a, b, c, d ]= someStruct;` is all that is needed and one has already performed
			 * an interesting feat of rudimentary reflection on the type `someStruct`.  By loading those values
			 * into a tuple (by reference), by code such as `std::tie( a, b, c, d )`, a programmer can provide
			 * an anonymized, distilled reflection of the contents of that `struct`.  This said, a library function
			 * which can decompose any `struct` into such a tie is very useful.  `tuplizeAggregate` is exactly this.
			 *
			 * This function contains a pre-built set of such decompositions for structs of various sizes.  C++17
			 * does not permit arbitrarily sized Structured Bindings, and so a limit had to be placed.  The limit
			 * is fairly generous, however.  If an aggregate size which is greater than the pre-build maximum is
			 * provided, then the compile will fail on a `static_assert` indicating this.
			 *
			 * Unfortunately, as a declaration syntax, the number of members in a `struct`'s body cannot be inferred
			 * through SFINAE by this means.  Normally the user must explicitly provide the number of member
			 * variables.  However, combined with a pair of C++11 features (based upon variadic templates and
			 * aggregate initialization syntax) we can infer the number of memmber values via a set of helper
			 * templates (which can also be called directly.)
			 *
			 * This kind of reflection into an aggregate type can prove very useful.  Code generators for
			 * serialization, conversion tools, universal utility functions, and much more can all be built in
			 * C++17, today, using this kind of reflection!  There's no need to wait for C++23 or beyond when
			 * static reflection is added to the language.  A great deal of desired reflection use cases can be
			 * attained today.  One just need write some code generators in terms of `std::tuple` and `std::tie`,
			 * then make any overloads (perhaps using ADL hooking tricks) which call `Alepha::Reflection::tuplizeAggregate`
			 * and pass that result to the general tuple form.  For serializers and such, other techniques such as
			 * `boost::core::demangle( typeid( instance ).name() )` can be used to get nice names for types when
			 * implementing universal serializers.  In fact, this can be used as a crutch for serializing more
			 * complicated user types (with private data and such).  Those types can produce an aggregate "view"
			 * of what they must serialize or deserialize, and then they can hand that view off to such code
			 * generators.  And, of cousre, this need not apply just to serialization.
			 *
			 * @param agg Aggregate instance to decompose into a `std::tie` based `std::tuple`.
			 * @tparam aggregate_size The number of members in the aggregate argument `agg`'s definition.
			 * @tparam Aggregate The type of the aggregate to decompose.
			 */
			// TODO: Make `aggregate_size` deduced via `Reflection::aggregate_ctor...` means.
			template< std::size_t aggregate_size, typename Aggregate, typename= std::enable_if_t< not std::is_rvalue_reference_v< Aggregate > > >
			constexpr decltype( auto )
			tuplizeAggregate( Aggregate &&agg )
			{
				static_assert( std::is_aggregate_v< std::decay_t< Aggregate > >, "`tuplizeAggregate` only can be used on aggregates" );

				// TODO: Generate these cases via boost preprocessor, to cut down on repetition...
				if constexpr( aggregate_size == 0 ) return std::tuple{};
				else if constexpr( aggregate_size == 1 )
				{
					auto &[ a0 ]= agg;
					return std::tie( a0 );
				}
				else if constexpr( aggregate_size == 2 )
				{
					auto &[ a0, a1 ]= agg;
					return std::tie( a0, a1 );
				}
				else if constexpr( aggregate_size == 3 )
				{
					auto &[ a0, a1, a2 ]= agg;
					return std::tie( a0, a1, a2 );
				}
				else if constexpr( aggregate_size == 4 )
				{
					auto &[ a0, a1, a2, a3 ]= agg;
					return std::tie( a0, a1, a2, a3 );
				}
				else if constexpr( aggregate_size == 5 )
				{
					auto &[ a0, a1, a2, a3, a4 ]= agg;
					return std::tie( a0, a1, a2, a3, a4 );
				}
				else if constexpr( aggregate_size == 6 )
				{
					auto &[ a0, a1, a2, a3, a4, a5 ]= agg;
					return std::tie( a0, a1, a2, a3, a4, a5 );
				}
				else if constexpr( aggregate_size == 7 )
				{
					auto &[ a0, a1, a2, a3, a4, a5, a6 ]= agg;
					return std::tie( a0, a1, a2, a3, a4, a5, a6 );
				}
				else if constexpr( aggregate_size == 8 )
				{
					auto &[ a0, a1, a2, a3, a4, a5, a6, a7 ]= agg;
					return std::tie( a0, a1, a2, a3, a4, a5, a6, a7 );
				}
				else if constexpr( aggregate_size == 9 )
				{
					auto &[ a0, a1, a2, a3, a4, a5, a6, a7, a8 ]= agg;
					return std::tie( a0, a1, a2, a3, a4, a5, a6, a7, a8 );
				}
				else if constexpr( aggregate_size == 10 )
				{
					auto &[ a0, a1, a2, a3, a4, a5, a6, a7, a8, a9 ]= agg;
					return std::tie( a0, a1, a2, a3, a4, a5, a6, a7, a8, a9 );
				}
				else if constexpr( aggregate_size == 11 )
				{
					auto &[ a0, a1, a2, a3, a4, a5, a6, a7, a8, a9, a10 ]= agg;
					return std::tie( a0, a1, a2, a3, a4, a5, a6, a7, a8, a9, a10 );
				}
				else if constexpr( aggregate_size == 12 )
				{
					auto &[ a0, a1, a2, a3, a4, a5, a6, a7, a8, a9, a10, a11 ]= agg;
					return std::tie( a0, a1, a2, a3, a4, a5, a6, a7, a8, a9, a10, a11 );
				}
				else if constexpr( aggregate_size == 13 )
				{
					auto &[ a0, a1, a2, a3, a4, a5, a6, a7, a8, a9, a10, a11, a12 ]= agg;
					return std::tie( a0, a1, a2, a3, a4, a5, a6, a7, a8, a9, a10, a11, a12 );
				}
				else if constexpr( aggregate_size == 14 )
				{
					auto &[ a0, a1, a2, a3, a4, a5, a6, a7, a8, a9, a10, a11, a12, a13 ]= agg;
					return std::tie( a0, a1, a2, a3, a4, a5, a6, a7, a8, a9, a10, a11, a12, a13 );
				}
				else if constexpr( aggregate_size == 15 )
				{
					auto &[ a0, a1, a2, a3, a4, a5, a6, a7, a8, a9, a10, a11, a12, a13, a14 ]= agg;
					return std::tie( a0, a1, a2, a3, a4, a5, a6, a7, a8, a9, a10, a11, a12, a13, a14 );
				}
				else if constexpr( aggregate_size == 16 )
				{
					auto &[ a0, a1, a2, a3, a4, a5, a6, a7, a8, a9, a10, a11, a12, a13, a14, a15 ]= agg;
					return std::tie( a0, a1, a2, a3, a4, a5, a6, a7, a8, a9, a10, a11, a12, a13, a14, a15 );
				}
				// Impossible, in this case -- we would have taken the original 0 branch were this so.
				else static_assert( aggregate_size == 0, "The specified aggregate has more members than `tuplizeAggregate` can handle" );
			}

			// This overload deduces the aggregate size using the initializer inspection utilities.
			template< typename Aggregate >
			constexpr decltype( auto )
			tuplizeAggregate( Aggregate &&agg )
			{
				return tuplizeAggregate< compute_salient_members_count_v< std::decay_t< Aggregate > > >( std::forward< Aggregate >( agg ) );
			}
		}
	}

	namespace exports::tuplize_aggregate
	{
		using namespace detail::tuplize_aggregate::exports;
	}
}
