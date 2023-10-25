static_assert( __cplusplus > 2020'00 );

#pragma once

#include <Alepha/Alepha.h>

#include <ios>
#include <ostream>
#include <optional>

#include <Alepha/StaticValue.h>

#include <Alepha/IOStreams/StreamState.h>

namespace Alepha::Hydrogen::IOStreams  ::detail::  delimiters
{
	inline namespace exports
	{
		enum { FieldDelimiter };
		enum { RecordDelimiter };
	}

	using FieldDelimiterState= StreamState< decltype( FieldDelimiter ), std::string >;

	namespace C
	{
		const std::string defaultFieldDelimiter= "\t";
		const char defaultRecordDelimiter= '\n';
	}

	namespace storage
	{
		inline StaticValue< std::optional< std::string > > globalFieldDelimiter;
		inline StaticValue< std::optional< char > > globalRecordDelimiter;
	}

	inline std::string
	globalFieldDelimiter()
	{
		if( not storage::globalFieldDelimiter().has_value() ) storage::globalFieldDelimiter()= C::defaultFieldDelimiter;
		return storage::globalFieldDelimiter().value();
	}

	inline char
	globalRecordDelimiter()
	{
		if( not storage::globalRecordDelimiter().has_value() ) storage::globalRecordDelimiter()= C::defaultRecordDelimiter;
		return storage::globalRecordDelimiter().value();
	}

	inline std::ostream &
	operator << ( std::ostream &os, decltype( FieldDelimiter ) )
	{
		return os << FieldDelimiterState::get( os );
	}

	namespace exports
	{
		auto
		setFieldDelimiter( const std::string delim )
		{
			return FieldDelimiterState::Setter{ delim };
		}
	}

	inline const int recordIndex= std::ios::xalloc();

	inline void
	setRecordDelimiterOnIOS( std::ios &ios, const char ch )
	{
		ios.iword( recordIndex )= ch;
	}

	inline char
	getRecordDelimiter( std::ios &ios )
	{
		if( ios.iword( recordIndex ) == 0 ) setRecordDelimiterOnIOS( ios, globalRecordDelimiter() );

		return ios.iword( recordIndex );
	}

	inline std::ostream &
	operator << ( std::ostream &os, decltype( RecordDelimiter ) )
	{
		return os << getRecordDelimiter( os );
	}

	struct RecordDelimiterSetter
	{
		const char ch;

		friend std::ostream &
		operator << ( std::ostream &os, const RecordDelimiterSetter &s )
		{
			setRecordDelimiterOnIOS( os, s.ch );
			return os;
		}
	};

	namespace exports
	{
		auto
		setRecordDelimiter( const char ch )
		{
			return RecordDelimiterSetter{ ch };
		}
	}
}

namespace Alepha::Hydrogen::IOStreams::inline exports::inline delimiters
{
	using namespace detail::delimiters::exports;
}
