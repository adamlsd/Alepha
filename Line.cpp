static_assert( __cplusplus > 2020'00 );

#include "Line.h"

namespace Alepha::Cavorite::detail::line
{
	namespace
	{
		namespace C
		{
			const bool debug= false;
		}
	}

	std::string
	exports::getLine( std::istream &is )
	{
		Line rv;
		is >> rv;
		return rv;
	}

	std::istream &
	impl::operator >> ( std::istream &is, Line &line )
	{
		std::getline( is, line );
		if( C::debug ) error() << "Got line: " << line << std::endl;
		return is;
	}
}
