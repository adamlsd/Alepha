static_assert( __cplusplus > 201700, "C++17 Required" );

#pragma once

#include <Alepha/Alepha.h>

#include <Alepha/Meta/find.h>
#include <Alepha/Stud/type_traits.h>

namespace Alepha::Hydrogen
{
	inline namespace exports { inline namespace capabilities {} }

	namespace detail::capabilities
	{
		inline namespace exports
		{
			template< typename ... capabilities >
			struct Capabilities;

			template< typename T, typename cap >
			struct has_capability; //: std::is_base_of< cap, T > {};


#if 0
			template< template< typename ... > class Type, typename ... Caps, typename cap >
			struct has_capability< Type< Capabilities< Caps... > >, cap >
			{
				using T= Type< Capabilities< Caps... > >;
				static constexpr bool value= Meta::find_in_tuple_v< cap, std::tuple< Caps... > > or std::is_base_of_v< cap, T >;
				using type= has_capability;
			};
			template< template< typename ... > class Type, typename ... Back, typename ... Caps, typename cap >
			struct has_capability< Type< Capabilities< Caps... >, Back... >, cap >
			{
				using T= Type< Capabilities< Caps... >, Back... >;
				static constexpr bool value= Meta::find_in_tuple_v< cap, std::tuple< Caps... > > or std::is_base_of_v< cap, T >;
				using type= has_capability;
			};


			template< template< typename ... > class Type, typename ... Front, typename ... Caps, typename cap >
			struct has_capability< Type< Front..., Capabilities< Caps... > >, cap >
			{
				using T= Type< Front..., Capabilities< Caps... > >;
				static constexpr bool value= Meta::find_in_tuple_v< cap, std::tuple< Caps... > > or std::is_base_of_v< cap, T >;
				using type= has_capability;
			};
#endif
		}

		template< typename T >
		struct is_capability_list : std::false_type {};

		template< typename ... Args >
		struct is_capability_list< Capabilities< Args... > > : std::true_type {};

		template< typename T >
		constexpr bool is_capability_list_v= is_capability_list< T >::value;

		template< template< typename ... > class ... HigherKinds >
		struct higher_kind_tuple {};

		template< typename cap, typename ... Caps >
		constexpr auto
		has_cap( const Stud::type_identity< Capabilities< Caps... > > & )
		{
			return Meta::find_if< Meta::bind1st< std::is_base_of, cap >, Meta::list< Caps... > >{};
		}

		template< typename cap >
		constexpr std::false_type has_cap( const Meta::list<> & ) { return {}; }

		template< typename cap, typename First, typename ... TParams >
		constexpr auto
		has_cap( const Meta::list< First, TParams... > & )
		{
			using depth_type= decltype( has_cap< cap >( Meta::list< TParams... >{} ) );
			if constexpr( is_capability_list_v< First > )
			{
				using bool_type= decltype( has_cap< cap >( Stud::type_identity< First >() ) );
				if constexpr( bool_type::value )
				{
					return std::bool_constant< bool_type::value >{};
				}
				else return depth_type{};
			}
			else return depth_type{};
		}

		template< typename cap, template< typename ... > class Class, typename ... TParams >
		constexpr auto
		has_cap( const Class< TParams... > & )
		{
			return has_cap< cap >( Meta::list< TParams... >{} );
		}

		namespace exports
		{
			template< typename T, typename cap >
			constexpr bool has_capability_v= std::is_base_of_v< cap, T > or decltype( has_cap< cap >( std::declval< T >() ) )::value;

			template< typename T, typename cap >
			struct has_capability : std::bool_constant< has_capability_v< T, cap > > {};
		}
	}

	namespace exports::capabilities
	{
		using namespace detail::capabilities::exports;
	}
}
