static_assert( __cplusplus > 2020'00 );

#pragma once

#include <Alepha/Alepha.h>

#include <functional>
#include <type_traits>

#include <boost/noncopyable.hpp>

namespace Alepha::Hydrogen
{
	inline namespace exports { inline namespace auto_raii {} }

	namespace detail::auto_raii
	{
		inline namespace exports
		{
			template< typename T, typename Dtor= std::function< void ( T ) > >
			class AutoRAII : boost::noncopyable
			{
				private:
					Dtor dtor;
					T value;

				public:
					~AutoRAII()
					{
						if constexpr( std::is_same_v< Dtor, std::function< void ( T ) > > )
						{
							if( dtor == nullptr ) return;
						}
						dtor( value );
					}

					template< typename Ctor >
					explicit AutoRAII( Ctor ctor, Dtor dtor ) : dtor( std::move( dtor ) ), value( ctor() ) {}

					operator const T &() const { return value; }

					decltype( auto )
					operator *() const
					requires( std::is_pointer_v< T > )
					{
						return *value;
					}
					
					decltype( auto )
					operator->() const
					requires( std::is_pointer_v< T > )
					{
						return value;
					}
			};

			template< typename Dtor >
			class AutoRAII< void, Dtor > : boost::noncopyable
			{
				private:
					Dtor dtor;

				public:
					~AutoRAII()
					{
						if constexpr( std::is_same_v< Dtor, std::function< void () > > )
						{
							if( dtor == nullptr ) return;
						}
						dtor();
					}

					template< typename Ctor >
					explicit AutoRAII( Ctor ctor, Dtor dtor ) : dtor( std::move( dtor ) ) { ctor(); }
			};

			template< typename Ctor, typename Dtor >
			explicit AutoRAII( Ctor ctor, Dtor ) -> AutoRAII< decltype( ctor() ), Dtor >;
		}
	}

	namespace exports::auto_raii
	{
		using namespace detail::auto_raii::exports;
	}
}
