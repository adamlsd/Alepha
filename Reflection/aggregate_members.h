static_assert( __cplusplus > 201700, "C++17 Required" );

#pragma once

#include <Alepha/Alepha.h>

#include <tuple>
#include <utility>
#include <type_traits>

#include <Alepha/Reflection/detail/config.h>
#include <Alepha/Reflection/aggregate_initialization.h>

#include <Alepha/Meta/overload.h>

namespace Alepha::Hydrogen::Reflection
{
	inline namespace exports { inline namespace aggregate_members {} }

	namespace detail::aggregate_members
	{
		inline namespace exports {}

		using Meta::overload;

		// Basic methodology here: The number of members in an aggregate is equal to the number of initializer parameters
		// it takes less the number of empty base classes it has.

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

		template< typename T, std::size_t index= 0, typename= std::enable_if_t< std::is_aggregate_v< T > > >
		constexpr std::size_t
		count_empty_bases()
		{
			constexpr auto init_size= aggregate_initializer_size_v< T >;

			if constexpr( is_constructible_from_tuple_v< T, decltype( build_init_tuple< T, index, init_size >() ) > )
			{
				return 1 + count_empty_bases< T, index + 1 >();
			}
			else return 0;
		}

		namespace exports
		{
			template< typename T >
			struct aggregate_empty_bases : std::integral_constant< std::size_t, count_empty_bases< T >() - 1 > {};

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
