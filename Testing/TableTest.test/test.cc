static_assert( __cplusplus > 2020'00 );

#include <cstdint>

#include <Alepha/Testing/test.h>
#include <Alepha/Testing/TableTest.h>
#include <Alepha/Utility/evaluation.h>

namespace
{
	namespace UnitTest= Alepha::Testing::exports;
}

int
main( const int argcnt, const char *const *const argvec )
{
	return UnitTest::runAllTests( argcnt, argvec );
}

namespace
{
	using namespace Alepha::Utility::exports::evaluation;
	using namespace UnitTest::literals;
	using UnitTest::TableTest;

	auto registration= enroll <=[]
	{
		 "enroll.basic.success"_test <=[]{};
		-"enroll.basic.failure"_test <=[]{ throw 0; };
	};

	auto named1= "named.basic.success"_test <= []{};
	auto named2= -"named.basic.failure"_test <=[]{ return 1; };

	int identity( int a ){ return a; }
	auto namedTable1= "named.table.success"_test <=TableTest< identity >::Cases
	{
		{ "smoke", { 1 }, 1 },
		//{ "fail", { 2 }, 1 },
	};
}
