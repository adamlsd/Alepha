static_assert( __cplusplus > 2020'00 );

#pragma once

#include <Alepha/Alepha.h>

#include <Alepha/Capabilities.h>
#include <Alepha/comparisons.h>

#include <Alepha/Reflection/tuplizeAggregate.h>

namespace Alepha::Hydrogen  ::detail::  auto_comparable_module
{
	inline namespace exports
	{
		struct auto_comparable : comparable {};
	}

	template< typename T >
	concept AutoComparableAggregate=
		Capability< T, auto_comparable >
			and
		Aggregate< T >;

	constexpr decltype( auto )
	equality_lens( const AutoComparableAggregate auto &item ) noexcept
	{
		return detail::comparisons::magma_hook{ Reflection::tuplizeAggregate( item ) };
	}
}

namespace Alepha::Hydrogen::inline exports::inline auto_comparable_module
{
	using namespace detail::auto_comparable_module::exports;
}
