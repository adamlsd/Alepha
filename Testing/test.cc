#include <Alepha/Testing/test.h>
#include <Alepha/Utility/evaluation.h>

namespace
{
	namespace UnitTest= Alepha::Testing::exports::testing;
}

int
main( const int argcnt, const char *const *const argvec )
{
	return UnitTest::runAllTests( argcnt, argvec );
}

namespace
{
	using namespace UnitTest::literals;
	using namespace Alepha::Utility::exports::evaluation;

	auto registration= enroll <=[]
	{
		 "enroll.basic.success"_test <=[]{};
		-"enroll.basic.failure"_test <=[]{ throw 0; };
	};

	auto named1= "named.basic.success"_test <= []{};
	auto named2= -"named.basic.failure"_test <=[]{ return 1; };
}
