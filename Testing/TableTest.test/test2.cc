static_assert( __cplusplus > 2020'00 );

#include <Alepha/Testing/TableTest.h>

#include <Alepha/Testing/test.h>
#include <Alepha/Utility/evaluation_helpers.h>

#include <Alepha/IOStreams/delimiters.h>
#include <Alepha/IOStreams/OStreamable.h>

#include <Alepha/comparisons.h>


namespace
{
	using namespace Alepha::Testing::exports;
	using namespace Alepha::Utility::exports::evaluation_helpers;

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

	template< typename= Alepha::Capabilities< Alepha::IOStreams::OStreamable, Alepha::comparable > >
	struct Aggregate_core
	{
		int x, y, z;

		friend bool operator == ( Aggregate_core, Aggregate_core ) noexcept= default;
	};
	using Aggregate= Aggregate_core<>;

	auto alltests= enroll <=[]
	{
		"addition.two.local"_test <=TableTest< add >::Cases
		{
			{ "Negative case", { -10, -20 }, -30 },
		};

		"Can we use Aggregates with universal cases, correctly?"_test <=
		TableTest
		<
			[]( const int x )
			{
				if( x < 0 ) throw std::runtime_error{ "Cannot be negative." };
				return Aggregate{ x, x, x };
			}
		>
		::UniversalCases
		{
			{ "Basic value case", { 42 }, { 42, 42, 42 } },
			{ "Ignore exceptions case (`std::nothrow`)", { 42 }, std::nothrow },
			{ "Ignore exceptions case (`std::type_identity< void >`)", { 42 }, std::type_identity< void >{} },
			{ "Expect exception type runtime_error", { -42 }, std::type_identity< std::runtime_error >{} },
			{ "Expect exception type exception", { -42 }, std::type_identity< std::exception >{} },
			{ "Expect exception value specific", { -42 }, std::runtime_error{ "Cannot be negative." } },

		/* These cases should fail, but we don't want to fail them in normal builds. */
		#if 0
			{ "Failing: Basic value case", { -42 }, { 42, 42, 42 } },
			{ "Failing: Ignore exceptions case (`std::nothrow`)", { -42 }, std::nothrow },
			{ "Failing: Ignore exceptions case (`std::type_identity< void >`)", { -42 }, std::type_identity< void >{} },
			{ "Failing: Expect exception type runtime_error", { 42 }, std::type_identity< std::runtime_error >{} },
			{ "Failing: Expect exception type exception", { 42 }, std::type_identity< std::exception >{} },
			{ "Failing: Expect exception value specific", { 42 }, std::runtime_error{ "Cannot be negative." } },
		#endif
		};
	};
}
