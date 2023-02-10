static_assert( __cplusplus > 2020'00 );

#pragma once

#include <Alepha/Alepha.h>

#include <type_traits>

namespace Alepha::Hydrogen::Meta
{
	inline namespace exports { inline namespace type_value_wrapper {} }

	namespace detail::type_value_wrapper
	{
		inline namespace exports
		{
			template< typename Type >
			struct type_value { using type= Type; };

			template< template< typename > class Trait, typename Value >
			constexpr bool
			check_trait( type_value< Value > )
			{
				return Trait< Value >::value;
			}

			template< template< typename, typename > class Trait, typename A, typename B >
			constexpr bool
			check_trait( type_value< A >, type_value< B > )
			{
				return Trait< A, B >::value;
			}

			template< typename Lhs, typename Rhs >
			constexpr bool
			operator == ( type_value< Lhs > lhs, type_value< Rhs > rhs )
			{
				return check_trait< std::is_same >( lhs, rhs );
			}

			template< typename Lhs, typename Rhs >
			constexpr bool
			operator != ( type_value< Lhs > lhs, type_value< Rhs > rhs )
			{
				return not( lhs == rhs );
			}

			template< typename T >
			constexpr auto
			make_value_type( T&& )
			{
				return type_value< std::decay_t< T > >{};
			}

			template< template< typename ... > class Trait > struct trait;

			template< template< typename > class Trait >
			struct trait< Trait >
			{
				template< typename Type >
				constexpr bool
				operator() ( type_value< Type > ) const
				{
					return Trait< Type >::value;
				}
			};

			template< template< typename, typename > class Trait >
			struct trait< Trait >
			{
				template< typename A, typename B >
				constexpr bool
				operator() ( type_value< A >, type_value< B > ) const
				{
					return Trait< A, B >::value;
				}
			};

			inline constexpr trait< std::is_base_of > is_base_of;
			inline constexpr trait< std::is_same > is_same;
			inline constexpr trait< std::is_default_constructible > is_default_constructible;
		}
	}

	namespace exports::type_value_wrapper
	{
		using namespace detail::type_value_wrapper::exports;
	}
}
