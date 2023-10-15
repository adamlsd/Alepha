static_assert( __cplusplus > 2020'00 );

#include "../string_algorithms.h"

#include <Alepha/Testing/test.h>
#include <Alepha/Testing/TableTest.h>

#include <Alepha/Utility/evaluation.h>

namespace
{
	using namespace Alepha::Testing::literals::test_literals;
	using namespace Alepha::Testing::exports;

	using Alepha::Utility::exports::enroll;
	using Alepha::Utility::exports::lambaste;
}

static auto init= enroll <=[]
{
	"Some simple substitution tests"_test <=TableTest< Alepha::expandVariables >::Cases
	{
		{
			"Hello World",
			{ "$H$ $W$", { { "H", lambaste<="Hello" }, { "W", lambaste<="World" } }, '$' },
			"Hello World"
		},

		{
			"Hello $$ World",
			{ "$H$ $$ $W$", { { "H", lambaste<="Hello" }, { "W", lambaste <="World" } }, '$' },
			"Hello $ World"
		},
	};

	"An exception should be thrown when there is a trailing unenclosed variable."_test <=[]
	{
		try
		{
			Alepha::expandVariables( "$H$ $W", { { "H", lambaste<="Hello" }, { "W", lambaste<="World" } }, '$' );
			abort();
		}
		catch( ... ) {}
	};
};
