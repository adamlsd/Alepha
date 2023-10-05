static_assert( __cplusplus > 2020'00 );

#pragma once

#include <boost/lexical_cast.hpp>

namespace Alepha::inline Cavorite  ::detail:: stringify_function
{
	inline namespace exports
	{
		template< typename T > std::string stringify( const T &t ) { return boost::lexical_cast< std::string >( t ); }
	}
}

namespace Alepha::Cavorite::inline exports::inline stringify_function
{
	using namespace detail::stringify_function::exports;
}
