static_assert( __cplusplus > 2020'00 );

#pragma once

#include <cassert>

#include <tuple>
#include <string>
#include <exception>
#include <stdexcept>
#include <iostream>
#include <variant>

#include <boost/core/demangle.hpp>

#include "Concepts.h"
#include "ConstexprString.h"
#include "meta.h"

namespace Alepha::inline Cavorite  ::detail:: enhanced_enum
{
	inline namespace exports {}

	using namespace std::literals::string_literals;

	namespace C
	{
		const bool debug= false;
		const bool debugConstruction= false or C::debug;
		const bool debugCheck= false or C::debug;
		const bool debugConversion= false or C::debug;
	}

	namespace exports
	{
		inline constexpr std::string
		buildAllNames( const std::initializer_list< ConstexprString > &allStrings )
		{
			std::string rv;

			bool first= true;
			for( const auto &str: allStrings )
			{
				if( not first ) rv+= ", ";
				first= false;
				rv+= str.c_str();
			}

			return rv;
		}
	}

	template< auto value, auto ... >
	constexpr decltype( auto )
	get_first_enum_string()
	{
		return value;
	}

	template< ConstexprString s >
	struct EnumValueString
	{
		consteval bool operator == ( EnumValueString ) const { return true; }

		template< ConstexprString alien >
		consteval bool operator == ( EnumValueString< alien > ) const { return false; }

		consteval static ConstexprString cs_string() noexcept { return s; }

		friend std::ostream &
		operator << ( std::ostream &os, const EnumValueString & )
		{
			return os << s;
		}
	};

	template< ConstexprString s, EnumValueString ... reqs >
	concept RequireMatch= ( ... || ( s == reqs.cs_string() ) );

	template< ConstexprString s, EnumValueString ... reqs >
	concept PreventMatch= not RequireMatch< s, reqs... >;

	namespace exports
	{
		inline namespace literals
		{
			template< ConstexprString s >
			consteval auto
			operator ""_value ()
			{
				return EnumValueString< s >{};
			}
		}

		class EnumTextMismatchError : virtual public std::runtime_error
		{
			public:
				using std::runtime_error::runtime_error;

				virtual std::vector< std::string > expectedValues() const= 0;
		};

		template< EnumValueString ... Values > class Enum;
	}

	template< EnumValueString ... values >
	class SpecificEnumTextMismatchError
		: public virtual EnumTextMismatchError
	{
		public:
			using EnumTextMismatchError::EnumTextMismatchError;

			std::vector< std::string >
			expectedValues() const final
			{
				return std::vector< std::string >{ ( values.cs_string().c_str() )... };
			}
	};

	template< EnumValueString ... values >
	class exports::Enum
		//: private comparable
		// Alepha::Hydrogen comparables work differently than the one-off Cavorite form.
	{
		private:
			enum class StrictInteger : unsigned {};

			StrictInteger value= static_cast< StrictInteger >( 0 );

		public:
			static constexpr std::string name() { return buildAllNames( { ( values.cs_string() )... } ); }

			static constexpr bool accepts( const std::string &s ) { return ( ... || ( values.cs_string().c_str() == s ) ); }

			constexpr Enum()= default;

			template< ConstexprString cs_str >
			requires( RequireMatch< cs_str, values... > )
			constexpr
			Enum( EnumValueString< cs_str > )
			{
				if( ( ... && ( cs_str != values.cs_string() ) ) )
				{
					throw std::logic_error( "An enumeration value was specified which is not supported by this `Enum`." );
				}

				unsigned value= 0;
				for( const auto &next: { ( values.cs_string() )... } )
				{
					if( next == cs_str ) break;
					++value;
				}

				set_index( value );
			}

			// ===============================================
			// ==========      IMPORANT NOTE      ============
			// ===============================================
			//
			// If your compiler gave you an error message that you've called this
			// deleted function, it's almost certainly because you are attempting
			// to assign or otherwise work with an enumerate value which is not
			// supported by the Enhanced Enum in question.
			//
			// Here are some examples.
			//
			// E.g.: `Enum< "1"_value > x= "0"_value;`.
			// E.g.: `Enum< "1"_value >{ "1"_value } == "0"_value`
			//
			// Unfortunately, putting a `static_assert` within the body of
			// this function (for the purpose of providing a nice error message),
			// would make a call to it "well formed", and thus SFINAE would
			// conclude that such a conversion was legal.  However, if we don't
			// explicitly delete it, the error message is more confusing.
			//
			// Thankfully, most compilers will point you directly at the attempted
			// deleted call (which is this call, in this case), and thus a suitable
			// helpful comment (such as this one) can hopefully give you some guidance.
			template< ConstexprString cs_str >
			requires( PreventMatch< cs_str, values... > )
			constexpr Enum( EnumValueString< cs_str > )= delete; // PROBABLE ILLEGAL ENUM USAGE.  SEE THE NEARBY COMMENTS IN THIS FILE!
			// IMPORTANT:
			//
			// IF YOU GOT HERE FROM A COMPILER ERROR MESSAGE
			// THEN PLEASE SEE THE NOTE ABOVE THIS FUNCTION!!!

			constexpr void
			set_index( const unsigned index )
			{
				if( index >= sizeof...( values ) ) throw std::runtime_error{ "Invalid index set on `Enum`." };

				value= static_cast< StrictInteger >( index );
			}

			constexpr unsigned get_index() const { return static_cast< unsigned >( value ); }

			constexpr auto equality_lens() const { return value; }

			bool operator == ( const Enum & ) const= default;

			friend std::ostream &
			operator << ( std::ostream &os, const Enum &rhs )
			{
				int idx= 0;

				for( const auto &value: { ( values.cs_string() )... } )
				{
					if( idx++ == int( rhs.value ) ) return os << value;
				}

				throw std::logic_error{ "Invalid index detected on `Enum`." };
			}

			friend std::istream &
			operator >> ( std::istream &is, Enum &rhs )
			{
				std::string input;
				is >> input;
				int idx= 0;
				for( const auto &value: { ( values.cs_string() )... } )
				{
					if( input == value.c_str() )
					{
						rhs.value= static_cast< StrictInteger >( idx );
						return is;
					}

					++idx;
				}

				throw SpecificEnumTextMismatchError< values... >( "Invalid argument (`" + input + "`), expected one of {" + rhs.name() + "}" );
			}
	};

	namespace exports
	{
		template< auto ... values >
		constexpr std::string buildAllNames( Enum< values... > ) { return Enum< values... >::name(); }

		template< typename T >
		constexpr bool is_enum_v= false;

		template< EnumValueString ... values >
		constexpr bool is_enum_v< Enum< values... > >{ true };
	}
}

namespace Alepha::Cavorite::inline exports::inline enhanced_enum
{
	using namespace detail::enhanced_enum::exports;
}

namespace Alepha::Cavorite::inline exports::inline literals::inline enum_literals
{
	using namespace detail::enhanced_enum::exports::literals;
}
