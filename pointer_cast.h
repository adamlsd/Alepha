static_assert( __cplusplus > 2020'00 );

#pragma once

#include <memory>

#include "Exception.h"

namespace Alepha::inline Cavorite  ::detail::  pointer_casts
{
	inline namespace exports
	{
		using std::dynamic_pointer_cast;

		class BadDynamicCastError : public virtual std::bad_cast
		{
			private:
				std::type_index source;
				std::type_index target;

			public:
				explicit
				BadDynamicCastError( const std::type_index source, const std::type_index target )
					: source( source ), target( target ) {}

				std::type_index sourceType() const noexcept { return source; }
				std::type_index targetType() const noexcept { return target; }

				const char *what() const noexcept final { return "Bad dynamic cast"; }
		};

		template< typename ResultType, typename InputType >
		std::unique_ptr< ResultType >
		dynamic_pointer_cast( std::unique_ptr< InputType > &&ptr )
		{
			if( not ptr ) return nullptr;

			if( not dynamic_cast< ResultType * >( ptr.get() ) )
			{
				throw BadDynamicCastError( typeid( *ptr ), typeid( ResultType ) );
			}

			// It is not dangerous to make this `release`, as the results get recaptured immediately.
			return std::unique_ptr< ResultType >( dynamic_cast< ResultType * >( ptr.release() ) );
		}
	}
}

namespace Alepha::Cavorite::inline exports::inline pointer_casts
{
	using detail::pointer_casts::exports;
}
