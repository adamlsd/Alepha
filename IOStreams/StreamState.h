static_assert( __cplusplus > 2020'00 );

#pragma once

#include <ios>
#include <exception>

#include <Alepha/Alepha.h>

namespace Alepha::Hydrogen::IOStreams  ::detail::  stream_state
{
	inline namespace exports
	{
		template< typename Tag, typename Type >
		class StreamState;
	}

	template< typename Tag, typename Type >
	class exports::StreamState
	{
		private:
			static auto
			index()
			{
				static const auto rv= std::ios::xalloc();
				return rv;
			}

			static Type *&
			get_ptr( std::ios_base &ios )
			{
				return reinterpret_cast< Type *& >( ios.pword( index() ) );
			}

			static void
			destroy( std::ios_base &ios )
			{
				delete get_ptr( ios );
				get_ptr( ios )= nullptr;
			}

			static void
			callback_impl( const std::ios_base::event event, std::ios_base &ios, const int idx )
			{
				if( idx != index() ) throw std::logic_error( "Wrong index." );

				if( event == std::ios_base::erase_event ) destroy( ios );
				else if( event == std::ios_base::imbue_event )
				{
					// Nothing to do... until I develop locale support.
				}
				else if( event == std::ios_base::copyfmt_event )
				{
					get_ptr( ios )= new Type{ get( ios ) };
				}
			}

			static void
			callback( const std::ios_base::event event, std::ios_base &ios, const int idx ) noexcept
			{
				return callback_impl( event, ios, idx );
			}

			static void
			init( std::ios_base &ios )
			{
				if( not ios.iword( index() ) )
				{
					ios.iword( index() )= 1;
					ios.register_callback( callback, index() );
				}
				auto *&ptr= get_ptr( ios );
				if( not ptr ) ptr= new Type{};
			}

		public:
			static Type &
			get( std::ios_base &ios )
			{
				init( ios );
				return *get_ptr( ios );
			}

			struct Setter
			{
				const Type val;

				friend std::ostream &
				operator << ( std::ostream &os, const Setter &s )
				{
					StreamState::get( os )= s.val;
					return os;
				}
			};
	};
}

namespace Alepha::Hydrogen::IOStreams::inline exports::inline stream_state
{
	using namespace detail::stream_state::exports;
}
