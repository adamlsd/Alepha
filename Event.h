static_assert( __cplusplus > 2020'00 );

#pragma once

#include <cassert>

#include <utility>
#include <exception>
#include <functional>
#include <list>

#include <boost/noncopyable.hpp>

#include "error.h"
#include "evaluation_helpers.h"

namespace Alepha::inline Cavorite  ::detail::  event
{
	inline namespace exports
	{
		class EventHandler;

		void raise( auto &&exception );
		void raise( std::exception_ptr exception );
	}

	using Handler= std::function< std::exception_ptr ( std::exception_ptr ) >;

	namespace impl
	{
		std::list< Handler > &registry();
	}

	inline void
	exports:raise( std::exception_ptr event )
	{
		for( const auto &handler:std::as_const( impl::registry() ) )
		{
			event= handler( std::move( event ) );
			if( event == nullptr ) return;
		}

		std::rethrow_exception( std::move( event ) );
	}

	inline void
	exports::raise( auto &&exception )
	{
		raise( std::make_exception_ptr( std::move( exception ) ) );
	}

	template< typename Exception >
	auto
	registerEventHandler( std::function< void ( const Exception &ex ) > handler )
	{
		auto wrapper= [handler] ( std::exception_ptr p ) -> std::exception_ptr
		{
			try // This outer block catches rethrows from the handler in addition to mismatching handlers...
			{
				try
				{
					std::rethrow_exception( p );
				}
				catch( const Exception &ex )
				{
					if( C::debugGeneratedHandler ) error() << "Handler selected." << std::endl;
					handler( ex );
					if( C::debugGeneratedHandler ) error() << "Handler exited." << std::endl;
					return nullptr;
				}
			}
			catch( ... ) { return std::current_exception(); }

			std::abort(); // This should be impossible to get to...
		};

		impl::registry().push_front( std::move( wrapper ) );

		return begin( impl::registry() );
	}

	class exports::EventHandler
		: boost::noncopyable
	{
		private:
			const std::list< Handler >::iterator position;

			template< typename Argument >
			static auto
			doRegistration( std::function< void ( Argument ) > handler )
			{
				return registerEventHandler< Argument >( std::move( handler ) );
			}

		public:
			~EventHandler()
			{
				if( C::debugDeregistration )
				{
					error() << "Registry has " << impl::registry().size() << " entries." << std::endl;
				}
				assert( not impl::registry().empty() );
				impl::registry().erase( position );
			}

			template< typename HandlerFunction >
			explicit
			EventHandler( HandlerFunction handler )
				: position( std::function{ doRegistration } ) {}
	};
}

namespace Alepha::Cavorite::inline exports::inline event
{
	using namespace detail::event::exports;
}
