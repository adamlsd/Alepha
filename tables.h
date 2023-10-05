static_assert( __cplusplus > 2020'00 );

#pragma once

#include <string>

namespace Alepha::inline Cavorite  ::detail::  tables
{
	inline namespace exports
	{
		template< typename StringContainer >
		std::string
		buildColumns( const StringContainer &decomposed )
		{
			std::string result;
			for( const auto &col: decomposed )
			{
				if( not result.empty() ) result+= '\t';
				result+= col;
			}

			return result;
		}

		/*!
		 * Build a string of justified width table output, based upon a specified separator.
		 *
		 * The input string is treated as a table, with several rows.  The specified separator
		 * character (default: `'\t'`) is used to separate columns on each row.  The widest value
		 * in each column is found and used as the width for that column.  The resulting table has
		 * the specified separator character as its separator.
		 *
		 * @param data The string containing columnated data to build a table from.
		 * @param separator The character which separates data within this table.
		 *
		 * @throw std::runtime_error If the string does not contain a rectangular table.
		 *
		 * @pre The `data` string must be column-based data where each row has the same number of fields.
		 */
		std::string buildTable( const std::string &data, const char separator= '\t' );
	}
}

namespace Alepha::Cavorite::inline exports::inline tables
{
	using namespace detail::tables::exports;
}
