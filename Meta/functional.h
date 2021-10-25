static_assert( __cplusplus > 201700, "C++17 Required" );

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
			template< template< typename, typename > class Function, typename First >
			struct bind1st
			{
				using type= bind1st;
				template< typename Arg >
				struct call : Function< First, Arg >::type {};
			};

			template< template< typename, typename > class Function, typename Second >
			struct bind2nd
			{
				using type= bind2nd;
				template< typename Arg >
				struct call : Function< Arg, Second >::type {};
			};

			template< typename Function, typename ... Args >
			struct call : Function::template call< Args... > {};
		}
	}

	namespace exports::functional
	{
		using namespace detail::functional::exports;
	}
}
