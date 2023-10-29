static_assert( __cplusplus > 2020'00 );

#pragma once

#include <Alepha/Alepha.h>

#include "IStreamable.h"
#include "OStreamable.h"

namespace Alepha::Hydrogen::IOStreams  ::detail:: streamable_m
{
	inline namespace exports
	{
		struct streamable : OStreamable, IStreamable {};
	}
}

namespace Alepha::Hydrogen::IOStreams::inline exports::inline streamable_m
{
	using namespace detail::streamable_m::exports;
}
