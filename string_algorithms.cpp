static_assert( __cplusplus > 2020'00 );

#include "string_algorithms.h"

#include <memory>
#include <algorithm>
#include <exception>

#include "error.h"

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
				enum { Symbol, Normal } mode= Normal;

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
					if( mode != Normal ) throw std::runtime_error{ "Unterminated variable `" + varName.str() + " in expansion." };
				}
		};

		const auto wrapperIndex= std::ios::xalloc();

		void
		releaseWrapper( std::ios_base &ios )
		{
			auto *const streambuf= static_cast< VariableExpansionStreambuf * >( ios.pword( wrapperIndex ) );
			if( not streambuf ) throw std::logic_error{ "Attempt to remove a substitution context which doesn't exist." };

			streambuf->drain();
			dynamic_cast< std::ostream & >( ios ).rdbuf( streambuf->underlying );
			delete streambuf;
			ios.pword( wrapperIndex )= nullptr;
		}

		void
		wordwrapCallback( const std::ios_base::event event, std::ios_base &ios, const int idx )
		{
			if( wrapperIndex != idx ) throw std::logic_error{ "Wrong index." };

			if( not ios.pword( wrapperIndex ) ) return;

			if( event == std::ios_base::erase_event ) releaseWrapper( ios );
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
