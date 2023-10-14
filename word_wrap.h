static_assert( __cplusplus > 2020'00 );

#pragma once

#include <cstddef>

#include <string>
#include <streambuf>

namespace Alepha::inline Cavorite  ::detail::  word_wrap
{
	inline namespace exports
	{
		std::string wordWrap( const std::string &text, std::size_t width, std::size_t nextLineOffset= 0 );

		struct StartWrap
		{
			std::size_t width;
			std::size_t nextLineOffset;
			
			explicit StartWrap( const std::size_t width, const std::size_t nextLineOffset= 0 ) : width( width ), nextLineOffset( nextLineOffset ) {}
		};

		constexpr struct EndWrap_t {} EndWrap;
	}

	inline namespace impl
	{
		std::ostream &operator << ( std::ostream &, StartWrap );
		std::ostream &operator << ( std::ostream &, EndWrap_t );
	}
}

namespace Alepha::Cavorite::inline exports::inline word_wrap
{
	using namespace detail::word_wrap::exports;
}
