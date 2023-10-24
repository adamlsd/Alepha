static_assert( __cplusplus > 2020'00 );

#include <cstdint>
#include <string>
#include <vector>
#include <map>
#include <iostream>
#include <iomanip>

#include <boost/core/demangle.hpp>

#include <Alepha/Testing/test.h>
#include <Alepha/Testing/TableTest.h>
#include <Alepha/Utility/evaluation.h>
#include <Alepha/comparisons.h>

namespace
{
	using namespace Alepha::Testing::exports;
	using namespace Alepha::Utility::exports::evaluation;
	using namespace Alepha::exports::types;

	using hash_type= std::uint64_t;

	namespace C
	{
		namespace best_constants_candidate
		{
			// These constants seem to produce a nice distribution
			const std::uint64_t spinBase= 7;
			const std::uint64_t spinRate= 191;
			const int maxRounds= 40;
			const std::uint64_t indexLimit= ( 1ull << 47 ) - 1;
		}

		namespace experimental_constants
		{
			using namespace best_constants_candidate;
			const std::uint64_t spinRate= 379;
			const int maxRounds= 20;
		}

		namespace weak_constants
		{
			using namespace best_constants_candidate;
			//const std::uint64_t spinBase= 9;
			//const std::uint64_t spinRate= 0;
			//const int maxRounds= 25;
		}

		namespace second_experiment
		{
			using namespace best_constants_candidate;
			const int maxRounds= 30;
			const std::uint64_t spinBase= 5;
		}

		using namespace best_constants_candidate;
	}

	enum { Unit };

	template< typename T >
	constexpr const char *
	typesig()
	{
		const char *p= __PRETTY_FUNCTION__;
		while( *p++ != '[' );
		return p + 5 + 4;
	}

	[[nodiscard]] constexpr std::uint64_t
	rotl( const std::uint64_t value, const std::uint64_t amt )
	{
		if( amt >= 64 ) return rotl( value, amt % 64 );
		if( amt == 0 ) return value;
		return ( value << amt ) | ( value >> 64 - amt );
	}

	constexpr hash_type
	computeHash( const char *str )
	{
		std::uint64_t rv= 0;

		unsigned index= 0;
		for( ; *str; ++str )
		{
			//++index;
			for( int i= 0; i < C::maxRounds; ++i )
			{
				index+= C::spinRate;
				index%= C::indexLimit;
				std::uint64_t ch= *str; //|0xFF'FF'FF'FF'FF'FF'FF'00;
				rv= rotl( rv, index + C::spinBase );

				// engage the carry only occasionally, to help with bit-mixing
				if( false ) ;
				else if( not( i % 11 ) or not( i % 17 ) ) rv+= ch;
				else rv^= ch;
			}
		}

		return rv;
	}

	template< typename T >
	constexpr auto
	computeTypeHash()
	{
		return computeHash( typesig< T >() );
	}

	auto
	printHash( const std::string &s )
	{
		std::cout << std::hex << std::setw( 16 ) << std::setfill( '0' ) << (std::uint64_t) computeHash( s.c_str() ) << ": " << s;
		return Unit;
	}

	struct abc {};
	struct abd {};
	struct abcd {};

	auto tests= enroll <=[]
	{
		"simple.case"_test <= TableTest< printHash >::Cases
		{
			{ "nothing", { "" }, Unit },
			{ "cab", { "cab" }, Unit },
			{ "cal", { "cal" }, Unit },
			{ "cam", { "cam" }, Unit },
			{ "can", { "can" }, Unit },
			{ "car", { "car" }, Unit },
			{ "cat", { "cat" }, Unit },
			{ "hat", { "hat" }, Unit },
			{ "hut", { "hut" }, Unit },
			{ "dog", { "dog" }, Unit },
			{ "pie", { "pie" }, Unit },
			{ "spoon", { "spoon" }, Unit },
			{ "frank", { "frank" }, Unit },
			{ "fronk", { "fronk" }, Unit },

			{ "typesig< void >", { typesig< void >() }, Unit },
			{ "typesig< char >", { typesig< char >() }, Unit },

			{ "typesig< int >", { typesig< int >() }, Unit },
			{ "typesig< unsigned >", { typesig< unsigned >() }, Unit },
			{ "typesig< signed >", { typesig< signed >() }, Unit },
			{ "typesig< float >", { typesig< float >() }, Unit },
			{ "typesig< double >", { typesig< double >() }, Unit },
			{ "typesig< long double >", { typesig< long double >() }, Unit },
			{ "typesig< std::vector< int > >", { typesig< std::vector< int > >() }, Unit },
			{ "typesig< short >", { typesig< short >() }, Unit },
			{ "typesig< unsigned short >", { typesig< unsigned short >() }, Unit },
			{ "typesig< unsigned char >", { typesig< unsigned char >() }, Unit },
			{ "typesig< signed char >", { typesig< signed char >() }, Unit },
			{ "typesig< int * >", { typesig< int * >() }, Unit },
			{ "typesig< int >", { typesig< int >() }, Unit },
			{ "typesig< std::string >", { typesig< std::string >() }, Unit },
			{ "typesig< std::vector< std::string > >", { typesig< std::vector< std::string > >() }, Unit },

			{ "typesig< abc >", { typesig< abc >() }, Unit },
			{ "typesig< abd >", { typesig< abd >() }, Unit },
			{ "typesig< abcd >", { typesig< abcd >() }, Unit },
		};
	};

	template< hash_type > struct type_registry;

#if 0
	template<>
	struct type_registry< computeTypeHash< std::string >() >
	{
		using type= std::string;
	};
#endif

	template< std::uint64_t val >
	struct hash_hook
	{
		template< typename T >
		friend constexpr auto hash_lookup( hash_hook, T );
	};


	template< typename T >
	struct register_type
	{
		template< typename X >
		friend constexpr auto
		hash_lookup( hash_hook< computeTypeHash< T >() >, X )
		{
			return Alepha::Meta::type_value< T >{};
		}

		using type= void;
	};

	template< hash_type val >
	struct type_registry
	{
		using type= typename decltype( hash_lookup( hash_hook< val >{}, nullptr ) )::type;
	};

	//using registration= register_type< int >::type;

	template< typename T >
	constexpr auto
	calculateTypeHash()
	{
		using registration= typename register_type< T >::type;
		return computeTypeHash< T >();
	}

	template< typename T >
	constexpr bool verify_registry= std::is_same_v< typename type_registry< calculateTypeHash< T >() >::type, T >;

	auto registry_test= "registry"_test <=[]
	{
		static_assert( std::is_same_v< type_registry< calculateTypeHash< std::string >() >::type, std::string > );

		static_assert( std::is_same_v< type_registry< calculateTypeHash< int >() >::type, int > );

		static_assert( std::is_same_v< type_registry< calculateTypeHash< char >() >::type, char > );
		static_assert( std::is_same_v< type_registry< calculateTypeHash< unsigned char >() >::type, unsigned char > );

		static_assert( verify_registry< long > );
		static_assert( verify_registry< unsigned long > );
		static_assert( verify_registry< unsigned long long > );
		static_assert( verify_registry< signed long long > );
		static_assert( verify_registry< double > );
		static_assert( verify_registry< long double > );
		static_assert( verify_registry< float > );

		static_assert( verify_registry< std::vector< int * > > );
		static_assert( verify_registry< void > );
		static_assert( verify_registry< unsigned char > );
		static_assert( verify_registry< signed char > );
		static_assert( verify_registry< short > );
		static_assert( verify_registry< unsigned short > );

		static_assert( verify_registry< std::function< void( int, long, std::string & ) > > );

#ifndef DISABLE
		static_assert( verify_registry< std::array< int, 1 > > );
		static_assert( verify_registry< std::array< int, 2 > > );
		static_assert( verify_registry< std::array< int, 3 > > );
		static_assert( verify_registry< std::array< int, 4 > > );
		static_assert( verify_registry< std::array< int, 5 > > );
		static_assert( verify_registry< std::array< int, 6 > > );
		static_assert( verify_registry< std::array< int, 7 > > );
		static_assert( verify_registry< std::array< int, 8 > > );
		static_assert( verify_registry< std::array< int, 9 > > );
		static_assert( verify_registry< std::array< int, 10 > > );
		static_assert( verify_registry< std::array< int, 11 > > );
		static_assert( verify_registry< std::array< int, 12 > > );
		static_assert( verify_registry< std::array< int, 13 > > );
		static_assert( verify_registry< std::array< int, 14 > > );
		static_assert( verify_registry< std::array< int, 15 > > );
		static_assert( verify_registry< std::array< int, 16 > > );
		static_assert( verify_registry< std::array< int, 17 > > );
		static_assert( verify_registry< std::array< int, 18 > > );
		static_assert( verify_registry< std::array< int, 19 > > );
		static_assert( verify_registry< std::array< int, 20 > > );
#endif

		static_assert( verify_registry< std::array< int, 1 > > );
		static_assert( verify_registry< std::array< int, 2 > > );
		static_assert( verify_registry< std::array< int, 3 > > );
		static_assert( verify_registry< std::array< int, 4 > > );
		static_assert( verify_registry< std::array< int, 5 > > );
		static_assert( verify_registry< std::array< int, 6 > > );
		static_assert( verify_registry< std::array< int, 7 > > );
		static_assert( verify_registry< std::array< int, 8 > > );
		static_assert( verify_registry< std::array< int, 9 > > );
		static_assert( verify_registry< std::array< int, 10 > > );
		static_assert( verify_registry< std::array< int, 11 > > );
		static_assert( verify_registry< std::array< int, 12 > > );
		static_assert( verify_registry< std::array< int, 13 > > );
		static_assert( verify_registry< std::array< int, 14 > > );
		static_assert( verify_registry< std::array< int, 15 > > );
		static_assert( verify_registry< std::array< int, 16 > > );
		static_assert( verify_registry< std::array< int, 17 > > );
		static_assert( verify_registry< std::array< int, 18 > > );
		static_assert( verify_registry< std::array< int, 19 > > );
		static_assert( verify_registry< std::array< int, 20 > > );
	};

	enum class TypeNTTP : hash_type {};

	template< typename T >
	struct nttp_maker
	{
		static_assert( verify_registry< T > );

		static constexpr auto value= TypeNTTP( calculateTypeHash< T >() );
	};

	template< typename T >
	constexpr auto make_nttp_v= nttp_maker< T >::value;

	template< TypeNTTP type_val >
	using type_from_nttp_t= typename type_registry< hash_type( type_val ) >::type;

	inline std::ostream &
	operator << ( std::ostream &os, const TypeNTTP &val )
	{
		return os << "Type Code: " << std::hex << std::setw( 16 ) << std::setfill( '0' ) << (std::uint64_t) hash_type( val );
		// With some more work, we can make a runtime code registry, if desired.
		// But the codes are *NOT* portable!
		//<< " which maps to " << typeid( type_from_nttp_t< val
	}

	auto type_nttp_test= "nttp"_test <=[]
	{
		std::cout << "NTTP of int: " << make_nttp_v< int > << std::endl;

		constexpr auto nttpVal= make_nttp_v< std::map< std::vector< std::string >, std::function< void( int, long, short ) > > >;

		std::cout << "type: " << boost::core::demangle( typeid( type_from_nttp_t< nttpVal > ).name() ) << std::endl;
	};

	template< typename ... Types >
	constexpr auto type_array= std::array< TypeNTTP, sizeof...( Types ) >{ make_nttp_v< Types >... };

	template< TypeNTTP ... type_codes >
	using typeset= std::tuple< type_from_nttp_t< type_codes >... >;

	template< typename T >
	constexpr T *
	max_element( const T *const first, const T *const last )
	{
		T *largest= first;
		for( T *const pos= first; pos < last; ++pos )
		{
			if( *pos > *largest ) largest= pos;
		}
		return largest;
	}

	template< typename Container >
	constexpr void
	sort( Container &array )
	{
		for( auto pos= begin( array ); pos != end( array ); ++pos )
		{
			const auto largest= max_element( pos, end( array ) );
			auto tmp= std::move( *largest );
			*largest= std::move( *pos );
			*pos= std::move( tmp );
		}
	}
}
