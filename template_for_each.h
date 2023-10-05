static_assert( __cplusplus > 2020'00 );

#pragma once

#include <type_identity>

#include "Concepts.h"

namespace Alepha::inline Cavorite  ::detail::  template_for_each_module
{
	inline namespace exports
	{
		constexpr void tuple_for_each( const std::tuple<> &, const Functional auto ) noexcept {}

		template< typename ... Args, typename Function >
		constexpr void
		tuple_for_each( const std::tuple< Args... > &tuple, Function body )
		noexcept
		(
			( ... and noexcept( body( std::declval< const Args & >() ) )
		)
		{
			const auto callWrapper= [&body]( auto &&element ) { body( element ); return nullptr; };

			auto loop_body_handler= [&]( auto &&... tuple_elements )
			{
				std::nullptr_t expansion[]= { callWrapper( tuple_elements )... };

				std::ignore= expansion;
			};

			std::apply( loop_body_handler, tuple );
		}

		// Apply type_identity to all tuple elements
		template< typename > struct type_identify_tuple;

		template< typename T >
		using type_identify_tuple_t= typename type_identify_tuple< T >::type;

		template<> struct type_identify_tuple< std::tuple<> > { using type= std::tuple<>; };

		template< typename ... Args >
		struct type_identify_tuple< std::tuple< Args... > >
		{
			using type= std::tuple< std::type_identity< Args >... >;
		};

		// Nicer for-each syntax helper:
		template< typename Tuple >
		struct for_each_syntax_adaptor
		{
			Tuple &tuple;

			template< typename Function >
			constexpr void
			operator <= ( Function &&func ) noexcept( noexcept( tuple_for_each( tuple, std::forward< Function >( func ) ) ) )
			{
				return tuple_for_each( tuple, std::forward< Function >( func ) );
			}

			constexpr operator decltype( std::ignore ) () const= delete;
		};

		template< typename Tuple >
		[[nodiscard]]
		constexpr auto
		tuple_for_each( Tuple &tuple ) noexcept
		{
			return for_each_syntax_adaptor< Tuple >{ tuple };
		}

		template< typename Tuple >
		[[nodiscard]]
		constexpr auto
		tuple_for_each( const Tuple &tuple ) noexcept
		{
			return for_each_syntax_adaptor< const Tuple >{ tuple };
		}
	}
}
