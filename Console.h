static_assert( __cplusplus > 2020'00 );

#pragma once

#include <Alepha/Alepha.h>

#include <string>
#include <memory>

#include <Alepha/TotalOrder.h>

// These are some terminal/console control primitives.
// There are several "modern" terminal assumptions built
// into this library.
//
// As long as this works on most (all?) modern terminal emulators, this should be
// fine.

namespace Alepha::Hydrogen  ::detail::  console
{
	inline namespace exports {}

	namespace exports
	{
		struct ScreenSize;
		struct CursorPosition;

		class Console;

		struct SGR_String
		{
			std::string code;
		};

		[[nodiscard]] inline SGR_String
		operator + ( const SGR_String lhs, const SGR_String rhs )
		{
			if( lhs.code.empty() ) return rhs;
			if( rhs.code.empty() ) return lhs;
			return SGR_String{ lhs.code + ';' + rhs.code };
		}

		inline SGR_String &
		operator += ( SGR_String &lhs, const SGR_String rhs )
		{
			return lhs= lhs + rhs;
		}

		// Parses sgr token names, like "bold ext:red"
		[[nodiscard]] SGR_String operator ""_sgr( const char *p, std::size_t sz );

		enum class BasicTextColor : int;
		enum class TextColor : int;

		struct Style
		{
			std::string name;

			TotalOrder operator <=> ( const Style & ) const= default;
		};
		std::ostream &operator << ( std::ostream &, const Style & );

		Style createStyle( const std::string &name, const SGR_String &style );
		bool styleVarSet( const std::string &name );

		enum ResetStyle { resetStyle };
		std::ostream &operator << ( std::ostream &, ResetStyle );

		// TODO: Move this to its own library.
		const std::string &applicationName();
		void setApplicationName( std::string name );
	}

	struct exports::ScreenSize
	{
		int rows;
		int columns;
	};

	struct exports::CursorPosition
	{
		int x;
		int y;
	};

	class exports::Console
	{
		private:
			class Impl;
			std::unique_ptr< Impl > impl;
			Impl &pimpl() noexcept { return *impl; }
			const Impl &pimpl() const noexcept { return *impl; }

			std::ostream &csi();


		public:
			// A console object can only be constructed on a raw UNIX file descriptor.
			explicit Console( int fd );

			static Console &main();
			auto getMode() const;

			int getScreenWidth();
			int getScreenHeight();

			// Althought this could be implemented by combining the above observers,
			// there are more efficient ways to implement this, thus we actually
			// keep them separate.
			ScreenSize getScreenSize();

			void hideCursor();
			void showCursor();

			// Pushes the current mode (raw or normal) onto the stack.
			void setRaw();

			// A nonblock mode is used to prevent terminal IO from blocking.  This helps with keypress driven
			// programs.  This is its own raw mode.
			void setNoblock();

			// The Console object maintains an internal stack of the last terminal modes it set and how to
			// revert tothe previous.  There is no arbitrary limit on this stack size.  Calling this with
			// no previous modes is a no-op.
			void popTermMode();

			// Line kill functions keep the cursor where it is, but erase the specified text on the line.
			void killLine();
			void killLineHead();
			void killLineTail();

			// One should avoid calling these, as some internals may also use these hardware functions.
			// It is better to use `gotoXY` and `getXY` to save/restore cursor positions.  One should
			// maintain a software stack (as a caller) of cursor positions, if necessary.
			void saveHardwareCursor();
			void restoreHardwareCursor();

			void gotoX( int x );
			void gotoY( int y );
			void gotoXY( int x, int y );

			int getX();
			int getY();
			CursorPosition getXY();

			void cursorUp( unsigned amt= 0 );
			void cursorDown( unsigned amt= 0 );

			void cursorLeft( unsigned amt= 0 );
			void cursorRight( unsigned amt= 0 );

			void clearScreen(); // `console` library does direct cursor control, so this won't return the cursor to 1,1.
	};

	namespace exports
	{
		[[nodiscard]] SGR_String resetTextEffects();

		// Non Colour effects (Mostly sorted by ANSI/ECMA SGR code numeric order)
		[[nodiscard]] SGR_String setBold();
		[[nodiscard]] SGR_String setFaint();
		[[nodiscard]] SGR_String setItalic();
		[[nodiscard]] SGR_String setUnderline();
		[[nodiscard]] SGR_String setBlink();
		[[nodiscard]] SGR_String setStrike();
		[[nodiscard]] SGR_String setDoubleUnderline();
		[[nodiscard]] SGR_String setFramed();
		[[nodiscard]] SGR_String setEncircled();
		[[nodiscard]] SGR_String setOverline();

		[[nodiscard]] SGR_String setFgColor( BasicTextColor fg );
		[[nodiscard]] SGR_String setBgColor( BasicTextColor bg );
		[[nodiscard]] SGR_String setColor( BasicTextColor fg, BasicTextColor bg );

		[[nodiscard]] SGR_String setExtFgColor( TextColor fg );
		[[nodiscard]] SGR_String setExtBgColor( TextColor fg );
		[[nodiscard]] SGR_String setExtColor( TextColor fg, TextColor bg );
		[[nodiscard]] SGR_String setExtUlColor( TextColor ul );

		// Basic color wrapping aliases:
		[[nodiscard]] inline SGR_String setExtFgColor( const BasicTextColor fg ) { return setExtFgColor( static_cast< TextColor >( fg ) ); }
		[[nodiscard]] inline SGR_String setExtBgColor( const BasicTextColor bg ) { return setExtBgColor( static_cast< TextColor >( bg ) ); }

		[[nodiscard]] inline SGR_String setExtColor( const      TextColor fg, const BasicTextColor bg ) { return setExtColor(                         ( fg ), static_cast< TextColor >( bg ) ); }
		[[nodiscard]] inline SGR_String setExtColor( const BasicTextColor fg, const TextColor      bg ) { return setExtColor( static_cast< TextColor >( fg ),                         ( bg ) ); }
		[[nodiscard]] inline SGR_String setExtColor( const BasicTextColor fg, const BasicTextColor bg ) { return setExtColor( static_cast< TextColor >( fg ), static_cast< TextColor >( bg ) ); }

		[[nodiscard]] SGR_String setFgTrueColor( int rgb );
		[[nodiscard]] SGR_String setFgTrueColor( int r, int g, int b );

		[[nodiscard]] SGR_String setBgTrueColor( int rgb );
		[[nodiscard]] SGR_String setBgTrueColor( int r, int g, int b );

		[[nodiscard]] SGR_String setUlTrueColor( int rgb );
		[[nodiscard]] SGR_String setUlTrueColor( int r, int g, int b );

		void sendSGR( std::ostream &os, SGR_String );

		int getConsoleWidth();
	}

	enum class exports::BasicTextColor : int
	{
		black= 0,
		red= 1,
		green= 2,
		brown= 3,
		blue= 4,
		magenta= 5,
		cyan= 6,
		grey= 7,
	};

	enum class exports::TextColor : int
	{
		black= 0,
		dim_red= 1,
		dim_green= 2,
		dim_brown= 3,
		dim_blue= 4,
		dim_magenta= 5,
		dim_cyan= 6,
		bright_grey= 7,

		// Note that bright and dim grey are reverse, since bright grey is dim white and dim grey si bright black.
		// The names are more understandable this way, I think

		dim_grey= 8,
		bright_red= 9,
		bright_green= 10,
		bright_brown= 11,
		bright_blue= 12,
		bright_magenta= 13,
		bright_cyan= 14,
		white= 15,

		rgb_base= 16,
		red_radix= 36,
		green_radix= 6,
		blue_radix= 0,

		greyscale_base= 232, // Add N to this to get the greyscale offset.
	};
		
}

namespace Alepha::Hydrogen::inline exports::inline console
{
	using namespace detail::console::exports;
}
