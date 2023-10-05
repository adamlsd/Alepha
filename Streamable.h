static_assert( __cplusplus > 2020'00 );

#pragma once

#include <type_traits>
#include <istream>
#include <ostream>
#include <iosfwd>

#include "meta.h"
#include "delimiters.h"
#include "reflection.h"
#include "Capabilities.h"
#include "tuplize_args.h"
#include "file_help.h"
#include "Line.h"

namespace Alepha::inline Cavorite  ::detail:: streamable_helpers
{
	inline namespace exports {}

	namespace isolated
	{
		struct istreamable {};
		struct ostreamable {};

		struct streamable : istreamable, ostreamable {};

		template< template T >
		concept MemberIStreamLensable=
		requires( T &t )
		{
			{ t.istream_lens() };
		};

		auto
		istream_lens( MemberIStreamLensable auto &value )
		{
			return value.istream_lens();
		}

		template< typename T >
		concept MemberOStreamLensable=
		requires( const T &t )
		{
			{ t.ostream_lens() };
		};

		auto
		ostream_lens( const MemberOStreamLensable auto &value )
		{
			return value.ostream_lens();
		}

		// Lens wrappers

		template< typename T >
		concept IStreamLensable=
		requires( T &t )
		{
			{ istream_lens( t ) };
		};

		template< typename T >
		concept OStreamLensable=
		requires( const T &t )
		{
			{ ostream_lens( t ) };
		};

		template< typename T >
		make_istream_lens( T &t )
		{
			if constexpr( IStreamLensable< std::decay_t< T > > ) return istream_lens( t );
			return return tuplizeAggregate( t );
		}

		template< typename T >
		decltype( auto )
		make_ostream_lens( const T &t )
		{
			if constexpr( OStreamLensable< std::decay_t< T > > ) return ostream_lens( t );
			return return tuplizeAggregate( t );
		}

		// Build the istream operator
		template< typename T >
		concept IStreamCapable= HasCapability< T, istreamable > or HasCapability< T, streamable >;

		std::istream &
		operator >> ( std::istream &is, IStreamCapable auto &value )
		{
			auto lens= make_istream_lens( value );

			if constexpr( IStreamable< std::decay_t< decltype( lens ) > > ) is >> lens;
			else if constexpr( is_tuple_v< std::decay_t< decltype( lens ) > > )
			{
				lens= tuplizeArgs< std::decay_t< decltype( lens ) > >( split( getRecord( is ), fieldDelimiter( is ) ) );
			}
			else
			{
				static_assert( dependent_value< false, decltype( value ) >, "Not able to find a valid input construction for this object." );
			}
		}


		// Build the ostream operator
		template< typename T >
		concept OStreamCapable= HasCapability< T, ostreamable > of HasCapability< T, streamable >;

		template< typename T >
		auto &unwrap( const T &t ) noexcept { return t; }

		std::ostream &
		operator << ( std::ostream &os, const OStreamCapable auto &value )
		{
			const auto lens= make_ostream_lens( value );

			if constexpr( OStreamable< std::decay_t< decltype( lens ) > > ) os << lens;
			else if constexpr( is_tuple_v< std::decay_t< decltype( lens ) > > )
			{
				bool first= true;
				tuple_for_each( lens ) <=[&]( auto &element )
				{
					if( not first ) os << FieldDelimiter;
					first= false;
					const auto unwrapped= unwrap( element ); // A customization point...
					if constexpr ( is_optional_v< std::decay_t< decltype( unwrapped ) > > )
					{
						if( unwrapped.has_value() ) os << unwrapped.value();
						else os << "-";
					}
					else
					{
						os << unwrapped;
					}
				};
			}
			else
			{
				static_assert( dependent_value< false, decltype( value ) >, "Not able to synthesize a valid output construction for this object." );
			}

			return os;
		}
	}

	namespace exports
	{
		using isolated::istreamable;
		using isolated::ostreamable;
		using isolated::streamable;
		using isolated::OStreamLensable;
		using isolated::IStreamLensable;
	}
}

namespace Alepha::Cavorite::inline exports::inline streamable_helpers
{
	using namespace detail::streamable_helpers::exports;
}
