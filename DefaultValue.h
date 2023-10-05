static_assert( __cplusplus > 2020'00 );

#pragma once

#include <optional>

#include "Concepts.h"


namespace Alepha::inline Cavorite  ::detail::  default_value
{
	inline namespace exports {}

	template< typename T, auto default_generator_ >
	class DefaultValue_impl
		: private std::optional< T >
	{
		public:
			struct alepha_defaulted_value_tag;

			static inline const auto default_generator= default_generator_;

			using std::optional< T >::value_type;
			using std::optional< T >::optional;
			using std::optional< T >::has_value;
			using std::optional< T >::operator=;


			T
			value() const
			{
				return this->value_or( default_generator() );
			}
	};

	template< typename T, auto default_generator >
	struct impl_selector
	{
		using type= DefaultValue_impl< T, default_generator >;
	};

	template< typename T, auto default_value >
	requires( ConvertibleTo< std::decay_t< decltype( default_value ) >, T > )
	struct impl_selector< T, default_value >
	{
		using type= DefaultValue_impl< T, [] { return default_value; } >;
	}

	namespace exports
	{
		template< typename T, auto default_value >
		using DefaultValue= typename impl_selector< T, default_value >::type;
	}
}

namespace Alepha::Cavorite::inline exports::inline default_value
{
	using namespace detail::default_value::exports;
}
