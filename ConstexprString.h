static_assert( __cplusplus > 2020'00 );

#pragma once

#include <cstddef>

#include <array>
#include <type_traits>
#include <algorithm>
#include <iostream>

#include <Alepha/Alepha.h>

namespace Alepha::Hydrogen  ::detail::  constexpr_string
{
	namespace C
	{
		// While a larger size for the maximum might be desirable, this consumes
		// some constant amount of space in the compiler's symbol table.  To be
		// mindful of that, we want to not get TOO large.
		//
		// As compared to the GCC extension which uses a variadic length NTTP
		// list, this may use less overall memory, in some ways.  However, the
		// benefit of the NTTP variadic list way, for compiler memory usage, is that
		// the a type using the string as a parameter has a symbol size which
		// is proportional to the number of chars in the string.
		const std::size_t maxSize= 128;
	}

	inline namespace exports
	{
		struct ConstexprString;

		inline namespace literals
		{
			consteval ConstexprString operator ""_cs( const char *s, std::size_t len );
		}
	}

	struct exports::ConstexprString
	{
		private:
			class BadConstantStringAllocationError
				: public std::bad_alloc
			{
				public:
					const char *
					what() const noexcept final
					{
						return "Failure  to allocate enough space for a a compile-time string.";
					}
			};

		// Pretend that these are private, despite the fat that they are public.
		// They have to be public, to work correctly with `template` parameters.
		public:
			std::array< char, C::maxSize > storage= {};// Always null terminated
			std::size_t length= 0;

			friend consteval ConstexprString literals::operator ""_cs( const char *, std::size_t );

		public:
			constexpr ConstexprString()= default;

			constexpr
			CosntexprString( const char *const s, std::size_t len )
			{
				if( len >= C::maxSize ) throw BadConstantStringAllocationError{};

				std::copy_n( s, len, storage.begin() );
				length= len;
			}

			template< std::size_t N >
			constexpr
			ConstexprString( const char (&s)[ N ] ) : ConstexprString( s, N ) {}


			constexpr bool empty() const noexcept { return length == 0; }
			constexpr std::size_t size() const noexcept { return length; }

			constexpr auto begin() const noexcept { return storage.begin(); }
			constexpr auto end() const noexcept { return storage.begin() + length; }

			constexpr const char *c_str() const noexcept { return &storage[ 0 ]; }
			constexpr const char *data() const noexcept { return &storage[ 0 ]; }
			constexpr char *data() noexcept { return &storage[ 0 ]; }

			friend constexpr ConstexprString
			operator + ( const ConstexprString &lhs, const ConstexprString &rhs )
			{
				ConstexprString rv;

				rv.length= lhs.size() + rhs.size();
				if( rv.length >= C::maxSize ) throw BadConstantStringAllocationError{};

				using std::begin, std::end;
				const auto next= std::copy( begin( lhs ), end( lhs ), begin( rv.storage ) );
				std::copy( begin( rhs ), end( rhs ), next );

				return rv;
			}

			// The C++20 rules that build more operators upon these operators are fine:

			friend constexpr bool
			operator < ( const ConstexprString &lhs, const ConstexprString &rhs ) noexcept
			{
				using std::begin, std::end;
				return std::lexicographical_compare( begin( lhs ), end( lhs ), begin( rhs ), end( rhs ) );
			}

			friend constexpr bool
			operator == ( const ConstexprString &lhs, const ConstexprString &rhs ) noexcept
			{
				using std::begin, std::end;
				return lhs.size() == rhs.size() and std::equal( begin( lhs ), end( lhs ), begin( rhs ) );
			}

			friend std::ostream &
			operator << ( std::ostream &os, const ConstexprString &rhs )
			{
				return os << rhs.c_str();
			}

			// Total order is not helpful, here.
			void operator<=>( ConstexprString )= delete;
	};

	consteval ConstexprString
	exports::literals::operator ""_cs( const char *const s, const std::size_t len )
	{
		return ConstexprString( s, len );
	}
}

namespace Alepha::Cavorite::inline exports::inline constexpr_string
{
	using namespace detail::constexpr_string::exports;
}

namespace Alepha::Cavorite::inline exports::inline literals::inline constexpr_string_literals
{
	using namespace detail::constexpr_string::exports::literals;
}
