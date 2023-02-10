static_assert( __cplusplus > 2020'00 );

#pragma once

#include <Alepha/Alepha.h>

namespace Alepha::Hydrogen::Meta
{
	inline namespace exports { inline namespace template_overload {} }

	namespace detail::template_overload
	{
		inline namespace exports {}

		template< std::size_t >
		struct holder
		{
			struct overload_t;
		};

		namespace exports
		{
			template< std::size_t i >
			using overload= typename holder< i >::overload_t *;
		}
	}

	namespace exports::template_overload
	{
		using namespace detail::template_overload::exports;
	}
}
