static_assert( __cplusplus > 2020'00 );

#pragma once

#include <cstddef>

#include <string>
#include <streambuf>

#include <Alepha/Utility/StackableStreambuf.h>

namespace Alepha::inline Cavorite  ::detail::  word_wrap
{
	inline namespace exports
	{
		std::string wordWrap( const std::string &text, std::size_t width, std::size_t nextLineOffset= 0 );

		struct StartWrap_params
		{
			std::size_t width;
			std::size_t nextLineOffset;
			
			explicit StartWrap_params( const std::size_t width, const std::size_t nextLineOffset= 0 ) : width( width ), nextLineOffset( nextLineOffset ) {}
		};

		using StartWrap= Utility::PushStack< StartWrap_params >;

		constexpr Utility::PopStack EndWrap;
	}

	inline namespace impl
	{
		void build_streambuf( std::ostream &os, StartWrap &&args );
	}
}

namespace Alepha::Cavorite::inline exports::inline word_wrap
{
	using namespace detail::word_wrap::exports;
}
