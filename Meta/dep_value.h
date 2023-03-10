static_assert( __cplusplus > 2020'00 );

#pragma once

#include <Alepha/Alepha.h>

namespace Alepha::Hydrogen::Meta
{
	inline namespace exports { inline namespace dependent_value {} }

	namespace detail::dependent_value
	{
		inline namespace exports
		{
			template< auto value, typename T >
			constexpr auto dep_value= value;
		}
	}

	namespace exports::dependent_value
	{
		using namespace detail::dependent_value::exports;
	}
}
