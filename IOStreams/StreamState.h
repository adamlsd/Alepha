static_assert( __cplusplus > 2020'00 );

#pragma once

#include <ios>
#include <exception>

#include <Alepha/Alepha.h>

namespace Alepha::Hydrogen::IOStreams  ::detail::  stream_state
{
	inline namespace exports
	{
		template< typename Tag, typename Type, auto Default= [] { return Type{}; } >
		class StreamState;
	}

	template< typename Type >
	class exports::StreamState : boost::noncopyable
	{
		private:
			const int index= std::ios::xalloc();

			static Type *&
			get_ptr( std::ios_base &ios, const int idx )
			{
				return reinterpret_cast< Type *& >( ios.pword( idx ) );
			}

			Type *&
			get_ptr( std::ios_base &ios )
			{
				get_ptr( ios, index );
			}

			static void
			destroy( std::ios_base &ios, const int idx )
			{
				delete get_ptr( ios, idx );
				get_ptr( ios, idx )= nullptr;
			}

			static void
			callback_impl( const std::ios_base::event event, std::ios_base &ios, const int idx )
			{
				if( event == std::ios_base::erase_event ) destroy( ios, idx );
				else if( event == std::ios_base::imbue_event )
				{
					// Nothing to do... until I develop locale support.
				}
				else if( event == std::ios_base::copyfmt_event )
				{
					get_ptr( ios, idx )= new Type{ get( ios, idx ) };
				}
			}

			static void
			callback( const std::ios_base::event event, std::ios_base &ios, const int idx ) noexcept
			{
				return callback_impl( event, ios, idx );
			}

			void
			init( std::ios_base &ios )
			{
				if( not ios.iword( idx ) )
				{
					ios.iword( index() )= 1;
					ios.register_callback( callback, idx );
				}
				auto *&ptr= get_ptr( ios, idx );
				if( not ptr ) ptr= new Type{ Default() };
			}

		public:
			Type &
			get( std::ios_base &ios )
			{
				init( ios );
				return *get_ptr( ios );
			}

			struct Setter
			{
				StreamState *state;
				const Type val;

				friend std::ostream &
				operator << ( std::ostream &os, const Setter &s )
				{
					s.get( os )= s.val;
					return os;
				}

				friend std::istream &
				operator >> ( std::istream &is, const Setter &s )
				{
					s.get( is )= s.val;
					return is;
				}
			};
	};
}

namespace Alepha::Hydrogen::IOStreams::inline exports::inline stream_state
{
	using namespace detail::stream_state::exports;
}
