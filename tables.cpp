static_assert( __cplusplus > 2020'00 );

#include "tables.h"

namespace Alepha::Cavorite  ::detail::  tables
{
	namespace
	{
		namespace C
		{
			const bool debug= false;
			const bool debugSplitting= false or C::debug;
			const bool debugColumnWidth= false or C::debug;
			const bool debugTableResult= false or C::debug;
			const bool debugTableResultWidths= false or C::debug;
		}

		using namespace std::literals::string_literals;

		// Strips leading and trailing whitespace.
		std::string
		stripWhitespace( std::string s )
		{
			std::reverse( begin( s ), end( s ) );
			while( not s.empty() and s.back() == ' ' ) s.pop_back();
			std::reverse( begin( s ), end( s ) );
			while( not s.empty() and s.back() == ' ' ) s.pop_back();
			return s;
		}

		std::vector< std::string >
		internal_split( const std::string &string, const char token )
		{
			std::vector< std::string > rv;

			using namespace boost::algorithm;
			boost::algorithm::split( rv, string, is_any_of( ""s + token ), token_compress_on );
			std::transform( begin( rv ), end( rv ), begin( rv ), stripWhitespace );
			rv.erase( std::remove_if( begin( rv ), end( rv ), std::mem_fn( &std::string::empty ) ), end( rv ) );

			return rv;
		}
	}

	std::string
	exports::buildTable( const std::string &data, const char separator )
	{
		const auto table= evaluate <=[&data, &separator]
		{
			const auto lines= internal_split( data, '\n' );
			std::vector< std::vector< std::string > > rv;
			for( const auto line: lines )
			{
				if( C::debugSplitting ) error() << "Line: '" << line << "'" << std::endl;
				auto lineMembers= internal_split( line, separator );
				if( C::debugSplitting ) for( const auto &element: lineMembers )
				{
					error() << "Element: '" << element << "'" << std::endl;
				}
				rv.push_back( lineMembers );
			}

			return rv;
		};

		if( table.empty() ) return ""s;

		// Verify that all rows are the same width...
		if( std::end( table ) != std::adjacent_find( begin( table ), end( table ),
				[]( const auto &lhs, const auto &rhs )
				{
					return lhs.size() != rhs.size();
				} ) )
		{
			throw std::runtime_error( "Table did not have rows of equal element count" );
		}

		// For each column, find its suitable width.
		const auto widths= evaluate<=[&table]
		{
			if( C::debugColumnWidth ) error() << "Computing maximum width." << std::endl;
			std::vector< std::size_t > rv;
			for( std::size_t i= 0; i < table.front().size(); ++i )
			{
				if( C::debugColumnWidth ) error() << "Computing width for row " << i << std::endl;
				rv.push_back( std::max_element( begin( table ), end( table ),
					[i]( const std::vector< std::string > &lhs, const auto &rhs )
					{
						const std::string &l= lhs.at( i );
						const std::string &r= rhs.at( i );
						if( C::debugColumnWidth )
						{
							error() << "Left size: " << l.size() << " Right size: " << r.size() << std::endl;
							error() << "Left string: '" << l << "' Right string: '" << r << "'" << std::endl;
						}
						return l.size() < r.size();
					} )->at( i ).size() );
				if( C::debugColumnWidth ) error() << "Computed " << rv.back() << " as width for row " << i << std::endl;
			}

			if( C::debugColumnWidth ) error() << "Computed maximum width." << std::endl;
			return rv;
		};

		std::ostringstream oss;
		for( const auto &row: table )
		{
			assert( row.size() == widths.size() );

			if( &row == table.data() ) oss << std::left;
			else oss << std::right << std::endl;
			auto pos= begin( widths );
			for( const auto &element: row )
			{
				if( C::debugTableResultWidths ) error() << "Width: " << *pos << std::endl;
				if( &element != row.data() ) oss << separator;
				oss << std::setw( *pos++ ) << element;
			}
		}

		if( C::debugTableResult ) error() << "Result\n" << oss.str() << "End of result" << std::endl;

		return oss.str();
	}
}
