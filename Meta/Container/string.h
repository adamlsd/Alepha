static_assert( __cplusplus > 201700, "C++17 Required" );

#pragma once

#include <Alepha/Alepha.h>

#include <array>

#include <Alepha/comparisons.h>
#include <Alepha/Constexpr/algorithm.h>
#include <Alepha/Constexpr/array.h>

namespace Alepha::Hydrogen::Meta::Container
{
	inline namespace exports { inline namespace meta_container_string {} }

	namespace detail::meta_container_string
	{
		inline namespace exports {}

		namespace C
		{
			const std::size_t amount= 4096;
		}

		namespace exports
		{
			template< typename CharT > class basic_string;

			template< typename CharT, CharT ... text >
			struct constexpr_str_wrapper
				: comparable
			{
				#define MAKE_OP( op ) \
					template< CharT ... text2 > \
					constexpr friend bool \
					operator op ( const constexpr_str_wrapper< CharT, text... > lhs, const constexpr_str_wrapper< CharT, text2... > rhs ) \
					{ \
						return basic_string< CharT >{ lhs } op basic_string< CharT >{ rhs }; \
					}

				MAKE_OP( < );
				MAKE_OP( > );

				MAKE_OP( <= );
				MAKE_OP( >= );

				MAKE_OP( == );
				MAKE_OP( != );

				#undef MAKE_OP
			};

			template< typename CharT >
			class basic_string
				: public comparable
			{
				private:
					Constexpr::array< CharT, C::amount > raw;

				public:
					template< CharT ... text >
					constexpr explicit
					basic_string( constexpr_str_wrapper< CharT, text... > ) 
						: raw()
					{
						CharT chars[]= { text..., '\0' };
						using std::begin, std::end;
						Constexpr::copy( begin( chars ), end( chars ), begin( raw ) );
					}

					constexpr auto value_lens() const noexcept { return ordering_magma( raw ); }
			};

			using string= basic_string< char >;
		}
	}

	namespace exports::meta_container_string
	{
		using namespace detail::meta_container_string::exports;
	}
}

template< typename CharT, CharT ... text >
struct Alepha::Meta::unique_for_type_binder< Alepha::Meta::type_value< Alepha::Meta::Container::constexpr_str_wrapper< CharT, text... > > >
{
	static constexpr auto
	build() noexcept
	{
		return Alepha::Meta::Container::basic_string{ Alepha::Meta::Container::constexpr_str_wrapper< CharT, text... >{} };
	}
};
