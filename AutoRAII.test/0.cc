static_assert( __cplusplus > 201700, "C++17 Required" );

#include <Alepha/AutoRAII.h>

#include <Alepha/Testing/test.h>
#include <Alepha/Utility/evaluation.h>

int
main( const int argcnt, const char *const argvec[] )
{
	return Alepha::Testing::runAllTests( argcnt, argvec );
}

namespace
{
	using namespace Alepha::exports::auto_raii;

	using namespace Alepha::Utility::exports::evaluation;
	using namespace Alepha::Testing::exports::literals;
	using Alepha::Testing::exports::TestState;

	auto tests= enroll <=[]
	{
		"basic.syntax"_test <=[]( TestState test )
		{
			AutoRAII managed{ []{ return 42; }, []( auto ){} };
			test.expect( managed == 42 );
		};

		"basic.functionality"_test <=[]( TestState test )
		{
			bool initialized= false;
			bool destroyed= false;
			{
				AutoRAII managed{ [&]{ initialized= true; return 0; }, [&]( auto ) { initialized= false; destroyed= true; } };
				test.expect( initialized );
				test.expect( not destroyed );
			}
			test.expect( not initialized );
			test.expect( destroyed );
		};
	};
}
