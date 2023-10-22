static_assert( __cplusplus > 2020'00 );

#pragma once

#include <Alepha/Alepha.h>

#include <istream>

#include <Alepha/Capabilities.h>
#include <Alepha/template_for_each.h>
#include <Alepha/string_algorithms.h>
#include <Alepha/Concepts.h>

#include <Alepha/Reflection/tuplizeAggregate.h>

namespace Alepha::Hydrogen::IOStreams  ::detail::  istreamable_module
{
	inline namespace exports
	{
		struct IStreamable {};
	}

	template< typename T >
	concept IStreamableAggregate= Aggregate< T > and Capability< T, IStreamable >;


	std::istream &
	operator >> ( std::istream &is, IStreamableAggregate auto &istreamable )
	{
		std::string line;
		std::getline( is, line );
		const auto commentChar= line.find( "#" );
		if( commentChar != std::string::npos ) line= line.substr( line.find( "#" ) );

		const auto tokens= split( line, '\t' );

		auto decomposed= Alepha::Reflection::tuplizeAggregate( istreamable );

		if( tokens.size() != std::tuple_size_v< std::decay_t< decltype( decomposed ) > > )
		{
			throw 0;
		}

		int index= 0;
		// TODO: Consider the lens system here... but the basic use case seems to be for
		// aggregates, so we'll go with this simple case for now...
		tuple_for_each( decomposed ) <=[&]( auto &element )
		{
			std::istringstream iss{ tokens.at( index++ ) };
			iss >> element;
		};

		return is;
	}
}

namespace Alepha::Hydrogen::IOStreams::inline exports::inline istreamable_module
{
	using namespace detail::istreamable_module::exports;
}
