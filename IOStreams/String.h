static_assert( __cplusplus > 2020'00 );

#pragma once

#include <Alepha/Alepha.h>

#include <sstream>

#include <Alepha/Concepts.h>

namespace Alepha::Hydrogen::IOStreams  ::detail::  string
{
	inline namespace exports
	{
		class String;

		enum { FinishString };

		std::string stringify( const Alepha::OStreamable auto &item, Alepha::OStreamable auto && ... params );
	}

	class exports::String
	{
		private:
			// TODO: We need the exception throwing capabilities of the
			// `boost::lexical_cast` operation.  But this stream technique
			// lets us build strings using stream modifiers and manipulators,
			// which `boost::lexical_cast` doesn't support.
			std::ostringstream oss;

		public:
			String &&
			operator << ( const Alepha::OStreamable auto &t ) &&
			{
				oss << t;
				return std::move( *this );
			}

			std::string
			operator << ( decltype( FinishString ) ) &&
			{
				return std::move( oss ).str();
			}

			operator std::string () &&
			{
				return std::move( *this ) << FinishString;
			}
	};

	inline std::string
	exports::stringify( const Alepha::OStreamable auto &item, Alepha::OStreamable auto && ... params )
	{
		return ( String{} << ... << params ) << item << FinishString;
	}
}

namespace Alepha::Hydrogen::IOStreams::inline exports::inline string
{
	using namespace detail::string::exports;
}
