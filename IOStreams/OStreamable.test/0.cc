static_assert( __cplusplus > 2020'00 );

#include "../OStreamable.h"

#include <Alepha/Testing/TableTest.h>
#include <Alepha/Testing/test.h>

#include <sstream>

#include <Alepha/auto_comparable.h>

#include <Alepha/IOStreams/delimiters.h>

namespace
{
	template< typename= Alepha::Capabilities< Alepha::auto_comparable, Alepha::IOStreams::OStreamable > >
	struct Agg_core
	{
		int x;
		int y;
		int z;
	};

	using Agg= Agg_core<>;
	static_assert( Alepha::Aggregate< Agg > );
	static_assert( Alepha::Capability< Agg, Alepha::IOStreams::OStreamable > );


	auto
	stringify( const Agg &agg, const std::string delim )
	{
		std::ostringstream oss;
		oss << Alepha::IOStreams::setFieldDelimiter( delim );
		oss << agg;
		return std::move( oss ).str();
	}
}

static auto init= Alepha::Utility::enroll <=[]
{
	using namespace Alepha::Testing::exports;
	using namespace Alepha::Testing::literals::test_literals;

	"Simple OStream"_test <=TableTest< stringify >
	::Cases
	{
		{ "smoke test", { { 1, 2, 3 }, "\t" }, { "1\t2\t3" } },
		{ "smoke test", { { 1, 2, 3 }, "," }, { "1,2,3" } },
		{ "smoke test", { { 1, 2, 3 }, ";;" }, { "1;;2;;3" } },
		{ "smoke test", { { 1, 2, 3 }, ", " }, { "1, 2, 3" } },
	};
};
