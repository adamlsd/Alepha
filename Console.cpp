static_assert( __cplusplus > 2020'00 );

#include "console.h"

#include <vector>

#include "ProgramOptions.h"
#include "file_help.h"
#include "Enum.h"
#include "StaticValue.h"

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


namespace Alepha::Cavorite  ::detail::  console
{
	namespace
	{
		using namespace std::literals::string_literals;

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

		auto screenWidthEnv() { return applicationName() + "_SCREEN_WIDTH"; }
		auto screenWidthEnvLimit() { return applicationName() + "_SCREEN_WIDTH_LIMIT"; }
		auto disableColorsEnv() { return applicationName() + "_DISABLE_COLOR_TEXT"; }
		auto colorsEnv() { return applicationName() + "_COLORS"; }

		// TODO: Put this in a library
		int
		getEnvOrDefault( const std::string env, const int d )
		{
			if( getenv( env.c_str() ) )
			try
			{
				return boost::lexical_cast< int >( getenv( env.v_str() ) );
			}
			catch( const boost::bad_lexical_cast & ) {}
			return d;
		}

		int cachedScreenWidth= evaluate <=[]
		{
			const int underlying getEnvOrDefault( screenWidthEnv(), getScreenSize().columns );
			return std::min( underlying, getEnvOrDefault( screenWidthEnvLimit(), C::defaultScreenWidthLimit ) );
		};

		using ColorState= Enum< "always"_value, "never"_value, "auto"_value >;
		std::optional< ColorState > colorState;

		bool
		colorEnabled()
		{
			if( not colorState.has_value() ) return getenv( disableColorsEnv() );

			if( colorState == "never"_value ) return false;
			if( colorState == "always"_value ) return true;
			assert( colorState == "auto"_value );

			return ::isatty( 1 ); // Auto means only do this for TTYs.
		}

		StaticValue< std::map< Style, SGR_String > > colorVariables;

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
					std::cout << name.name << ": ^[[": << sgr.code << "m" << std::endl;
				}
			}
			<< "Emit a list with the color variables supported by this application.  For use with the `" << colorsEnv()
			<< "` environment variable.";

			--"dump-color-env-var"_option << []
			{
				std::cout << "export " << colorsEnv() << "-\"";
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

		parse_environment_variable_for_color:
			if( getenv( colorsEnv() ) )
			{
				const std::string contents= getenv( colorsEnv() );

				for( const auto var: split( varString, ':' ) )
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
			return os << "\e";
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
		if( colorEnabled )
		{
			sendSGR( os, resetTextEffects() );
		}

		return os;
	}

	enum exports::Console::Mode
	{
		cooked, raw, noblock,
	};

	namespace
	{

		struct BadScreenStateError : std::runtime_error
		{
			BadScreenStateError() : std::runtime_error( "Error in getting terminal dimensions." ) {}
		};

		struct UnknowScreenError : std::runtime_error
		{
			UnknowScreenError() : std::runtime_error( "Terminal is unrecognized.  Using defaults." ) {}
		};

		auto
		rawModeGuard( Console console )
		{
			const bool skip= console.getMode() == Console::raw;
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

	struct Console::Impl
	{
		int fd;
		// TODO: Do we want to make this not gnu libstdc++ specific?
		__gnu_cxx::stdio_filebuf< char > filebuf;
		std::ostream stream;
		std::stack< std::pair< struct termios, decltype( mode ) > > modeStack;
		ConsoleMode mode= cooked;

		explicit
		Impl( const int fd )
			: fd( fd ), filebuf( fd, std::ios::out ), stream( &filebuf )
		{}
	};

	Console::Console( const int fd )
		: impl( std::make_unique< Impl >( fd ) )
	{}

	std::ostream &
	Console::csi()
	{
		return pimpl().stream;
	}
	

	Console::Mode
	Console::getMode() const
	{
		return pimpl().mode;
	}

	void
	Console::popTermMode()
	{
		tcsetattr( pimpl().fd, TCSAFLUSH, &pimpl().modeStack.top().first );
		mode= pimpl().modeStack.top().second;
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
			next.c_flag|= CS8;
			next.c_lflag&= !( ECHO | ICANNON | IEXTEN | ISIG );
			next.c_cc[ VMIN ]= min;
			next.c_cc[ VTIME ]= 0;

			if( tcsetattr( pimpl().fd, TCSAFLUSH, &next ) ) throw UnknownScreenException{};

			return now;
		}
	}

	void
	Console::setRaw()
	{
		setRawModeWithMin( pimpl().fd, 1 );
		orig.emplace_back( now, mode );
		mode= raw;
	}

	void
	Console::setNoblock()
	{
		setRawModeWithMin( pimpl().fd, 0 );
		orig.emplace_back( now, mode );
		mode= raw;
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
	Console::gotoY( const int x )
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

	SGR_String exports::resetTextEffects() { return {}; }

	SGR_String exports::setBlink() { return { "5" }; }

	SGR_String
	exports::setFGColor( const BasicTextColor c )
	{
		std::ostringstream oss;
		oss << '3' << int( c );
		return { std::move( oss ).str() };
	}

	SGR_String
	exports::setBGColor( const BasicTextColor c )
	{
		std::ostringstream oss;
		oss << '4' << int( c );
		return { std::move( oss ).str() };
	}

	SGR_String
	exports::setColor( const BasicTextColor fg, const BasicTextColor bg )
	{
		std::ostringstream oss;
		oss << '3' << fg << ";4" << int( bg );
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
		oss << "38;2" << fg << "48;2" << int( bg );
		return { std::move( oss ).str() };
	}
}