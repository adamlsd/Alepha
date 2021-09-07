static_assert( __cplusplus > 201700, "C++17 Required" );

#pragma once

#include <Alepha/Alepha.h>

#include <string>

namespace Alepha::Hydrogen
{
	inline namespace exports { inline namespace console {} }

	namespace detail::console
	{
		inline namespace exports {}

		namespace C
		{
			const std::string csi= "\e[";
			const std::string green= C::csi + "32m";
			const std::string red= C::csi + "31m";
			const std::string normal= C::csi + "0m";
		}

		namespace exports
		{
			namespace C= detail::console::C;
		}
	}

	namespace exports::console
	{
		using namespace detail::console::exports;
	}
}
