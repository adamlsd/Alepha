static_assert( __cplusplus > 2020'00 );

#include "string_algorithms.h"

#include <memory>
#include <algorithm>
#include <exception>

#include "error.h"
#include "AutoRAII.h"

namespace Alepha::Hydrogen  ::detail::  string_algorithms
{
	namespace
	{
		namespace C
		{
			const bool debug= false;
			const bool debugExpansion= false or C::debug;
			const bool debugCommas= false or C::debug;
			const bool debugIOStreamLifecycle= false or C::debug;
		}

		struct VariableExpansionStreambuf
			: public Utility::StackableStreambuf
		{
			public:
				VarMap substitutions;
				std::stringbuf varName;
				char sigil;
				enum { Symbol= 1, Normal= 0 } mode= Normal;
				int throws= 0;

				explicit
				VariableExpansionStreambuf( std::ostream &os, VarMap &&substitutions, const char sigil )
					: StackableStreambuf( os ), substitutions( std::move( substitutions ) ), sigil( sigil )
				{}

				void
				writeChar( const char ch ) override
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

				void
				drain()
				{
					if( C::debugIOStreamLifecycle ) error() << "Drain called, and mode is: " << mode << std::endl;
					if( mode != Normal )
					{
						if( C::debugIOStreamLifecycle ) error() << "Mode not being normal, we're throwing (" << ++throws << " times now)..." << std::endl;
						mode= Normal;
						throw std::runtime_error{ "Unterminated variable `" + varName.str() + " in expansion." };
					}
				}
		};
	}

	void
	impl::build_streambuf( std::ostream &os, StartSubstitutions &&params )
	{
		new VariableExpansionStreambuf{ os, std::move( params.substitutions ), params.sigil };
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
		rv.push_back( std::move( next ) );
		return rv;
	}

	std::vector< std::string >
	exports::split( std::string s, const std::string &delim )
	{
		std::vector< std::string > rv;
		while( true )
		{
			const auto pos= s.find( delim );
			rv.push_back( s.substr( 0, pos ) );
			if( pos == std::string::npos ) break;
			s= s.substr( pos + delim.size() );
		}

		return rv;
	}
}
