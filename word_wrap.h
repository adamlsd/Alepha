static_assert( __cplusplus > 2020'00 );

#pragma once

#include <Alepha/Alepha.h>

#include <cstddef>

#include <string>
#include <streambuf>

#include <Alepha/Utility/StackableStreambuf.h>

namespace Alepha::Hydrogen  ::detail::  word_wrap_m
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

namespace Alepha::Hydrogen::inline exports::inline word_wrap_m
{
	using namespace detail::word_wrap_m::exports;
}
