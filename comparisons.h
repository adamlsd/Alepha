static_assert( __cplusplus > 2020'00 );

#pragma once

#include <Alepha/Alepha.h>

#include <type_traits>

#include <Alepha/Meta/dep_value.h>

#include <Alepha/Capabilities.h>

#include <Alepha/Meta/overload.h>


namespace Alepha::Hydrogen
{
	inline namespace exports { inline namespace comparisons {} }

	namespace detail::comparisons
	{
		inline namespace exports {}

		using namespace Meta::exports::template_overload;

		struct comparable {};

		namespace exports
		{
			using detail::comparisons::comparable;
			inline constexpr Meta::type_value< comparable > comparable_capability;
		}

		template< typename T >
		constexpr bool has_comparable_capability_v= has_capability( Meta::type_value< std::decay_t< T > >{}, comparable_capability );

		template< typename T >
		struct has_comparable_capability_s : std::bool_constant< has_comparable_capability_v< T > > {};

		inline constexpr Meta::trait< has_comparable_capability_s > has_comparable_capability;


		// Spaceship lens support

		template< typename T, typename= void >
		struct has_spaceship_lens_member : std::false_type {};

		template< typename T >
		struct has_spaceship_lens_member< T, std::void_t< decltype( std::declval< const T & >().spaceship_lens() ) > > : std::true_type {};

		template< typename T >
		constexpr bool has_spaceship_lens_member_v= has_spaceship_lens_member< T >::value;

		template
		<
			typename T,
			typename= std::enable_if_t< has_comparable_capability( Meta::type_value< T >{} ) >,
			typename= std::enable_if_t< has_spaceship_lens_member_v< T > >,
			overload< __LINE__ > = nullptr
		>
		constexpr decltype( auto )
		spaceship_lens( T &t )
		{
			return t.spaceship_lens();
		}

		template< typename T, typename= void >
		struct supports_spaceship_lens : std::false_type {};

		template< typename T >
		struct supports_spaceship_lens< T, std::void_t< decltype( spaceship_lens( std::declval< const T & >() ) ) > > : std::true_type {};

		template< typename T >
		constexpr bool supports_spaceship_lens_v= supports_spaceship_lens< T >::value;

		template< typename T >
		constexpr decltype( auto )
		make_spaceship_lens( T &t )
		{
			if constexpr( supports_spaceship_lens_v< T > ) return spaceship_lens( t );
			//else if constexpr( supports_default_lens_v< T > ) return default_lens( t );
			else static_assert( Meta::dep_value< false, T > );
		}

		// Value lens support

		template< typename T, typename= void >
		struct has_value_lens_member : std::false_type {};

		template< typename T >
		struct has_value_lens_member< T, std::void_t< decltype( std::declval< const T & >().value_lens() ) > > : std::true_type {};

		template< typename T >
		constexpr bool has_value_lens_member_v= has_value_lens_member< T >::value;

		template
		<
			typename T,
			typename= std::enable_if_t< has_comparable_capability_v< T > >,
			typename= std::enable_if_t< has_value_lens_member_v< T > >,
			overload< __LINE__ > = nullptr
		>
		constexpr decltype( auto )
		value_lens( T &t )
		{
			return t.value_lens();
		}

		template
		<
			typename T,
			typename= std::enable_if_t< has_comparable_capability_v< T > >,
			typename= std::enable_if_t< supports_spaceship_lens_v< T > >,
			overload< __LINE__ > = nullptr
		>
		constexpr decltype( auto )
		value_lens( T &t )
		{
			return spaceship_lens( t );
		}

		template< typename T, typename= void >
		struct supports_value_lens : std::false_type {};

		template< typename T >
		struct supports_value_lens< T, std::void_t< decltype( value_lens( std::declval< const T & >() ) ) > > : std::true_type {};

		template< typename T >
		constexpr bool supports_value_lens_v= supports_value_lens< T >::value;

		template< typename T >
		constexpr decltype( auto )
		make_value_lens( T &t )
		{
			if constexpr( supports_value_lens_v< T > ) return value_lens( t );
			//else if constexpr( supports_default_lens_v< T > ) return default_lens( t );
			else static_assert( Meta::dep_value< false, T > );
		}

		// Equality Lens support

		template< typename T, typename= void >
		struct has_equality_lens_member : std::false_type {};

		template< typename T >
		struct has_equality_lens_member< T, std::void_t< decltype( std::declval< const T & >().equality_lens() ) > > : std::true_type {};

		template< typename T >
		constexpr bool has_equality_lens_member_v= has_equality_lens_member< T >::value;

		template
		<
			typename T,
			typename= std::enable_if_t< has_comparable_capability_v< T > >,
			typename= std::enable_if_t< has_equality_lens_member_v< T > >,
			overload< __LINE__ > = nullptr
		>
		constexpr decltype( auto )
		equality_lens( T &t )
		{
			return t.equality_lens();
		}

		template
		<
			typename T,
			typename= std::enable_if_t< has_comparable_capability_v< T > >,
			typename= std::enable_if_t< supports_value_lens_v< T > >,
			overload< __LINE__ > = nullptr
		>
		constexpr decltype( auto )
		equality_lens( T &t )
		{
			return value_lens( t );
		}

		template< typename T, typename= void >
		struct supports_equality_lens : std::false_type {};

		template< typename T >
		struct supports_equality_lens< T, std::void_t< decltype( equality_lens( std::declval< const T & >() ) ) > > : std::true_type {};

		template< typename T >
		constexpr bool supports_equality_lens_v= supports_equality_lens< T >::value;

		template< typename T >
		constexpr decltype( auto )
		make_equality_lens( T &t )
		{
			if constexpr( supports_equality_lens_v< T > ) return equality_lens( t );
			//else if constexpr( supports_default_lens_v< T > ) return default_lens( t );
			else static_assert( Meta::dep_value< false, T > );
		}

		// Strict weak order lens support

		template< typename T, typename= void >
		struct has_strict_weak_order_lens_member : std::false_type {};

		template< typename T >
		struct has_strict_weak_order_lens_member< T, std::void_t< decltype( std::declval< const T & >().strict_weak_order_lens() ) > > : std::true_type {};

		template< typename T >
		constexpr bool has_strict_weak_order_lens_member_v= has_strict_weak_order_lens_member< T >::value;

		template
		<
			typename T,
			typename= std::enable_if_t< has_comparable_capability_v< T > >,
			typename= std::enable_if_t< has_strict_weak_order_lens_member_v< T > >,
			overload< __LINE__ > = nullptr
		>
		constexpr decltype( auto )
		strict_weak_order_lens( T &t )
		{
			return t.strict_weak_order_lens();
		}

		template
		<
			typename T,
			typename= std::enable_if_t< has_comparable_capability_v< T > >,
			typename= std::enable_if_t< supports_value_lens_v< T > >,
			overload< __LINE__ > = nullptr
		>
		constexpr decltype( auto )
		strict_weak_order_lens( T &t )
		{
			return value_lens( t );
		}

		template< typename T, typename= void >
		struct supports_strict_weak_order_lens : std::false_type {};

		template< typename T >
		struct supports_strict_weak_order_lens< T, std::void_t< decltype( strict_weak_order_lens( std::declval< const T & >() ) ) > > : std::true_type {};

		template< typename T >
		constexpr bool supports_strict_weak_order_lens_v= supports_strict_weak_order_lens< T >::value;

		template< typename T >
		constexpr decltype( auto )
		make_strict_weak_order_lens( T &t )
		{
			if constexpr( supports_strict_weak_order_lens_v< T > ) return strict_weak_order_lens( t );
			//else if constexpr( supports_default_lens_v< T > ) return default_lens( t );
			else static_assert( Meta::dep_value< false, T > );
		}

		// Operator support:
		template
		<
			typename T,
			typename= std::enable_if_t< has_comparable_capability_v< T > >,
			overload< __LINE__ > = nullptr
		>
		constexpr bool
		operator == ( const T &lhs, const T &rhs )
		{
			return make_equality_lens( lhs ) == make_equality_lens( rhs );
		}

		template
		<
			typename T,
			typename= std::enable_if_t< has_comparable_capability_v< T > >,
			overload< __LINE__ > = nullptr
		>
		constexpr bool
		operator != ( const T &lhs, const T &rhs )
		{
			return make_equality_lens( lhs ) != make_equality_lens( rhs );
		}

		template
		<
			typename T,
			typename= std::enable_if_t< has_comparable_capability_v< T > >,
			overload< __LINE__ > = nullptr
		>
		constexpr bool
		operator < ( const T &lhs, const T &rhs )
		{
			return make_strict_weak_order_lens( lhs ) < make_strict_weak_order_lens( rhs );
		}

		template
		<
			typename T,
			typename= std::enable_if_t< has_comparable_capability_v< T > >,
			overload< __LINE__ > = nullptr
		>
		constexpr bool
		operator > ( const T &lhs, const T &rhs )
		{
			return make_strict_weak_order_lens( lhs ) > make_strict_weak_order_lens( rhs );
		}

		template
		<
			typename T,
			typename= std::enable_if_t< has_comparable_capability_v< T > >,
			overload< __LINE__ > = nullptr
		>
		constexpr bool
		operator <= ( const T &lhs, const T &rhs )
		{
			return make_strict_weak_order_lens( lhs ) <= make_strict_weak_order_lens( rhs );
		}

		template
		<
			typename T,
			typename= std::enable_if_t< has_comparable_capability_v< T > >,
			overload< __LINE__ > = nullptr
		>
		constexpr bool
		operator >= ( const T &lhs, const T &rhs )
		{
			return make_strict_weak_order_lens( lhs ) >= make_strict_weak_order_lens( rhs );
		}

		template< typename ... Args >
		struct magma_hook
		{
			std::tuple< Args... > view;
		};

		namespace exports
		{
			template< typename ... Args >
			constexpr auto
			ordering_magma( Args && ... args )
			{
				return magma_hook< Args... >{ std::tie( std::forward< Args >( args )... ) };
			}
		}

		// TODO: Sort out the linear-trichotomous problem.

		template< typename comp, std::size_t index= 0, typename ... Args >
		constexpr bool
		compOp( const std::tuple< Args... > &lhs, const std::tuple< Args... > &rhs )
		{
			if constexpr( index == sizeof...( Args ) ) return false;
			else
			{
				const auto &l= std::get< index >( lhs );
				const auto &r= std::get< index >( rhs );
				if( comp{}( l, r ) ) return true;
				else if( comp{}( r, l ) ) return false;
				return compOp< comp, index + 1 >( lhs, rhs );
			}
		}

		template< typename ... Args >
		constexpr bool
		operator < ( const magma_hook< Args... > &lhs, const magma_hook< Args... > &rhs )
		{
			return compOp< std::less<> >( lhs.view, rhs.view );
		}

		template< typename ... Args >
		constexpr bool
		operator > ( const magma_hook< Args... > &lhs, const magma_hook< Args... > &rhs )
		{
			return compOp< std::greater<> >( lhs.view, rhs.view );
		}


		template< typename comp, std::size_t index= 0, typename ... Args >
		constexpr bool
		comp_eqOp( const std::tuple< Args... > &lhs, const std::tuple< Args... > &rhs )
		{
			if constexpr( index == sizeof...( Args ) ) return true;
			else
			{
				const auto &l= std::get< index >( lhs );
				const auto &r= std::get< index >( rhs );
				const bool first_pass= comp{}( l, r );
				if( first_pass and comp{}( r, l ) ) return comp_eqOp< comp, index + 1 >( lhs, rhs );
				return first_pass;
			}
		}

		template< typename ... Args >
		constexpr bool
		operator <= ( const magma_hook< Args... > &lhs, const magma_hook< Args... > &rhs )
		{
			return comp_eqOp< std::less_equal<> >( lhs.view, rhs.view );
		}

		template< typename ... Args >
		constexpr bool
		operator >= ( const magma_hook< Args... > &lhs, const magma_hook< Args... > &rhs )
		{
			return comp_eqOp< std::greater_equal<> >( lhs.view, rhs.view );
		}

		template< std::size_t index= 0, typename ... Args >
		constexpr bool
		eq( const std::tuple< Args... > &lhs, const std::tuple< Args... > &rhs )
		{
			if constexpr( index == sizeof...( Args ) ) return true;
			else
			{
				const auto &l= std::get< index >( lhs );
				const auto &r= std::get< index >( rhs );
				return ( l == r ) and eq< index + 1 >( lhs, rhs );
			}
		}

		template< typename ... Args >
		constexpr bool
		operator == ( const magma_hook< Args... > &lhs, const magma_hook< Args... > &rhs )
		{
			return eq( lhs.view, rhs.view );
		}

		template< std::size_t index= 0, typename ... Args >
		constexpr bool
		ne( const std::tuple< Args... > &lhs, const std::tuple< Args... > &rhs )
		{
			if constexpr( index == sizeof...( Args ) ) return false;
			else
			{
				const auto &l= std::get< index >( lhs );
				const auto &r= std::get< index >( rhs );
				return l != r and ne< index + 1 >( lhs, rhs );
			}
		}

		template< typename ... Args >
		constexpr bool
		operator != ( const magma_hook< Args... > &lhs, const magma_hook< Args... > &rhs )
		{
			return ne( lhs, rhs );
		}
	}

	namespace exports::comparisons
	{
		using namespace detail::comparisons::exports;
	}
}
