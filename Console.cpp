static_assert( __cplusplus > 2020'00 );

#include "Console.h"

#include <termios.h>
#include <unistd.h>
#include <sys/ioctl.h>

#include <stack>
#include <vector>
#include <utility>
#include <string>
#include <sstream>
#include <iostream>

#include <ext/stdio_filebuf.h>

#include <Alepha/Utility/evaluation_helpers.h>

#include "Enum.h"
#include "ProgramOptions.h"
#include "StaticValue.h"
#include "string_algorithms.h"
#include "AutoRAII.h"

/*  
 * All of the terminal control code in this library uses ANSI escape sequences (https://en.wikipedia.org/wiki/ANSI_escape_code).
 * Instead of using termcap and curses, code can use this simpler library instead.  The simple fact is that in 2022, there's
 * probably no terminal software that you're using that does not understand these escape sequences.  Truth be told, the
 * termcap databases are suffering from a tremendous amount of bitrot, as no actual hardware or software in common use
 * actually uses anything but the ANSI codes.
 *  
 * Some ANSI engines have a maximum limit to how many "clauses" a CSI sequence can have.  To this end, no significant effort
 * should be put into trying to collapse the sequences for foreground and background into one SGR command.  Despite it taking
 * a few more bytes, it's probably more portable to issue multiple commands.
 */

// It's fairly safe to assume, in 2022, that common ANSI terminal sequences are
// universally supported for effectively all cases modern users will care about.


namespace Alepha::Hydrogen  ::detail::  console
{
	namespace
	{
		using namespace std::literals::string_literals;

		using namespace Utility::exports::evaluation_helpers;

		namespace C
		{
			const bool debug= false;

			const int defaultScreenWidthLimit= 100;

			// The Device Status Report should never be longer than this.
			const int maxLengthOfDSR= 64;
		}

		// TODO, this should be in its own lib.
		namespace storage
		{
			StaticValue< std::string > applicationName;

			auto init= enroll <=[]
			{
				if( applicationName().empty() ) applicationName()= "ALEPHA";
			};
		}
	}

	void
	exports::setApplicationName( std::string name )
	{
		storage::applicationName()= std::move( name );
	}

	const std::string &
	exports::applicationName()
	{
		return storage::applicationName();
	}

	namespace
	{
		auto screenWidthEnv() { return applicationName() + "_SCREEN_WIDTH"; }
		auto screenWidthEnvLimit() { return applicationName() + "_SCREEN_WIDTH_LIMIT"; }
		auto disableColorsEnv() { return applicationName() + "_DISABLE_COLOR_TEXT"; }
		auto colorsEnv() { return applicationName() + "_COLORS"; }
		auto sgr_nameEnv() { return applicationName() + "_SGR_NAMES"; }

		// TODO: Put this in a library
		int
		getEnvOrDefault( const std::string env, const int d )
		{
			if( getenv( env.c_str() ) )
			try
			{
				return boost::lexical_cast< int >( getenv( env.c_str() ) );
			}
			catch( const boost::bad_lexical_cast & ) {}
			return d;
		}

		int cachedScreenWidth= evaluate <=[]
		{
			const int underlying= getEnvOrDefault( screenWidthEnv(), Console::main().getScreenSize().columns );
			return std::min( underlying, getEnvOrDefault( screenWidthEnvLimit(), C::defaultScreenWidthLimit ) );
		};

		using ColorState= Enum< "always"_value, "never"_value, "auto"_value >;
		std::optional< ColorState > colorState;

		bool
		colorEnabled()
		{
			if( not colorState.has_value() ) return not ::getenv( disableColorsEnv().c_str() );

			if( colorState.value() == "never"_value ) return false;
			if( colorState.value() == "always"_value ) return true;
			assert( colorState.value() == "auto"_value );

			return ::isatty( 1 ); // Auto means only do this for TTYs.
		}

		StaticValue< std::map< Style, SGR_String > > colorVariables;


		SGR_String
		parse( const std::string &token )
		{
			const std::map< std::string, std::function< SGR_String () > > simple=
			{
				{ "reset", resetTextEffects },
				{ "bold", setBold },
				{ "dim", setFaint },
				{ "faint", setFaint },
				{ "italic", setItalic },
				{ "underline", setUnderline },
				{ "under", setUnderline },
				{ "blink", setBlink },
				{ "strike", setStrike },
				{ "strikethrough", setStrike },
				{ "strikethru", setStrike },
				{ "doubleunderline", setDoubleUnderline },
				{ "doubleunder", setDoubleUnderline },
				{ "framed", setFramed },
				{ "encircled", setEncircled },
				{ "overline", setOverline },
			};
			if( simple.contains( token ) ) return simple.at( token )();

			// The `fg:` is optional in "SGR Name"...
			if( token.starts_with( "fg:" ) ) return parse( token.substr( 2 ) );

			if( token.starts_with( "ansi:" ) ) return { '3' + token.substr( 5 ) };
			if( token.starts_with( "ext:grey" ) )
			{
				std::ostringstream oss;
				oss << "3;5;";
				oss << int( TextColor::greyscale_base ) + boost::lexical_cast< int >( token.substr( 8 ) );
				return { std::move( oss ).str() };
			}
			if( token.starts_with( "ext:rgb" ) )
			{
				const std::string rgb= token.substr( 7 );
				if( rgb.size() != 3 ) throw std::runtime_error{ "RGB request with more than 3 digits..." };
				const int r= boost::lexical_cast< int >( rgb.substr( 0, 1 ) );
				const int g= boost::lexical_cast< int >( rgb.substr( 1, 1 ) );
				const int b= boost::lexical_cast< int >( rgb.substr( 2, 1 ) );
				std::ostringstream oss;
				oss <<  "3;5;";
				oss << int( TextColor::rgb_base ) + r * int( TextColor::red_radix ) + g * int( TextColor::green_radix ) + b * int( TextColor::blue_radix );
				return { std::move( oss ).str() };
			}
			if( token.starts_with( "ext:" ) ) return { "38;5;" + token.substr( 4 ) };
			if( token.starts_with( "#" ) )
			{
				const auto code= token.substr( 1 );
				const auto [ r_s, g_s, b_s ]= evaluate <=[&]
				{
					if( code.size() == 3 ) return std::tuple{ code.substr( 0, 1 ), code.substr( 1, 1 ), code.substr( 2, 1 ) };
					if( code.size() == 6 ) return std::tuple{ code.substr( 0, 2 ), code.substr( 2, 2 ), code.substr( 4, 2 ) };
					throw std::runtime_error( "True color code parse error." );
				};
				int r, g, b;
				{
					std::istringstream iss{ r_s };
					iss >> std::hex >> r;
				}

				{
					std::istringstream iss{ g_s };
					iss >> std::hex >> g;
				}

				{
					std::istringstream iss{ b_s };
					iss >> std::hex >> b;
				}

				std::ostringstream oss;
				oss << "38;2;" << std::dec << r << ';' << g << ';' << b;
				return { std::move( oss ).str() };
			}

			throw std::runtime_error{ "Unrecognized SGR Name keyword: `" + token + "`" };
		}

		SGR_String
		parseTokens( std::vector< std::string > tokens )
		{

			SGR_String rv;
			for( const auto token: tokens ) rv+= parse( token );

			return rv;
		}

		auto init= enroll <=[]
		{
			--"screen-width"_option << affectsHelp << cachedScreenWidth << "Sets the screen width for use in automatic word-wrapping. !default!";
			--"color"_option << affectsHelp << colorState << "Select the application color preference.  If not passed, the environment variable `"
					<< disableColorsEnv() << "` will be respected.  Otherwise, `auto` will detect if a TTY is on stdout.  `never` will entirely "
					<< "disable color output.  And `always` will force color output.";
			--"list-color-variables"_option << []
			{
				for( const auto [ name, sgr ]: colorVariables() )
				{
					std::cout << name.name << ": ^[[" << sgr.code << "m" << std::endl;
				}
				::exit( EXIT_SUCCESS );
			}
			<< "Emit a list with the color variables supported by this application.  For use with the `" << colorsEnv()
			<< "` environment variable.";

			--"dump-color-env-var"_option << []
			{
				std::cout << "export " << colorsEnv() << "-\"";
				bool first= true;
				for( const auto &[ name, sgr ]: colorVariables() )
				{
					if( not first ) std::cout << ":";
					first= false;
					std::cout << name.name << "=" << sgr.code;
				}
				std::cout << "\"" << std::endl;
				::exit( EXIT_SUCCESS );
			}
			<< "Emit a BASH command which will set the appropriate environment variable to capture the current color settings for this "
			<< "application.";

		parse_environment_variables_for_color:
			// First the SGR Name language
			if( getenv( sgr_nameEnv().c_str() ) )
			{
				const std::string contents= getenv( sgr_nameEnv().c_str() );

				for( const auto var: split( contents, ';' ) )
				{
					const auto parsed= split( var, '=' );
					if( parsed.size() != 2 )
					{
						throw std::runtime_error{ "Color environment variable parse error in: `" + var + "`." };
					}

					const Style name{ parsed.at( 0 ) };
					const auto value= parsed.at( 1 );

					colorVariables()[ name ]= parseTokens( split( value, ' ' ) );
				}
			}
			// Then the regular terminal codes
			if( getenv( colorsEnv().c_str() ) )
			{
				const std::string contents= getenv( colorsEnv().c_str() );

				for( const auto var: split( contents, ':' ) )
				{
					const auto parsed= split( var, '=' );
					if( parsed.size() != 2 )
					{
						throw std::runtime_error{ "Color environment variable parse error in: `" + var + "`." };
					}

					const Style name{ parsed.at( 0 ) };
					const auto value= parsed.at( 1 );

					colorVariables()[ name ]= SGR_String{ value };
				}
			}
		};

		std::ostream &
		csi( std::ostream &os )
		{
			return os << "\e[";
		}
	}

	Style
	exports::createStyle( const std::string &name, const SGR_String &sgr )
	{
		if( name == "reset" ) throw std::runtime_error( "The `reset` style name is reserved." );
		Style style{ name };
		colorVariables().insert( { style, sgr } );
		return style;
	}

	void
	exports::sendSGR( std::ostream &os, const SGR_String style )
	{
		csi( os ) << style.code << 'm';
	}

	std::ostream &
	exports::operator << ( std::ostream &os, const Style &s )
	{
		if( colorEnabled() and colorVariables().contains( s ) )
		{
			sendSGR( os, colorVariables().at( s ) );
		}

		return os;
	}

	std::ostream &
	exports::operator << ( std::ostream &os, decltype( resetStyle ) )
	{
		if( colorEnabled() )
		{
			sendSGR( os, resetTextEffects() );
		}

		return os;
	}


	enum ConsoleMode
	{
		cooked, raw, noblock,
	};


	struct Console::Impl
	{
		int fd;
		// TODO: Do we want to make this not gnu libstdc++ specific?
		__gnu_cxx::stdio_filebuf< char > filebuf;
		std::ostream stream;
		std::stack< std::pair< struct termios, ConsoleMode > > modeStack;
		ConsoleMode mode= cooked;
		std::optional< int > cachedScreenWidth;

		explicit
		Impl( const int fd )
			: fd( fd ), filebuf( fd, std::ios::out ), stream( &filebuf )
		{}
	};

	auto
	Console::getMode() const
	{
		return pimpl().mode;
	}


	namespace
	{

		struct BadScreenStateError : std::runtime_error
		{
			BadScreenStateError() : std::runtime_error( "Error in getting terminal dimensions." ) {}
		};

		struct UnknownScreenError : std::runtime_error
		{
			UnknownScreenError() : std::runtime_error( "Terminal is unrecognized.  Using defaults." ) {}
		};

		auto
		rawModeGuard( Console console )
		{
			const bool skip= console.getMode() == ConsoleMode::raw;
			return AutoRAII
			{
				[skip, &console]
				{
					if( skip ) return;
					console.setRaw();
				},
				[skip, &console]
				{
					if( skip ) return;
					console.popTermMode();
				},
			};
		}
	}
	

	Console::Console( const int fd )
		: impl( std::make_unique< Impl >( fd ) )
	{}

	std::ostream &
	Console::csi()
	{
		return pimpl().stream;
	}

	void
	Console::popTermMode()
	{
		tcsetattr( pimpl().fd, TCSAFLUSH, &pimpl().modeStack.top().first );
		pimpl().mode= pimpl().modeStack.top().second;
		pimpl().modeStack.pop();
	}

	namespace
	{
		struct termios
		setRawModeWithMin( const int fd, const int min )
		{
			struct termios next;
			struct termios now;

			if( tcgetattr( fd, &now ) == -1 ) throw UnknownScreenError{};

			next.c_iflag&= ~( BRKINT | ICRNL | INPCK | ISTRIP | IXON );
			next.c_oflag&= ~( OPOST );
			next.c_cflag|= CS8;
			next.c_lflag&= !( ECHO | ICANON | IEXTEN | ISIG );
			next.c_cc[ VMIN ]= min;
			next.c_cc[ VTIME ]= 0;

			if( tcsetattr( fd, TCSAFLUSH, &next ) ) throw UnknownScreenError{};

			return now;
		}
	}

	void
	Console::setRaw()
	{
		const auto old= setRawModeWithMin( pimpl().fd, 1 );
		pimpl().modeStack.emplace( old, pimpl().mode );
		pimpl().mode= raw;
	}

	void
	Console::setNoblock()
	{
		const auto old= setRawModeWithMin( pimpl().fd, 0 );
		pimpl().modeStack.emplace( old, pimpl().mode );
		pimpl().mode= raw;
	}

	void Console::killLineTail() { csi() << 'K'; }
	void Console::killLineHead() { csi() << "1K"; }
	void Console::killLine() { csi() << "2K"; }

	void Console::hideCursor() { csi() << "?25l"; }
	void Console::showCursor() { csi() << "?25h"; }

	void Console::saveHardwareCursor() { csi() << 's'; }
	void Console::restoreHardwareCursor() { csi() << 'u'; }

	void Console::gotoX( const int x ) { csi() << x << 'G'; }

	void
	Console::gotoY( const int y )
	{
		cursorUp( 1'000'000 );
		cursorDown( y );
	}

	void Console::gotoXY( const int x, const int y ) { csi() << y << ';' << x << 'H'; }

	void Console::cursorUp( const unsigned amt ) { csi() << amt << 'A'; }
	void Console::cursorDown( const unsigned amt ) { csi() << amt << 'B'; }
	void Console::cursorRight( const unsigned amt ) { csi() << amt << 'C'; }
	void Console::cursorLeft( const unsigned amt ) { csi() << amt << 'D'; }

	void Console::clearScreen() { csi() << "2J"; }

	SGR_String exports::resetTextEffects() { return { "0" }; }

	SGR_String exports::setBold() { return { "1" }; }
	SGR_String exports::setFaint() { return { "2" }; }
	SGR_String exports::setItalic() { return { "3" }; }
	SGR_String exports::setUnderline() { return { "4" }; }
	SGR_String exports::setBlink() { return { "5" }; }
	SGR_String exports::setStrike() { return { "9" }; }
	SGR_String exports::setDoubleUnderline() { return { "21" }; }
	SGR_String exports::setFramed() { return { "51" }; }
	SGR_String exports::setEncircled() { return { "52" }; }
	SGR_String exports::setOverline() { return { "52" }; }

	SGR_String
	exports::setFgColor( const BasicTextColor c )
	{
		std::ostringstream oss;
		oss << '3' << int( c );
		return { std::move( oss ).str() };
	}

	SGR_String
	exports::setBgColor( const BasicTextColor c )
	{
		std::ostringstream oss;
		oss << '4' << int( c );
		return { std::move( oss ).str() };
	}

	SGR_String
	exports::setColor( const BasicTextColor fg, const BasicTextColor bg )
	{
		std::ostringstream oss;
		oss << '3' << int( fg ) << ";4" << int( bg );
		return { std::move( oss ).str() };
	}

	SGR_String
	exports::setExtFgColor( const TextColor c )
	{
		std::ostringstream oss;
		oss << "38;5;" << int( c );
		return { std::move( oss ).str() };
	}

	SGR_String
	exports::setExtBgColor( const TextColor c )
	{
		std::ostringstream oss;
		oss << "48;5;" << int( c );
		return { std::move( oss ).str() };
	}


	SGR_String
	exports::setExtColor( const TextColor fg, const TextColor bg )
	{
		std::ostringstream oss;
		oss << "38;2" << int( fg ) << "48;2" << int( bg );
		return { std::move( oss ).str() };
	}

	SGR_String
	exports::setExtUlColor( const TextColor ul )
	{
		std::ostringstream oss;
		oss << "58;5;" << int( ul );
		return { std::move( oss ).str() };
	}

	SGR_String exports::setFgTrueColor( const int rgb ) { return setFgTrueColor( rgb & 0xFF, ( rgb >> 8 ) & 0xFF, ( rgb >> 16 ) & 0xFF ); }

	SGR_String
	exports::setFgTrueColor( const int r, const int g, const int b )
	{
		std::ostringstream oss;
		oss << "38;2;" << r << ';' << g << ';' << b;
		return { std::move( oss ).str() };
	}

	SGR_String exports::setBgTrueColor( const int rgb ) { return setBgTrueColor( rgb & 0xFF, ( rgb >> 8 ) & 0xFF, ( rgb >> 16 ) & 0xFF ); }

	SGR_String
	exports::setBgTrueColor( const int r, const int g, const int b )
	{
		std::ostringstream oss;
		oss << "48;2;" << r << ';' << g << ';' << b;
		return { std::move( oss ).str() };
	}

	SGR_String exports::setUlTrueColor( const int rgb ) { return setUlTrueColor( rgb & 0xFF, ( rgb >> 8 ) & 0xFF, ( rgb >> 16 ) & 0xFF ); }

	SGR_String
	exports::setUlTrueColor( const int r, const int g, const int b )
	{
		std::ostringstream oss;
		oss << "58;2;" << r << ';' << g << ';' << b;
		return { std::move( oss ).str() };
	}

	SGR_String
	exports::operator ""_sgr( const char *const p, const std::size_t sz )
	{
		const std::string s{ p, p + sz };

		const auto tokens= split( s, ' ' );
		return parseTokens( tokens );
	}

	int
	exports::getConsoleWidth()
	{
		return cachedScreenWidth;
	}

	int
	Console::getScreenWidth()
	{
		if( not pimpl().cachedScreenWidth.has_value() )
		{
			pimpl().cachedScreenWidth= getScreenSize().columns;
		}

		return pimpl().cachedScreenWidth.value();
	}

	ScreenSize
	Console::getScreenSize()
	try
	{
		if( not isatty( pimpl().fd )  ) throw UnknownScreenError{};

		// Use the `ioctl( TIOCGWINSZ )`, but we'll just defer to 24x80 if we fail that...
		struct winsize ws;
		const int ec= ioctl( pimpl().fd, TIOCGWINSZ, &ws );
		if( ec == -1 or ws.ws_col == 0 ) throw UnknownScreenError{};

		return { ws.ws_row, ws.ws_col };
	}
	catch( const UnknownScreenError & ) { return { 24, 80 }; } // Fallback position....

	namespace
	{
		namespace storage
		{
			std::unique_ptr< Console > console;
		}
	}

	Console &
	Console::main()
	{
		if( not storage::console ) storage::console= std::make_unique< Console >( 1 ); // stdout
		return *storage::console;
	}
}
