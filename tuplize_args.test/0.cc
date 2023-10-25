#include "../tuplize_args.h"

#include <Alepha/Testing/test.h>
#include <Alepha/Testing/TableTest.h>

#include <Alepha/Utility/evaluation_helpers.h>

static auto init= Alepha::Utility::enroll <=[]
{
	using namespace Alepha::Testing::exports;
	using namespace Alepha::Utility::exports::evaluation_helpers;

	"A basic tuplization example"_test <=TableTest
	<
		Alepha::tuplizeArgs< std::tuple< int, std::string, int, char > >
	>
	::Cases
	{
		{ "Smoke example", { { "1", "Hello", "42", "x" } }, { 1, "Hello", 42, 'x' } },
	};

	"Do trailing vectors permit variadics?"_test <=TableTest
	<
		Alepha::tuplizeArgs< std::tuple< int, std::string, std::vector< int > > >
	>
	::Cases
	{
		{ "One extra argument", { { "1", "Hello", "2" } }, { 1, "Hello", { 2 } } },
		{ "Two extra arguments", { { "1", "Hello", "2", "3" } }, { 1, "Hello", { 2, 3 } } },
		{ "Three extra arguments", { { "1", "Hello", "2", "3", "4" } }, { 1, "Hello", { 2, 3, 4 } } },
	};

	"Does trailing optional stacked permit variadics??"_test <=TableTest
	<
		Alepha::tuplizeArgs< std::tuple< std::optional< int >, std::optional< std::string >, std::optional< int > > >
	>
	::Cases
	{
		{ "No arguments", { {} }, { std::nullopt, std::nullopt, std::nullopt } },
		{ "One argument", { { "1" } }, { 1, std::nullopt, std::nullopt } },
		{ "Two arguments", { { "1", "Hello" } }, { 1, "Hello", std::nullopt } },
		{ "Three arguments", { { "1", "Hello", "2" } }, { 1, "Hello", 2 } },
	};
};
