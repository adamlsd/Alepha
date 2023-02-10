static_assert( __cplusplus > 2020'00 );

#pragma once

#include <Alepha/Alepha.h>

namespace Alepha::Hydrogen
{
	inline namespace exports { inline namespace types {} }

	namespace detail::types
	{
		inline namespace exports
		{
			using argvec_t= const char *const [];
			using argcnt_t= int;
		}
	}

	namespace exports::types
	{
		using namespace detail::types::exports;
	}
}
