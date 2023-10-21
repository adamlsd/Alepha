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

	struct nil;

	template< bool noexceptness, typename ClassHolder, typename ReturnType, typename ... Args >
	struct traits : traits< noexceptness, nil, ReturnType, Args... >
	{
		using member_type= ReturnType (ClassHolder::*)( Args... );
		using const_member_type= ReturnType (ClassHolder::*)( Args... ) const;
	};

	template< bool noexceptness, typename ReturnType, typename ... Args >
	struct traits< noexceptness, nil, ReturnType, Args... >
	{
		static constexpr bool noexcepted= noexceptness;

		using function_type= std::function< ReturnType ( Args... ) noexcept( noexcepted ) >;

		using member_type= nil;
		using const_member_type= nil;

		using args_type= std::tuple< Args... >;
		static constexpr std::size_t args_size= sizeof...( Args );

		using return_type= ReturnType;
		using signature_type= ReturnType( Args... );
	};
		

	// General catchall...
	template< typename Function >
	struct exports::function_traits
		: exports::function_traits< std::decay_t< decltype( std::function( std::declval< Function >() ) ) > >
	{};


	template< typename ReturnType, typename ... Args, bool noexceptness >
	struct exports::function_traits< ReturnType ( Args... ) noexcept( noexceptness ) >
		: traits< noexceptness, nil, ReturnType, Args... >
	{};

	template< typename ReturnType, typename ... Args, bool noexceptness >
	struct exports::function_traits< ReturnType (*)( Args... ) noexcept( noexceptness ) >
		: traits< noexceptness, nil, ReturnType, Args... >
	{};

	template< typename ReturnType, typename ... Args, bool noexceptness >
	struct exports::function_traits< ReturnType (&)( Args... ) noexcept( noexceptness ) >
		: traits< noexceptness, nil, ReturnType, Args... >
	{};


	// The `std::function` form...  It also is the catchall for other types...
	template< typename ReturnType, typename ... Args, bool noexceptness >
	struct exports::function_traits< std::function< ReturnType ( Args... ) noexcept( noexceptness ) > >
		: traits< noexceptness, nil, ReturnType, Args... >
	{};
}

namespace Alepha::Cavorite::inline exports::inline function_traits_module
{
	using namespace detail::function_traits_module::exports;
}
