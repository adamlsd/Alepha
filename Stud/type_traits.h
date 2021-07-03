static_assert( __cplusplus > 201700, "C++17 Required" );

#pragma once

#include <Alepha/Alepha.h>

#include <type_traits>

namespace Alepha::Hydrogen::Stud
{
	inline namespace exports { inline namespace type_traits {} }

	namespace detail::type_traits
	{
		inline namespace exports
		{
			inline namespace
			template< typename T >
			struct type_identity
			{
				using type= T;
			};

			template< typename T >
			using type_identity_t= typename type_identity< T >::type;

			using std::remove_const;
			using std::remove_const_t;

			using std::decay;
			using std::decay_t;

			using std::is_same;
			using std::is_same_v;

			using std::enable_if;
			using std::enable_if_t;
		}
	}

	namespace exports::type_traits
	{
		using namespace detail::type_traits::exports;
	}
}
