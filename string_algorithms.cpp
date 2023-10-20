static_assert( __cplusplus > 2020'00 );

#include "string_algorithms.h"

#include <memory>
#include <algorithm>
#include <exception>

#include "error.h"
#include "AutoRAII.h"

namespace Alepha::Cavorite  ::detail::  string_algorithms
{
	namespace
	{
		namespace C
		{
			const bool debug= false;
			const bool debugExpansion= false or C::debug;
			const bool debugCommas= false or C::debug;
		}

		struct VariableExpansionStreambuf
			: public std::streambuf
		{
			public:
				std::streambuf *underlying= nullptr;
				VarMap substitutions;
				std::stringbuf varName;
				char sigil;
				enum { Symbol= 1, Normal= 0 } mode= Normal;
				int throws= 0;

				void
				writeChar( const char ch )
				{
					std::ostream current{ underlying };

					if( mode == Normal and ch == sigil )
					{
						mode= Symbol;
						varName.str( "" );
						return;
					}
					if( mode == Symbol and ch == sigil )
					{
						mode= Normal;
						const std::string name( varName.view() );
						if( not name.empty() )
						{
							if( not substitutions.contains( name ) )
							{
								throw std::runtime_error{ "No such variable: `" + name +"`" };
							}
							if( C::debugExpansion ) error() << "Expanding variable with name `" << name << "`" << std::endl;
							current << substitutions.at( name )();
						}
						else current << sigil;
						return;
					}

					if( mode == Symbol ) current.rdbuf( &varName );
					current << ch;
				}

				int
				overflow( const int ch ) override
				{
					if( ch == EOF ) throw std::logic_error{ "EOF!" };
					writeChar( ch );

					return 1;
				}

				std::streamsize
				xsputn( const char *const data, const std::streamsize amt ) override
				{
					for( std::streamsize i= 0; i< amt; ++i ) overflow( data[ i ] );
					return amt;
				}

				void
				drain()
				{
					std::cerr << "Drain called, and mode is: " << mode << std::endl;
					if( mode != Normal )
					{
						std::cerr << "Mode not being normal, we're throwing (" << ++throws << " times now)..." << std::endl;
						mode= Normal;
						throw std::runtime_error{ "Unterminated variable `" + varName.str() + " in expansion." };
					}
				}
		};

		const auto wrapperIndex= std::ios::xalloc();

		void
		releaseWrapper( std::ostream &os )
		{
			std::cerr << "Release wrapper called on: " << &os << std::endl;
			auto *const streambuf= static_cast< VariableExpansionStreambuf * >( os.pword( wrapperIndex ) );
			if( not streambuf ) throw std::logic_error{ "Attempt to remove a substitution context which doesn't exist." };

			AutoRAII current
			{
				[&] { return os.rdbuf( streambuf->underlying ); },
				[&] ( std::streambuf *streambuf ) noexcept
				{
					std::cerr << "Deletion actually happening, now." << std::endl;
					delete streambuf;
					os.pword( wrapperIndex )= nullptr;
				}
			};
			streambuf->drain();
		}

		void
		wordwrapCallback( const std::ios_base::event event, std::ios_base &ios, const int idx ) noexcept
		{
			std::cerr << "ios callback called on: " << &ios << std::endl;
			if( wrapperIndex != idx ) throw std::logic_error{ "Wrong index." };

			if( not ios.pword( wrapperIndex ) ) return;


			if( const auto os_p= dynamic_cast< std::ostream * >( &ios ); os_p and event == std::ios_base::erase_event )
			{
				releaseWrapper( *os_p );
			}
		}
	}

	std::ostream &
	impl::operator << ( std::ostream &os, StartSubstitutions &&params )
	{
		auto streambuf= std::make_unique< VariableExpansionStreambuf >();
		streambuf->underlying= os.rdbuf( streambuf.get() );
		streambuf->substitutions= std::move( params.substitutions );
		streambuf->sigil= params.sigil;
		auto &state= os.iword( wrapperIndex );
		if( not state )
		{
			state= 1;
			os.register_callback( wordwrapCallback, wrapperIndex );
			std::cerr << "Adding callback to " << (void *) static_cast< std::ios * >( &os ) << std::endl;
		}

		assert( os.pword( wrapperIndex ) == nullptr );
		os.pword( wrapperIndex )= streambuf.release();

		return os;
	}

	std::ostream &
	impl::operator << ( std::ostream &os, EndSubstitutions_t )
	{
		releaseWrapper( os );
		return os;
	}

	std::string
	exports::expandVariables( const std::string &text, const VarMap &vars, const char sigil )
	{
		std::ostringstream oss;

		oss << StartSubstitutions{ sigil, vars };
		oss << text;
		oss << EndSubstitutions;

		return std::move( oss ).str();
	}

	std::vector< std::string >
	exports::parseCommas( const std::string &string )
	{
		enum { Text, Backslash } state= Text;

		std::vector< std::string > rv;
		std::string next;
		for( const char ch: string )
		{
			if( state == Backslash )
			{
				state= Text;
				next+= ch;
			}
			else if( state == Text )
			{
				if( ch == '\\' ) state= Backslash;
				else if( ch == ',' )
				{
					if( C::debugCommas ) error() << "Parsed from commas: `" << next << "`" << std::endl;
					rv.push_back( std::move( next ) );
					next.clear();
				}
				else next+= ch;
			}
		}

		if( C::debugCommas ) error() << "Final parsed from commas: `" << next << "`" << std::endl;
		rv.push_back( std::move( next ) );

		return rv;
	}

	std::vector< std::string >
	exports::split( const std::string &s, const char token )
	{
		std::vector< std::string > rv;
		std::string next;
		for( const char ch: s )
		{
			if( ch != token )
			{
				next+= ch;
				continue;
			}
			rv.push_back( std::move( next ) );
			next.clear();
		}
		return rv;
	}
}
