static_assert( __cplusplus > 2020'00 );

#include <cstdint>

#include <Alepha/Testing/TableTest.h>
#include <Alepha/Testing/test.h>
#include <Alepha/Utility/evaluation.h>


int
main( const int argcnt, const char *const *const argvec )
{
	return Alepha::Testing::runAllTests( argcnt, argvec );
}


namespace
{
	using namespace Alepha::Testing::exports;
	using namespace Alepha::Utility::exports::evaluation;

	int
	add( int a, int b )
	{
		return a + b;
	}

	auto basic_test= "basic_test"_test <=[]
	{
		return 0;
	};

	auto test= "addition.two.test"_test <=TableTest< add >::Cases
	{
		{ "Basic Smoke Test", { 2, 2 }, 4 },
		{ "Lefthand identity", { 0, 25 }, 25 },
		{ "Righthand identity", { 25, 0 }, 25 },
	};

	auto alltests= enroll <=[]
	{
		"addition.two.local"_test <=TableTest< add >::Cases
		{
			{ "Negative case", { -10, -20 }, -30 },
		};
	};
}
