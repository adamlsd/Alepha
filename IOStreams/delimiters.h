static_assert( __cplusplus > 2020'00 );

#pragma once

#include <Alepha/Alepha.h>

#include <ios>
#include <ostream>
#include <optional>

#include <Alepha/StaticValue.h>

#include <Alepha/IOStreams/StreamState.h>

namespace Alepha::Hydrogen::IOStreams  ::detail::  delimiters_m
{
	inline namespace exports {}

	// Syntax I want to work:
	//
	// std::cout << someDelimiter;
	// auto value= getDelimiter( someDelimiter, std::cin );
	// std::cout << setDelimiter( someDelimiter, value );

	struct Delimiter : boost::noncopyable
	{
		StreamState< std::string > state;

		explicit
		Delimiter( const std::string dflt )
			: state( [dflt] { return dflt; } ) {}
	};

	namespace exports
	{
		inline Delimiter fieldDelimiter{ "\t" };
		inline Delimiter recordDelimiter{ "\n" };
	}

	inline std::ostream &
	operator << ( std::ostream &os, Delimiter &delim )
	{
		const auto &s= delim.state.get( os );
		return os << s;
	}

	namespace exports
	{
		inline std::string
		getDelimiter( Delimiter &delim, std::ios_base &ios )
		{
			return delim.state.get( ios );
		}

		inline auto
		setDelimiter( Delimiter &delim, const std::string s )
		{
			return delim.state.makeSetter( s );
		}

		inline void
		setGlobalDelimiter( Delimiter &delim, const std::string s )
		{
			return delim.state.setDefault( s );
		}
	}
}

namespace Alepha::Hydrogen::IOStreams::inline exports::inline delimiters_m
{
	using namespace detail::delimiters_m::exports;
}
