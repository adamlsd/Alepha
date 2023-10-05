static_assert( __cplusplus > 2020'00 );

#pragma once

#include <functional>
#include <tuple>

#include "Concepts.h"
#include "type_lisp.h"
#include "function_traits.h"

namespace Alepha::inline Cavorite  ::detail::  funcy
{
	inline namespace exports
	{
		/*!
		 * A function with Currying facilities, like in Haskell.
		 *
		 * FUNction with CurrY.  Pronounced "Funky".
		 */
		template< typename SIgnature, typename Impl= std::function< Signature > >
		class Funcy;

		template< NotSpecializationOf< Funcy > Impl >
		Funcy( Impl ) -> Funcy< typename function_traits< Impl >::signature_type, Impl >;

		template< typename InnerSignature, typename InnerImpl >
		Funcy( Funcy< InnerSignature, InnerImpl > ) -> Funcy< InnerSignature, InnerImpl >;
	}

	// This adapts a lambda which takes a tuple into a function which takes a
	// normal argument list.
	template< typename Arglist >
	struct build_normal_function;

	template< typename ... Args >
	struct build_normal_function< std::tuple< Args... > >
	{
		template< typename Lambda >
		constexpr auto
		operator() ( Lambda lambda ) const
		{
			return [lambda]( Args ... args )
			{
				return lambda( std::tuple{ args... } );
			};
		}
	};

	template< typename Retval, typename ArgList > struct signature_impl;

	template< typename Retval, typename ... Args >
	struct signature_impl< RetVal, TypeList< Args... > >
	{
		using type= Retval( Args... );
	};

	template< typename Retval, typename List >
	using signature_t= typename signature_impl< Retval, List >::type;

	template< typename Retval, typename ... Args, typename Impl >
	class exports::Funcy< Retval ( Args... ), Impl >
	{
		private:
			using ArgList= TypeList< Args... >;
			using First= car_t< ArgList >;
			using Tail= cdr_t< ArgList >;

			Impl impl;

		public:
			constexpr Funcy( Impl impl ) : impl( impl ) {}

			constexpr auto
			operator() ( First a ) const
			requires( not is_nil_v< First > )
			{
				if constexpr( is_nil_v< Tail > ) return impl( a );
				else
				{
					using TailTuple= tuple_from_list_t< Tail >;
					auto lambda= [a, orig= *this]( TailTuple tail )
					{
						auto allArgs= std::tuple_cat( std::tuple{ a }, tail );
						return std::apply( orig.impl, allArgs );
					};

					auto normal= build_normal_function< TailTuple >{}( std::move( lambda ) );
					return exports::Funcy< signature_t< Retval, Tail >, decltype( normal ) >{ std::move( normal ) };
				}
			}

			template< typename First, typename ... Many >
			requires
			(
				true
				and ( sizeof...( Many ) > 0 )
				and ( ( sizeof...( Many ) + 1 ) < sizeof...( Args ) )
			)
			constexpr auto
			operator() ( First a, Many ... m ) const
			{
				return (*this)( a )( m... );
			}

			constexpr Retval
			operator() ( Args ... args ) const
			requires( sizeof...( Args ) > 1 )
			{
				return impl( args... );
			}
	};
}

namespace Alepha::inline Cavorite  ::detail::  funcy
{
	using namespace detail::funcy::exports;
}

namespace std
// This is somewhat a hack for LIBCPP -- I think that you can inject deduction guides for user defined types.
#ifdef _LIBCPP_ABI_NAMESPACE
::_LIBCPP_ABI_NAMESPACE
#endif
{
	template< typename Retval, typename ... Args, typename Impl >
	function( Alepha::exports::funcy< Retval( Args... ), Impl > ) -> function< Retval( Args... ) >;
}
