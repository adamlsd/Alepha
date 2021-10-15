static_assert( __cplusplus > 201700, "C++17 Required" );

#pragma once

#include <Alepha/Alepha.h>

#include <tuple>
#include <utility>

namespace Alepha::Hydrogen::Utility
{
	inline namespace exports { inline namespace evaluation {} }

	namespace detail::evaluation
	{
		struct evaluate_t {};
		struct enroll_t {};
		struct lambaste_t {};

		inline namespace exports
		{
			inline constexpr evaluate_t evaluate;
			inline constexpr enroll_t enroll;
			inline constexpr lambaste_t lambaste;
		}

		template< typename Function >
		decltype( auto )
		operator <=( evaluate_t, Function &&init )
		{
			return std::forward< Function >( init )();
		}

		template< typename Init >
		auto
		operator <=( enroll_t, Init init )
		{
			struct {} registration;

			(void) ( evaluate <=init );

			return registration;
		}

		template< typename Value >
		auto
		operator <=( lambaste_t, Value value )
		{
			return [value]{ return value; };
		}
	}

	namespace exports::evaluation
	{
		using namespace detail::evaluation::exports;
	}
}
