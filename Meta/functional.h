static_assert( __cplusplus > 2020'00 );

#pragma once

// Analogue to `#include <functional>`

#include <Alepha/Alepha.h>

namespace Alepha::Hydrogen::Meta
{
	inline namespace exports { inline namespace functional {} }

	namespace detail::functional
	{
		inline namespace exports
		{
			template< typename MetaFunction, typename Arg >
			constexpr auto bind1st( MetaFunction func, Arg arg );
		}

		template< typename MetaFunction, typename Arg > struct binder1st;

		template< typename MetaFunction, typename Arg1, typename Arg2 >
		constexpr decltype( auto )
		invoke_call( MetaFunction func, Meta::type_value< Arg1 > arg1, Meta::type_value< Arg2 > arg2 )
		{
			return func( arg1, arg2 );
		}

		template< typename MetaFunction, typename Arg >
		struct binder1st< MetaFunction, Meta::type_value< Arg > >
		{
			MetaFunction func;
			Meta::type_value< Arg > arg;

			template< typename Second >
			constexpr decltype( auto )
			operator () ( const Second &second )
			{
				return invoke_call( func, arg, second );
			}
		};

		template< typename MetaFunction, typename Arg >
		constexpr auto
		exports::bind1st( MetaFunction func, Arg arg )
		{
			return binder1st< MetaFunction, Arg >{ func, arg };
		}
	}

	namespace exports::functional
	{
		using namespace detail::functional::exports;
	}
}
