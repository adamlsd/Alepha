static_assert( __cplusplus > 2020'00 );

#pragma once

#include <string>
#include <istream>

namespace Alepha::inline Cavorite  ::detail::  line
{
	inline namespace exports
	{
		struct Line;
		std::string getLine( std::istream &is );
	}

	struct Line : std::string
	{
		friend std::istream &operator >> ( std::istream &is, Line &line );

		using std::string::string;
		using std::string::operator=;

		std::string &str() { return *this; }
		const std::string &str() const { return *this; }
	};

	inline namespace impl
	{
		std::istream &operator >> ( std::istream &is, Line &line );
	}
}

namespace Alepha::Cavorite::inline exports::inline line
{
	using namespace detail::line::exports;
}
