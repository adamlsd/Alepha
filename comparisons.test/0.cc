static_assert( __cplusplus > 201700, "C++17 Required" );

#include <Alepha/comparisons.h>

#include <Alepha/Testing/test.h>
#include <Alepha/Testing/TableTest.h>
#include <Alepha/Utility/evaluation.h>

namespace
{
	using Alepha::exports::types::argcnt_t;
	using Alepha::exports::types::argvec_t;
}

int
main( const argcnt_t argcnt, const argvec_t argvec )
{
	return Alepha::Testing::runAllTests( argcnt, argvec );
}

namespace
{
	using namespace Alepha::Testing::exports;
	using namespace Alepha::exports::comparisons;
	using namespace Alepha::exports::capabilities;

	template
	<
		typename= int,
		typename= Capabilities< comparable >,
		typename= float,
		typename= Capabilities< short >
	>
	struct Date_core
	{
		int y;
		int m;
		int d;

		auto value_lens() const { return std::tie( y, m, d ); }
	};

	using Date= Date_core<>;
	namespace detail= Alepha::detail::capabilities;

	namespace Meta= Alepha::Meta;

	constexpr Meta::Container::vector< short, long, int, int, std::string, std::vector< int >, long, void, void > vec;
	constexpr Meta::type_value< int > val;
	using std::begin, std::end;

	static_assert( Meta::find_if( begin( vec ), end( vec ), Meta::bind1st( Meta::is_same, Meta::type_value< int >{} ) ) );
	static_assert( not Meta::find_if( begin( vec ), end( vec ), Meta::bind1st( Meta::is_same, Meta::type_value< double >{} ) ) );

	static_assert( detail::is_capability_list_v< Capabilities< comparable > > );
	static_assert( Alepha::has_capability( Meta::type_value< Date >{}, comparable_capability ) );

	template< template< typename > class op, typename T >
	constexpr bool
	comp( const T &lhs, const T &rhs )
	{
		return op< T >{}( lhs, rhs );
	}

	auto tests= Alepha::Utility::enroll <=[]
	{
		"smoke.lt"_test <=TableTest< comp< std::less, Date > >::Cases
		{
			{ "smoke1", { { 1982, 12, 21 }, { 2020, 12, 15 } }, true },
			{ "smoke1", { { 2020, 12, 15 }, { 1982, 12, 21 } }, false },
		};

		"smoke.gt"_test <=TableTest< comp< std::greater, Date > >::Cases
		{
			{ "smoke1", { { 1982, 12, 21 }, { 2020, 12, 15 } }, false },
			{ "smoke1", { { 2020, 12, 15 }, { 1982, 12, 21 } }, true },
		};

		"smoke.le"_test <=TableTest< comp< std::less_equal, Date > >::Cases
		{
			{ "smoke1", { { 1982, 12, 21 }, { 2020, 12, 15 } }, true },
			{ "smoke1", { { 2020, 12, 15 }, { 1982, 12, 21 } }, false },
		};

		"smoke.ge"_test <=TableTest< comp< std::greater_equal, Date > >::Cases
		{
			{ "smoke1", { { 1982, 12, 21 }, { 2020, 12, 15 } }, false },
			{ "smoke1", { { 2020, 12, 15 }, { 1982, 12, 21 } }, true },
		};

		"smoke.eq"_test <=TableTest< comp< std::equal_to, Date > >::Cases
		{
			{ "smoke1", { { 1982, 12, 21 }, { 2020, 12, 15 } }, false },
			{ "smoke1", { { 2020, 12, 15 }, { 1982, 12, 21 } }, false },
		};

		"smoke.ne"_test <=TableTest< comp< std::not_equal_to, Date > >::Cases
		{
			{ "smoke1", { { 1982, 12, 21 }, { 2020, 12, 15 } }, true },
			{ "smoke1", { { 2020, 12, 15 }, { 1982, 12, 21 } }, true },
		};
	};
}
