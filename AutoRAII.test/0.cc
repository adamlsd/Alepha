static_assert( __cplusplus > 2020'00 );

#include <Alepha/AutoRAII.h>

#include <Alepha/Testing/test.h>
#include <Alepha/Utility/evaluation_helpers.h>

int
main( const int argcnt, const char *const argvec[] )
{
	return Alepha::Testing::runAllTests( argcnt, argvec );
}

static auto tests= Alepha::Utility::enroll <=[]
{
	using namespace Alepha::exports::auto_raii;
	using namespace Alepha::Testing::exports::literals;

	using Alepha::Testing::exports::TestState;

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
