static_assert( __cplusplus > 2020'00 );

#pragma once

#include <Alepha/Alepha.h>

#include <cstdint>

#include <iostream>
#include <type_traits>
#include <iomanip>
#include <string>

#include <boost/core/demangle.hpp>
#include <boost/lexical_cast.hpp>

#include <Alepha/TotalOrder.h>
#include <Alepha/Console.h>

#include <Alepha/IOStreams/String.h>

#include <Alepha/Meta/product_type_decay.h>
#include <Alepha/Meta/sequence_kind.h>

#include <Alepha/Meta/is_vector.h>
#include <Alepha/Meta/is_optional.h>
#include <Alepha/Meta/is_streamable.h>
#include <Alepha/Meta/is_sequence.h>
#include <Alepha/Meta/is_product_type.h>

namespace Alepha::Hydrogen::Testing  ::detail::  printDebugging_m
{
	inline namespace exports
	{
		enum class OutputMode { All, Relaxed };

		template< OutputMode outputMode, typename T >
		void printDebugging( const T &witness, const T &expected );
	}

	using namespace std::literals::string_literals;

	template< OutputMode outputMode, typename T >
	std::string
	stringifyValue( const T &v )
	{
		std::ostringstream oss;
		if constexpr( false ) ; // To keep the rest of the clauses regular
		else if constexpr( std::is_same_v< std::uint8_t, std::decay_t< T > > )
		{
			oss << std::hex << std::setw( 2 ) << std::setfill( '0' ) << int( v );
		}
		else if constexpr( std::is_same_v< bool, std::decay_t< T > > )
		{
			oss << std::boolalpha << v;
		}
		else if constexpr( std::is_same_v< std::string, std::decay_t< T > > )
		{
			oss << "(String with " << v.size() << " chars)";
			oss << '\n' << R"(""")" << '\n';
			for( const char ch: v )
			{
				if( ch == '\n' ) oss << "<EOL>\n";
				else if( std::isalnum( ch ) or std::ispunct( ch ) or ( ch == ' ' ) ) oss << ch;
				else oss << "<\\0x" << std::hex << std::setw( 2 ) << std::setfill( '0' ) << unsigned( ch ) << '>';
			}
			oss << '\n' << R"(""")";
		}
		else if constexpr( Meta::is_ostreamable_v< T > )
		{
			return IOStreams::stringify( v );
		}
		else if constexpr( Meta::is_optional_v< T > )
		{
			return v.has_value() ? stringifyValue< outputMode >( v.value() ) : "<noopt>"s;
		}
		else if constexpr( Meta::is_sequence_v< T > )
		{
			if constexpr( outputMode == OutputMode::Relaxed and not Meta::is_ostreamable_v< typename T::value_type > )
			{
				oss << "<Unstreamable sequence of " << v.size() << " elements.>";
			}
			else
			{
				oss << Meta::sequence_kind_v< T > << "(" << v.size() << " elements):\n{" << std::endl;

				int index= 0;
				for( const auto &elem: v ) oss << "\t" << index++ << ":  " << stringifyValue< outputMode >( elem ) << "," << std::endl;
				oss << "}" << std::endl;
			}
		}
		else if constexpr( Meta::is_pair_v< T > )
		{
			const auto &[ first, second ]= v;
			return stringifyValue< outputMode >( std::tie( first, second ) );
		}
		else if constexpr( Meta::is_tuple_v< T > )
		{
			oss << '[';
			tuple_for_each( v ) <=[&oss, first= true]( const auto &elem ) mutable
			{
				if( not first ) oss << ", ";
				first= false;
				oss << std::endl << stringifyValue< outputMode >( elem );
			};
			oss << std::endl << ']' << std::endl;
		}
		else if constexpr( std::is_same_v< T, TotalOrder > )
		{
			if( false ) ; // For alignment
			else if( v == TotalOrder::less ) oss << "less";
			else if ( v == TotalOrder::equal ) oss << "equal";
			else if( v == TotalOrder::greater ) oss << "greater";
			else throw std::logic_error( "Impossible `TotalOrder` condition." );
		}
		else
		{
			static_assert( dependent_value< false, T >, "One of the types used in the testing table does not support stringification." );
		}
		return std::move( oss ).str();
	}

	inline void
	printDebuggingForStrings( const std::string &witness, const std::string &expected )
	{
		const std::size_t amount= std::min( witness.size(), expected.size() );
		if( witness.size() != expected.size() )
		{
			std::cout << "Witness string size did not match the expected string size.  Only mismatches found in the first "
					<< amount << " characters will be printed." << std::endl;
		}

		for( int i= 0; i < amount; ++i )
		{
			if( witness.at( i ) == expected.at( i ) ) continue;
			std::cout << "Mismatch at index: " << i << std::endl;
			std::cout << "witness: " << witness.at( i ) << std::endl;
			std::cout << "expected: " << expected.at( i ) << std::endl;
		}
	}

	template< OutputMode outputMode, typename T >
	void
	exports::printDebugging( const T &witness, const T &expected )
	{
		if constexpr( std::is_same_v< std::string, std::decay_t< T > > )
		{
			printDebuggingForStrings( witness, expected );
		}
		else if constexpr( Meta::is_sequence_v< T > )
		{
			if constexpr( std::is_same_v< std::string, typename T::value_type > )
			{
				if( witness.size() == expected.size() ) for( std::size_t i= 0; i < witness.size(); ++i )
				{
					if( witness.at( i ) != expected.at( i ) ) printDebuggingForStrings( witness.at( i ), expected.at( i ) );
				}
			}
			else
			{
				if( witness.size() != expected.size() )
				{
					std::cout << "Witness sequence size of " << witness.size() << " did not match the expected sequence size of "
							<< expected.size() << std::endl;
				}

				auto next= std::make_pair( begin( witness ), begin( expected ) );
				bool first= true;
				while( next.first != end( witness ) and next.second != end( expected ) )
				{
					if( not first )
					{
						std::cout << "Mismatch at witness index " << std::distance( begin( witness ), next.first ) << " and "
								<< "expected index " << std::distance( begin( expected ), next.second ) << std::endl;
						++next.first; ++next.second;
					}
					first= false;
					next= std::mismatch( next.first, end( witness ), next.second, end( expected ) );
				}
			}
		}

		std::cout << std::endl
				<< "computed: " << stringifyValue< outputMode >( witness ) << std::endl
				<< "expected: " << stringifyValue< outputMode >( expected ) << std::endl << std::endl;
	}

}

namespace Alepha::Hydrogen::Testing::inline exports::inline printDebugging_m
{
	using namespace detail::printDebugging_m::exports;
}
