static_assert( __cplusplus > 2020'00 );

#include <Alepha/Testing/test.h>
#include <Alepha/Testing/TableTest.h>
#include <Alepha/Utility/evaluation_helpers.h>

namespace
{
	namespace UnitTest= Alepha::Testing::exports;

	using namespace Alepha::Utility::exports::evaluation_helpers;
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
