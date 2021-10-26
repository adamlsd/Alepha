static_assert( __cplusplus > 201700, "C++17 Required" );

#include <Alepha/Meta/is_sequence.h>
#include <Alepha/Meta/is_streamable.h>
#include <Alepha/Meta/type_value.h>
#include <Alepha/Meta/Container/vector.h>

#include <Alepha/Testing/test.h>

#include <Alepha/Utility/evaluation.h>

#include <cassert>

int
main( const int argcnt, const char *const *const argvec )
{
	return Alepha::Testing::runAllTests( argcnt, argvec );
}

namespace
{
	using namespace Alepha::Utility::evaluation;
	using namespace Alepha::Testing::literals;

	using std::begin, std::end;

	// These tests never actually fail at runtime, but they provide a simple way to have them
	// as unit tests.  There's no call to actually assert them at runtime.  If this test built,
	// it passes.
	auto tests= enroll <=[]
	{
		"meta.specific"_test <=[]
		{
			static_assert( Alepha::Meta::is_vector_v< std::vector< int > > );
			static_assert( Alepha::Meta::is_list_v< std::list< int > > );
			static_assert( Alepha::Meta::is_forward_list_v< std::forward_list< int > > );
			static_assert( Alepha::Meta::is_deque_v< std::deque< int > > );

			static_assert( Alepha::Meta::is_vector_v< std::vector< std::string > > );
			static_assert( Alepha::Meta::is_list_v< std::list< std::string > > );
			static_assert( Alepha::Meta::is_forward_list_v< std::forward_list< std::string > > );
			static_assert( Alepha::Meta::is_deque_v< std::deque< std::string > > );

			static_assert( Alepha::Meta::is_string_v< std::string > );
			static_assert( Alepha::Meta::is_string_v< std::wstring > );
		};

		"meta.sequence"_test <=[]
		{
			static_assert( Alepha::Meta::is_sequence_v< std::vector< int > > );
			static_assert( Alepha::Meta::is_sequence_v< std::list< int > > );
			static_assert( Alepha::Meta::is_sequence_v< std::forward_list< int > > );
			static_assert( Alepha::Meta::is_sequence_v< std::deque< int > > );

			static_assert( Alepha::Meta::is_sequence_v< std::vector< std::string > > );
			static_assert( Alepha::Meta::is_sequence_v< std::list< std::string > > );
			static_assert( Alepha::Meta::is_sequence_v< std::forward_list< std::string > > );
			static_assert( Alepha::Meta::is_sequence_v< std::deque< std::string > > );

			static_assert( Alepha::Meta::is_sequence_v< std::string > );
			static_assert( Alepha::Meta::is_sequence_v< std::wstring > );
		};

		"meta.confusion"_test <=[]
		{
			static_assert( not Alepha::Meta::is_sequence_v< int > );
			static_assert( not Alepha::Meta::is_sequence_v< std::nullptr_t > );

			static_assert( not Alepha::Meta::is_vector_v< std::list< int > > );
			static_assert( not Alepha::Meta::is_vector_v< std::forward_list< int > > );
			static_assert( not Alepha::Meta::is_vector_v< std::deque< int > > );
			static_assert( not Alepha::Meta::is_vector_v< std::list< std::string > > );
			static_assert( not Alepha::Meta::is_vector_v< std::forward_list< std::string > > );
			static_assert( not Alepha::Meta::is_vector_v< std::deque< std::string > > );
			static_assert( not Alepha::Meta::is_vector_v< std::string > );
			static_assert( not Alepha::Meta::is_vector_v< std::wstring > );

			static_assert( not Alepha::Meta::is_list_v< std::vector< int > > );
			static_assert( not Alepha::Meta::is_list_v< std::forward_list< int > > );
			static_assert( not Alepha::Meta::is_list_v< std::deque< int > > );
			static_assert( not Alepha::Meta::is_list_v< std::vector< std::string > > );
			static_assert( not Alepha::Meta::is_list_v< std::forward_list< std::string > > );
			static_assert( not Alepha::Meta::is_list_v< std::deque< std::string > > );
			static_assert( not Alepha::Meta::is_list_v< std::string > );
			static_assert( not Alepha::Meta::is_list_v< std::wstring > );

			static_assert( not Alepha::Meta::is_forward_list_v< std::vector< int > > );
			static_assert( not Alepha::Meta::is_forward_list_v< std::list< int > > );
			static_assert( not Alepha::Meta::is_forward_list_v< std::deque< int > > );
			static_assert( not Alepha::Meta::is_forward_list_v< std::vector< std::string > > );
			static_assert( not Alepha::Meta::is_forward_list_v< std::list< std::string > > );
			static_assert( not Alepha::Meta::is_forward_list_v< std::deque< std::string > > );
			static_assert( not Alepha::Meta::is_forward_list_v< std::string > );
			static_assert( not Alepha::Meta::is_forward_list_v< std::wstring > );

			static_assert( not Alepha::Meta::is_deque_v< std::vector< int > > );
			static_assert( not Alepha::Meta::is_deque_v< std::list< int > > );
			static_assert( not Alepha::Meta::is_deque_v< std::forward_list< int > > );
			static_assert( not Alepha::Meta::is_deque_v< std::vector< std::string > > );
			static_assert( not Alepha::Meta::is_deque_v< std::list< std::string > > );
			static_assert( not Alepha::Meta::is_deque_v< std::forward_list< std::string > > );
			static_assert( not Alepha::Meta::is_deque_v< std::string > );
			static_assert( not Alepha::Meta::is_deque_v< std::wstring > );

			static_assert( not Alepha::Meta::is_string_v< std::vector< int > > );
			static_assert( not Alepha::Meta::is_string_v< std::list< int > > );
			static_assert( not Alepha::Meta::is_string_v< std::forward_list< int > > );
			static_assert( not Alepha::Meta::is_string_v< std::deque< int > > );
			static_assert( not Alepha::Meta::is_string_v< std::vector< std::string > > );
			static_assert( not Alepha::Meta::is_string_v< std::list< std::string > > );
			static_assert( not Alepha::Meta::is_string_v< std::forward_list< std::string > > );
			static_assert( not Alepha::Meta::is_string_v< std::deque< std::string > > );
		};

		"meta.streams"_test <=[]
		{
			static_assert( Alepha::Meta::is_streamable_v< int > );
			static_assert( not Alepha::Meta::is_streamable_v< void > );
		};
	};

	namespace MetaContainer= Alepha::Meta::Container::exports;
	using Alepha::Meta::type_value;
	constexpr MetaContainer::vector< int, float, char > my_list;

	template< typename First, typename Last, typename Type >
	constexpr bool
	containsChar( First first, Last last, type_value< Type > value )
	{
		for( auto pos= first; pos != last; ++pos )
		{
			if( *pos == value ) return true;
		}
		return false;
	}

	static_assert( containsChar( begin( my_list ), end( my_list ), type_value< char >{} ) );
	static_assert( not containsChar( begin( my_list ), end( my_list ), type_value< long double >{} ) );
	static_assert( containsChar( begin( my_list ), end( my_list ), type_value< int >{} ) );
}


