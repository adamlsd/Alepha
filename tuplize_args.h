static_assert( __cplusplus > 2020'00 );

#pragma once

#include <Alepha/Alepha.h>

#include <cassert>
#include <cstddef>

#include <iostream>

#include <vector>
#include <string>
#include <optional>
#include <tuple>

#include <boost/lexical_cast.hpp>

#include <Alepha/IOStreams/Stream.h>

#include "meta.h"
#include "error.h"
#include "Concepts.h"
#include "type_lisp.h"

namespace Alepha::Hydrogen  ::detail::  tuplize_args
{

	inline namespace exports
	{
		/*!
		 * And automatic parser/parser-generator for runtime string arguments to compiletime type lists.
		 *
		 * Iterates thru the runtime arguments and attempts to parse them to the compiletime specified types.
		 */
		template< Tuple T >
		auto
		tuplizeArgs( const std::vector< std::string > &args );

		class ArityMismatchError : public std::exception
		{
			private:
				std::size_t remaining_;
				std::size_t processed_;
				std::string clarification;

				std::string message;

			public:
				explicit
				ArityMismatchError( const std::size_t remaining, const std::size_t processed, const std::string &clarification= "" )
				: remaining_( remaining ), processed_( processed ), clarification( clarification ),
				message( IOStreams::Stream{} << ( clarification.empty() ? "" : ( clarification + ": " ) )
						<< "Argument count mismatch.  "
						<<  remaining << " remaining "
						<<  processed << " processed" ) {}


				const char *
				what() const noexcept override
				{
					return this->message.c_str();
				}

				std::size_t remaining() const { return this->remaining_; }
				std::size_t processed() const { return this->processed_; }
		};
	}

	namespace C
	{
		const bool debug= false;
	}

	// TODO: Expand this to handle compiletime-bound defaulted values.
	template< typename Type >
	concept Omittable= is_optional_v< Type >;

	template< TypeListType list >
	tuple_from_list_t< list >
	tuplizeArgsBackend( list, const std::vector< std::string > &args, std::size_t offset= 0 )
	{
		using first= car_t< list >;
		using tail= cdr_t< list >;

		if constexpr( std::is_same_v< first, Nil > )
		{
			// By this point, everything should be consumed.  If any remain, throw an exception.
			if( args.size() > offset ) throw ArityMismatchError{ args.size() - offset, offset, "some arguments left" };
			assert( offset == args.size() );
			return std::tuple{};
		}
		else if constexpr( Omittable< first > )
		{
			constexpr bool flexible_end= std::is_same_v< tail, Nil > or Omittable< car_t< tail > >;
			static_assert( flexible_end, "All optional arguments must be at the end." );
			if( offset >= args.size() )
			{
				return std::tuple_cat( std::tuple< first >{}, tuplizeArgsBackend( tail{}, args, offset ) );
			}
			else return tuplizeArgsBackend( cons_t< typename first::value_type, tail >{}, args, offset );
		}
		else if( offset >= args.size() ) throw ArityMismatchError{ args.size() - offset, offset, "no more arguments left" };
		else if constexpr( Vector< first > )
		{
			static_assert( std::is_same_v< cdr_t< list >, Nil >, "A vector is only permissible as the final argument." );
			if( args.size() <= offset )
			{
				throw ArityMismatchError{ args.size() - offset, offset, "a vector/list requires at least one runtime argument." };
			}
			using first_type= typename first::value_type;

			std::vector< first_type > rv;
			std::transform( begin( args ) + offset, end( args ), back_inserter( rv ),
					boost::lexical_cast< first_type, std::string > );
			return std::tuple_cat( std::tuple{ rv }, tuplizeArgsBackend( tail{}, args, offset + rv.size() ) );
		}
		else
		{
			if( offset >= args.size() )
			{
				throw ArityMismatchError{ args.size() - offset, offset, "no more arguments left." };
			}
			auto remainder= tuplizeArgsBackend( tail{}, args, offset + 1 );
			auto rv= std::tuple_cat( std::make_tuple( boost::lexical_cast< first >( args.at( offset ) ) ),
				std::move( remainder ) );
			return rv;
		}
	}

	template< Tuple T >
	auto
	exports::tuplizeArgs( const std::vector< std::string > &args )
	{
		return tuplizeArgsBackend( list_from_tuple_t< T >{}, args );
	}
}

namespace Alepha::Hydrogen::inline exports::inline tuplize_args
{
	using namespace detail::tuplize_args::exports;
}
