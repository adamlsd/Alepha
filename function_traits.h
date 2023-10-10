static_assert( __cplusplus > 2020'00 );

#pragma once

#include <tuple>
#include <type_traits>
#include <functional>

namespace Alepha::inline Cavorite  ::detail::  function_traits_module
{
	inline namespace exports
	{
		template< typename Function > struct function_traits;

		template< typename Function >
		using get_args_t= typename function_traits< Function >::args_type;

		template< typename Function, int index >
		using get_arg_t= std::decay_t< std::tuple_element_t< index, get_args_t< Function > > >;
	}

	// General catchall...
	template< typename Function >
	struct exports::function_traits
		: exports::function_traits< std::decay_t< decltype( std::function( std::declval< Function >() ) ) > >
	{};

	template< typename Result, typename ... Args, bool noexceptness >
	struct exports::function_traits< std::function< Result ( Args... ) noexcept( noexceptness ) > >
	{
		static constexpr bool noexcepted= noexceptness;
		using function_type= std::function< Result ( Args... ) noexcept( noexceptness ) >;
		using member_type= std::false_type;
		using const_member_type= std::false_type;

		using args_type= std::tuple< Args... >;
		static constexpr std::size_t args_size= sizeof...( Args );

		using return_type= Result;
		using signature_type= Result( Args... );
	};
}

namespace Alepha::Cavorite::inline exports::inline function_traits_module
{
	using namespace detail::function_traits_module::exports;
}
