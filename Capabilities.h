static_assert( __cplusplus > 201700, "C++17 Required" );

#pragma once

#include <Alepha/Alepha.h>

#include <Alepha/Meta/find.h>
#include <Alepha/Meta/type_value.h>

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
			struct has_capability_s; //: std::is_base_of< cap, T > {};


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
		struct is_capability_list_s : std::false_type {};

		template< typename ... Args >
		struct is_capability_list_s< Capabilities< Args... > > : std::true_type {};

		inline constexpr Meta::trait< is_capability_list_s > is_capability_list;

		template< typename T >
		constexpr bool is_capability_list_v= is_capability_list( Meta::type_value< T >{} );
		

		template< template< typename ... > class ... HigherKinds >
		struct higher_kind_tuple {};

		template< typename Cap, typename ... Caps >
		constexpr bool
		has_cap_in_capability_base( const Meta::type_value< Capabilities< Caps... > > &, Meta::type_value< Cap > cap )
		{
			Meta::Container::vector< Caps... > types;
			using std::begin, std::end;
			return Meta::find_if( begin( types ), end( types ), Meta::bind1st( Meta::is_base_of, cap ) );
		}

		template< typename Left, typename Cap >
		constexpr bool
		has_cap_in_capability_base( const Left &, Meta::type_value< Cap > cap )
		{
			throw "Unevaluated";
		}

		template< typename Cap, typename ... TParams >
		constexpr bool
		has_cap( const Meta::Container::vector< TParams... > &types, Meta::type_value< Cap > cap )
		{
			bool rv= 0;
			template_for( types ) <=[&]
			( const auto type )
			{
				if( is_capability_list( type ) and has_cap_in_capability_base( type, cap ) ) rv= true;
			};
			return rv;
		}

		template< typename Cap, template< typename ... > class Class, typename ... TParams >
		constexpr bool
		has_cap( const Meta::type_value< Class< TParams... > > &, Meta::type_value< Cap > cap )
		{
			return has_cap( Meta::Container::vector< TParams... >{}, cap );
		}

		namespace exports
		{
			template< typename T, typename cap >
			constexpr bool has_capability_v=
					std::is_base_of_v< cap, T >
						or
					has_cap( Meta::type_value< T >{}, Meta::type_value< cap >{} );

			template< typename T, typename cap >
			struct has_capability_s : std::bool_constant< has_capability_v< T, cap > > {};

			inline constexpr Meta::trait< has_capability_s > has_capability;
		}
	}

	namespace exports::capabilities
	{
		using namespace detail::capabilities::exports;
	}
}
