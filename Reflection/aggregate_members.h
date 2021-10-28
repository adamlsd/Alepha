static_assert( __cplusplus > 201700, "C++17 Required" );

#pragma once

#include <Alepha/Alepha.h>

#include <tuple>
#include <utility>
#include <type_traits>

#include <Alepha/Reflection/detail/config.h>
#include <Alepha/Reflection/aggregate_initialization.h>

#include <Alepha/Meta/overload.h>
#include <Alepha/Meta/type_value.h>

namespace Alepha::Hydrogen::Reflection
{
	inline namespace exports { inline namespace aggregate_members {} }

	namespace detail::aggregate_members
	{
		inline namespace exports {}

		using Meta::overload;

		/*!
		 * Basic methodology here.
		 *
		 * The number of members in an aggregate is equal to the number of initializer parameters it takes less
		 * the number of empty base classes it has.  In simple terms, this would be `init_terms< T > - empty_bases< T >`,
		 * However, it's not that simple.  To do that the easy way, one might need get `std::tuple< InitTerms... >` and then compute
		 * which terms were bases.  Now that gets a bit complicated, as in C++ one can't just directly get a tuple of initializer
		 * arguments (portably) to scoop out the arguments and analyze them one-by-one.  One can, however, constrain the arguments
		 * one-by-one in templates.  Those constraints cannot directly leak out the types they conclude, as that requires side
		 * effects.  (Yes, template-friend-injection can be used here, but these mechanisms are extremely delicate.  They're
		 * not really portable.  Further, the way that constraints get instantiated for matching is prone to complications.)
		 *
		 * Instead, a side-stepping approach is required.  It's trivial to ask: "Can this object be constructed from these
		 * N adaptive types, where the first one is constrained to be a base class of your object's type?"  If yes, then
		 * this proves a (likely) empty base.  One can just recursively iterate through more an more constrained adaptive
		 * types until the first non-base type is reached.  At this point, there are no more than that many base classes.
		 *
		 * There may actually be fewer base classes, however.  Consider:
		 *
		 * ```
		 * struct SneakyBase {};
		 *
		 * struct Complicated : SneakyBase
		 * {
		 *	SneakyBase member;
		 * };
		 * ```
		 * In that case, a constrained adaptable argument would see two base types.  Here is where a bit of C++ trivia and
		 * knowledge comes into play.  C++ forbids repetition of a base class's type.  Therefore the sequence of base classes
		 * cannot have repeats.  The solution is to perform a nested exploration of instantiations of `checker` types which
		 * has each descent disable casting to `std::is_base_of_v` types which already have been expanded.  Thus whatever
		 * count this expands to, it will be the correct empty-bases count.  Then that count can be subtracted from the
		 * initializer list count.
		 *
		 * Note that this will not work with types that have non-empty bases, but those types cannot be decomposed,
		 * anyhow.  Such types cannot have C++17 reflection performed on them.
		 */

		// The basic adaptable argument.  Because it pretends to be anything, it can be used as a parameter in invoking
		// any initialization method.
		struct argument { template< typename T > constexpr operator T (); };

		template< typename T >
		struct checker
		{
			using type= typename checker< T >::type;
			//using type= void;
			//static_assert( std::is_empty_v< T > );
		};

		// Any empty-base-class argument.
		template< typename Aggregate >
		struct empty_base
		{
			template
			<
				typename T,
				//typename= typename checker< std::decay_t< T > >::type,
				typename= std::enable_if_t< std::is_empty_v< std::decay_t< T > > >,
				typename= std::enable_if_t< not std::is_same_v< std::decay_t< T >, Aggregate > >,
				typename= std::enable_if_t< std::is_base_of_v< std::decay_t< T >, Aggregate > >,
				overload< __LINE__ > = nullptr
			>
			constexpr operator T ();

			//template< typename T > constexpr operator T ()= delete;
		};

		template< typename T >
		constexpr bool is_empty_base_v= false;

		template< typename T >
		constexpr bool is_empty_base_v< empty_base< T > >{ true };

		template< typename Tuple, std::size_t baseCount, std::size_t totalCount >
		constexpr void
		check_tuple()
		{
			static_assert( std::tuple_size_v< Tuple > == totalCount );
		}

		template< typename Aggregate, std::size_t bases, std::size_t total >
		constexpr auto
		build_init_tuple()
		{
			static_assert( bases <= total );
			if constexpr( total == 0 ) return std::tuple{};
			else if constexpr( bases > 0 )
			{
				auto result= std::tuple_cat( std::tuple{ empty_base< Aggregate >{} }, build_init_tuple< Aggregate, bases - 1, total - 1 >() );
				check_tuple< decltype( result ), bases, total >();
				return result;
			}
			else
			{
				static_assert( bases == 0 );
				auto result= std::tuple_cat( std::tuple{ argument{} }, build_init_tuple< Aggregate, 0, total - 1 >() );
				check_tuple< decltype( result ), bases, total >();
				return result;
			}
		}

		template< typename T, typename Tuple, typename= void >
		struct is_constructible_from_tuple : std::false_type {};

		template< typename T, typename ... TupleArgs >
		struct is_constructible_from_tuple
		<
			T,
			std::tuple< TupleArgs... >,
			std::void_t< decltype( T{ std::declval< TupleArgs >()... } ) >
		>
			: std::true_type {};

		template< typename T, typename Tuple >
		constexpr bool is_constructible_from_tuple_v= is_constructible_from_tuple< T, Tuple >::value;

		template< typename T, typename InitTuple, std::size_t index= 0, typename= std::enable_if_t< std::is_aggregate_v< T > > >
		constexpr auto
		build_base_tuple()
		{
			constexpr auto init_size= aggregate_initializer_size_v< T >;

			using DeeperTuple= decltype( build_init_tuple< T, index, init_size >() );

			if constexpr( is_constructible_from_tuple_v< T, DeeperTuple > )
			{
				return build_base_tuple< T, DeeperTuple, index + 1 >();
			}
			else return Meta::type_value< InitTuple >{};
		}

		template< typename ... Args, typename First >
		constexpr std::size_t
		count_empty_bases( Meta::type_value< std::tuple< First, Args... > > )
		{
			if constexpr( is_empty_base_v< First > ) return 1 + count_empty_bases( Meta::type_value< std::tuple< Args... > >{} );
			else return 0;
		}

		constexpr std::size_t
		count_empty_bases( Meta::type_value< std::tuple<> > )
		{
			return 0;
		}

		template< typename T, std::size_t index= 0, typename= std::enable_if_t< std::is_aggregate_v< T > > >
		constexpr std::size_t
		count_empty_bases()
		{
			return count_empty_bases( build_base_tuple< T, decltype( build_init_tuple< T, 0, aggregate_initializer_size_v< T > > ) >() );
		}

		namespace exports
		{
			template< typename T >
			struct aggregate_empty_bases : std::integral_constant< std::size_t, count_empty_bases< T >() > {};

			template< typename T >
			constexpr std::size_t aggregate_empty_bases_v= aggregate_empty_bases< T >::value;

			template< typename T >
			constexpr std::size_t aggregate_member_count_v= aggregate_initializer_size_v< T > - aggregate_empty_bases_v< T >;

			template< typename T >
			struct aggregate_member_count : std::integral_constant< std::size_t, aggregate_member_count_v< T > > {};
		}
	}

	namespace exports::aggregate_members
	{
		using namespace detail::aggregate_members::exports;
	}
}
