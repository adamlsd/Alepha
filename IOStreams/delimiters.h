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
		std::string current;
		StreamState state;

		explicit
		Delimiter( const std::string current )
			: current( current )
		{
		}
	};

	template< ConstexprString s >
	using CreateDelimiter= void ( const DelimiterParamType< s > & );

	template< typename T >
	constexpr is_delimiter_v= false;

	template< ConstexprString s >
	constexpr is_delimiter_v< CreateDelimiter< s > >;

	inline StaticValue< std::map< void (*)( const DelimiterBase & ), StreamState< Delim, std::string, globalDelimiter< 
	

	template< typename Delimiter >
	using DelimiterStateByValue= StreamState< Delim, std::string, globalDelimiter< Delim > >;

	inline namespace exports
	{
		Delimiter< "\t"_cs > FieldDelimiter;
		Delimiter< "\t"_cs > FieldDelimiter2;

		static_assert( FieldDelimiter != FieldDelimiter2 );

		Delimiter< "\n"_cs > RecordDelimiter;
	}

	namespace storage
	{
		template< auto Delim >
		inline StaticValue< std::optional< std::string > > globalDelimiter;
	}

	template< auto Delim >
	std::string &
	globalDelimiter()
	{
		if( not storage::globalDelimiter< Delim >().has_value() )
		{
		
			storage::globalDelimiter()= std::string{} + static_cast< char >( Delim );
		}
		assert( storage::globalDelimiter< Delim >().has_value() );
		return storage::globalFieldDelimiter().value();
	}

	namespace exports
	{
		template< auto Delim >
		void
		setGlobaDelimiter( const std::string delim )
		{
			storage::globalDelimiter< Delim >()= delim;
		}
	}

	template< auto Delim >
	using DelimiterStateByValue= StreamState< Delim, std::string, globalDelimiter< Delim > >;

	

	template< typename DelimType >
	using DelimiterState= StreamState< Delim, std::string, globalDelimiter >;


	struct DelimWrap
	{	
		template< typename Delim >
		Delim val;
	};

	std::ostream &
	operator << ( std::ostream &os, auto DelimVal )
	{
		return os << DelimiterState< decltype( DelimVal ) >::get( os );
	}

	namespace exports
	{
		auto
		setFieldDelimiter( const std::string delim )
		{
			return FieldDelimiterState::Setter{ delim };
		}

		const auto &
		getFieldDelimiter( std::ios_base &ios )
		{
			return FieldDelimiterState::get( ios );
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

namespace Alepha::Hydrogen::IOStreams::inline exports::inline delimiters_m
{
	using namespace detail::delimiters_m::exports;
}
