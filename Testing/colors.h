static_assert( __cplusplus > 2020'00 );

#pragma once

#include <Alepha/Console.h>

namespace Alepha::Hydrogen::Testing  ::detail::  testing_colors
{
	namespace C
	{
		inline namespace Colors
		{
			inline const auto testFail= createStyle( "test-failure", setFgColor( BasicTextColor::red ) );
			inline const auto testPass= createStyle( "test-success", setFgColor( BasicTextColor::green ) );
		}
	}
}
