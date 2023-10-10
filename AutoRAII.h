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

					template
					<
						typename AutoRAII_= AutoRAII,
						typename= std::enable_if_t< std::is_same_v< AutoRAII_, AutoRAII > >,
						typename= std::enable_if_t< std::is_function_v< decltype( AutoRAII_::dtor ) > >
					>
					AutoRAII( AutoRAII &&move )
						: value()
					{
						std::swap( move.dtor, this->dtor );
						std::swap( move.value, this->value );
					}

					template< typename Ctor >
					explicit AutoRAII( Ctor ctor, Dtor dtor ) : dtor( std::move( dtor ) ), value( ctor() ) {}

					template
					<
						typename AutoRAII_= AutoRAII,
						typename= std::enable_if_t< std::is_same_v< AutoRAII_, AutoRAII > >,
						typename= std::enable_if_t< std::is_function_v< decltype( AutoRAII_::dtor ) > >
					>
					AutoRAII &operator= ( AutoRAII_ &&move )
					{
						std::swap( move.dtor, this->dtor );
						std::swap( move.value, this->value );
					}

					operator const T &() const { return value; }

					template
					<
						typename T_= T,
						typename= std::enable_if_t< std::is_pointer_v< T_ > >
					>
					decltype( auto ) operator *() const { return *value; }
					
					template
					<
						typename T_= T,
						typename= std::enable_if_t< std::is_pointer_v< T_ > >
					>
					decltype( auto ) operator->() const { return value; }
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
