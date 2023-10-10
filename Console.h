static_assert( __cplusplus > 2020'00 );

#pragma once

#include <string>
#include <memory>

#include <Alepha/TotalOrder.h>

// These are some terminal/console control primitives.
// There are several "modern" terminal assumptions built
// into this library.
//
// As long as this works on most (all?) modern terminal emulators, this should be
// fine.

namespace Alepha::inline Cavorite  ::detail::  console
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

		inline auto
		operator ""_sgr( const char *const p, const std::size_t sz )
		{
			return SGR_String{ { p, p + sz } };
		}

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

		[[nodiscard]] SGR_String setBlink();

		[[nodiscard]] SGR_String setFgColor( BasicTextColor fg );
		[[nodiscard]] SGR_String setBgColor( BasicTextColor bg );
		[[nodiscard]] SGR_String setColor( BasicTextColor fg, BasicTextColor bg );

		[[nodiscard]] SGR_String setExtFgColor( TextColor fg );
		[[nodiscard]] SGR_String setExtBgColor( TextColor fg );
		[[nodiscard]] SGR_String setExtColor( TextColor fg, TextColor bg );

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

		void sendSGR( std::ostream &os, SGR_String );

		int getConsoleWidth();
	}
}

namespace Alepha::Cavorite::inline exports::inline console
{
	using namespace detail::console::exports;
}
