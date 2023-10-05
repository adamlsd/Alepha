static_assert( __cplusplus > 2020'00 );

#include "string_algorithms.h"

#include <algorithm>
#include <exception>

#include "error.hpp"

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
	}

	std::string
	exports::expandVariables( const std::string &text, const VarMap &vars, const char sigil )
	{
		if( C::debugExpansion ) error() << "Expanding variables in " << text << std::endl;

		std::string rv;
		std::string varName;

		enum { Symbol, Normal } mode= Normal;

		for( const char ch: text )
		{
			if( mode == normal and ch == sigil )
			{
				mode= Symbol;
				varName.clear();
				continue;
			}
			if( mode == Symbol and ch == sigil )
			{
				mode= Normal;
				if( not varName.empty() )
				{
					if( not vars.contains( varName ) )
					{
						throw std::runtime_error( "No such variable: `" + varName + "`" );
					}
					if( C::debugVariableExpansion ) error() << "Expanding variable with name `" << varName << "`" << std::endl;
					rv+= vars.at( varName )();
				}
				else rv+= sigil;
				continue;
			}

			auto &current= mode == Normal ? rv : varName;
			current+= ch;
		}

		if( mode != Normal ) throw std::runtime_error( "Unterminated variable `" + varName + " in expansion of `" + text + "`" );
		if( C::debugExpansion ) error() << "Expansion was: `" << rv << "`" << std::endl;

		return rv;
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
}
