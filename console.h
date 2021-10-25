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
			const std::string color_code= "m";

			enum Layer
			{
				fg_code= '3',
				bg_code= '4',
			};

			enum class Color : char
			{
				black= '0',
				red= '1',
				green= '2',
				brown= '3',
				blue= '4',
				magenta= '5',
				cyan= '6',
				white= '7',
			};
		}
		using C::Layer;
		using C::Color;

		inline std::string
		make_color( const Layer layer, const Color color )
		{
			return C::csi + char(layer) + char(color) + C::color_code;
		}

		namespace C
		{
			inline namespace exports
			{
				const std::string normal= C::csi + '0' + color_code;

				inline namespace fg
				{
					const std::string black= make_color( C::fg_code,  C::Color::black );
					const std::string red= make_color( C::fg_code, C::Color::red );
					const std::string green= make_color( C::fg_code, C::Color::green );
					const std::string brown= make_color( C::fg_code, C::Color::brown );
					const std::string blue= make_color( C::fg_code, C::Color::blue );
					const std::string magenta= make_color( C::fg_code, C::Color::magenta );
					const std::string cyan= make_color( C::fg_code, C::Color::cyan );
					const std::string white= make_color( C::fg_code, C::Color::white );
				}

				namespace bg
				{
					const std::string black= make_color( C::bg_code,  C::Color::black );
					const std::string red= make_color( C::bg_code, C::Color::red );
					const std::string green= make_color( C::bg_code, C::Color::green );
					const std::string brown= make_color( C::bg_code, C::Color::brown );
					const std::string blue= make_color( C::bg_code, C::Color::blue );
					const std::string magenta= make_color( C::bg_code, C::Color::magenta );
					const std::string cyan= make_color( C::bg_code, C::Color::cyan );
					const std::string white= make_color( C::bg_code, C::Color::white );
				}
			}
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
