static_assert( __cplusplus > 2020'00 );

#include "../string_algorithms.h"

#include <Alepha/Testing/test.h>
#include <Alepha/Testing/TableTest.h>

#include <Alepha/Utility/evaluation_helpers.h>

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
			"Hello World (with some spaces)",
			{ "$with space$ $can-expand$", { { "with space", lambaste<="Hello" }, { "can-expand", lambaste<="World" } }, '$' },
			"Hello World"
		},

		{
			"Hello $$ World",
			{ "$H$ $$ $W$", { { "H", lambaste<="Hello" }, { "W", lambaste <="World" } }, '$' },
			"Hello $ World"
		},
	};

	"An exception should be thrown when there is a trailing unenclosed variable."_test <=TableTest< Alepha::expandVariables >::ExceptionCases
	{
		{ "Complete var",
			{ "$H$ $W$", { { "H", lambaste<="Hello" }, { "W", lambaste<="World" } }, '$' },
			std::nothrow
		},
		{ "Complete var",
			{ "$H$ $W$", { { "H", lambaste<="Hello" }, { "W", lambaste<="World" } }, '$' },
			std::type_identity< void >{}
		},
		{ "Incomplete var",
			{ "$H$ $W", { { "H", lambaste<="Hello" }, { "W", lambaste<="World" } }, '$' },
			std::type_identity< std::exception >{} 
		},
	};

	"Does the `split` function handle simple cases correctly?"_test <=TableTest< Alepha::split >::Cases
	{
		{ "Empty string", { "", ':' }, { "" } },
		{ "Single token", { "item", ':' }, { "item" } },
		{ "Two tokens", { "first:second", ':' }, { "first", "second" } },
		{ "Empty string many tokens", { ":::", ':' }, { "", "", "", "" } },
	};
};
