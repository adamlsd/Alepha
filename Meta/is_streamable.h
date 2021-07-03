static_assert( __cplusplus > 201700, "C++17 Required" );

#pragma once

#include <Alepha/Alepha.h>

#include <iosfwd>

namespace Alepha::Hydrogen::Meta
{
	inline namespace exports { inline namespace type_traits {} }

	namespace detail::type_traits::is_streamable
	{
		inline namespace exports {}

		namespace impl
		{
			template< typename T, typename= void >
			struct is_istreamable : std::false_type {};

			template< typename T >
			struct is_istreamable< T, std::void_t< decltype( std::declval< std::istream & >() >> std::declval< T & >() ) > > : std::true_type {};

			template< typename T, typename= void >
			struct is_ostreamable : std::false_type {};

			template< typename T >
			struct is_ostreamable< T, std::void_t< decltype( std::declval< std::ostream & >() << std::declval< T & >() ) > > : std::true_type {};
		}

		namespace exports
		{
			template< typename T >
			constexpr bool is_istreamable_v= impl::is_istreamable< T >::value;

			template< typename T >
			constexpr bool is_ostreamable_v= impl::is_ostreamable< T >::value;

			template< typename T >
			constexpr bool is_streamable_v= is_istreamable_v< T > and is_ostreamable_v< T >;

			template< typename T >
			struct is_istreamable : std::bool_constant< is_istreamable_v< T > > {};

			template< typename T >
			struct is_ostreamable : std::bool_constant< is_ostreamable_v< T > > {};

			template< typename T >
			struct is_streamable : std::bool_constant< is_streamable_v< T > > {};
		}
	}

	namespace exports::type_traits
	{
		using namespace detail::type_traits::is_streamable::exports;
	}
}
