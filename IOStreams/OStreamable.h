static_assert( __cplusplus > 2020'00 );

#pragma once

#include <Alepha/Alepha.h>

#include <ostream>

#include <Alepha/Capabilities.h>
#include <Alepha/template_for_each.h>
#include <Alepha/string_algorithms.h>
#include <Alepha/Concepts.h>

#include <Alepha/Reflection/tuplizeAggregate.h>

namespace Alepha::Hydrogen::IOStreams  ::detail::  ostreamable_module
{
	inline namespace exports
	{
		struct OStreamable {};
	}

	template< typename T >
	concept OStreamableAggregate= Aggregate< T >
		// For now, this part seems to have problems, since the `has_capability` function
		// is either broken... or gcc has a bug which prevents it from functioning
		// correctly.
		//
		// Therefore, we just allow the aggregate case, for the moment.  Which is the
		// most common case, anyhow.
		//and Capability< T, OStreamable >
	;

	template< OStreamableAggregate Agg >
	std::ostream &
	operator << ( std::ostream &os, Agg &ostreamable )
	{
		const auto decomposed= Alepha::Reflection::tuplizeAggregate( ostreamable );
		//static_assert( Capability< Agg, exports::OStreamable > );

		bool first= true;
		// TODO: Consider the lens system here... but the basic use case seems to be for
		// aggregates, so we'll go with this simple case for now...
		tuple_for_each( decomposed ) <=[&]( const auto &element )
		{
			if( not first ) os << '\t';
			first= false;
			os << element;
		};
		os << '\n';

		return os;
	}
}

namespace Alepha::Hydrogen::IOStreams::inline exports::inline ostreamable_module
{
	using namespace detail::ostreamable_module::exports;
}

