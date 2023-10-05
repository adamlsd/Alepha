static_assert( __cplusplus > 2020'00 );

#pragma once

#include <cstddef>

namespace Alepha::inline Cavorite  ::detail::  word_wrap
{
	inline namespace exports
	{
		std::string wordWrap( const std::string &text, std::size_t width, std::size_t nextLineOffset= 0 );
	}
}
