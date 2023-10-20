static_assert( __cplusplus > 2020'00 );

#include "StackableStreambuf.h"

namespace Alepha::Hydrogen::Utility::detail::stackable_streambuf
{
	namespace
	{
		const auto index= std::ios::xalloc();

		inline auto &
		getStack( std::ios_base &ios )
		{
			auto &ownership= reinterpret_cast< std::stack< std::unique_ptr< StackableStreambuf > > *& >( ios.pword( index ) );
			if( not ownership ) ownership= new std::decay_t< decltype( *ownership ) >{};

			return *ownership;
		}

		enum { token };

		inline bool
		releaseTop( std::ios_base &ios, decltype( token )= token )
		{
			auto &ownership= getStack( ios );

			// Since it's owned, delete happens at scope exit.
			const std::unique_ptr top= std::move( ownership.top() );
			ownership.pop();

			top->drain();
			return not ownership.empty();
		}

		inline bool
		releaseTop( std::ostream &os )
		{
			const auto *const current= dynamic_cast< StackableStreambuf * >( os.rdbuf() );
			if( not current ) return false;

			os.rdbuf( current->underlying );

			releaseTop( os, token );

			return true;
		}

		inline void
		releaseStack( std::ios_base &ios )
		{
			auto &os= dynamic_cast< std::ostream & >( ios );

			while( releaseTop( os ) );
		}

		void
		iosCallback( const std::ios_base::event event, std::ios_base &ios, const int idx )
		{
			#pragma GCC diagnostic push
			#pragma GCC diagnostic ignored "-Wterminate"

			if( index != idx ) throw std::logic_error{ "Wrong index." };

			if( event == std::ios_base::erase_event ) releaseStack( ios );
			else if( event == std::ios_base::imbue_event ) {}
			else if( event == std::ios_base::copyfmt_event ) throw std::runtime_error{ "Can't copy?" };

			#pragma GCC diagnostic pop
		}
	}

	std::ostream &
	impl::operator << ( std::ostream &os, PopStack )
	{
		if( not releaseTop( os ) ) throw std::logic_error( "OStream has no stacked streambufs!" );
		return os;
	}

	StackableStreambuf::~StackableStreambuf() {}

	StackableStreambuf::StackableStreambuf( std::ostream &host )
		: underlying( host.rdbuf( this ) )
	{
		// TODO: Atomicity for this:
		if( not host.iword( index ) ) host.register_callback( iosCallback, index );
		host.iword( index )= 1;
		getStack( host ).emplace( this );
	}

	int
	StackableStreambuf::overflow( const int ch )
	{
		if( ch == EOF ) throw std::logic_error( "EOF!" );
		writeChar( ch );

		return 1;
	}

	std::streamsize
	StackableStreambuf::xsputn( const char *const data, const std::streamsize amt )
	{
		for( std::streamsize i= 0; i< amt; ++i ) overflow( data[ i ] );
		return amt;
	}
}
