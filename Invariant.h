static_assert( __cplusplus > 2020'00 );

#pragma once

#include <boost/noncopyable.h>

#include "assertion.h"

namespace Alepha::inline Cavorite  ::detail::  invariant
{
	inline namespace exports
	{
		template< typename Class, auto selector > class Invariant;
	}

	struct nil_selector {};

	template< typename Class, auto selector= nil_selector >
	class Invariant : boost::noncopyable
	{
		private:
			#ifdef NDEBUG
			void check() const {}
			#else
			const Class *const checked;

			bool
			invariant() const
			{
				if constexpr( std::is_same_v< nil_selector, std::decay_t< decltype( selector ) > > )
				{
					return this->checked->invariant();
				}
				else
				{
					return this->checked->invariant( selector );
				}
			}

			void check() const { assertion( this->invariant() ); }
			#endif

		public:
			~Invariant() { this->check(); }

			#ifdef NDEBUG
			explicit Invariant( const Class *const checked ) {}
			#else
			explicit Invariant( const Class *const checked ) : checked( checked ) { this->check(); }
			#endif
	};
}

namespace Alepha::Cavorite::inline exports::inline invariant
{
	using namespace detail::invariant::exports;
}
