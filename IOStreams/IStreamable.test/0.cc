static_assert( __cplusplus > 2020'00 );

#include "../IStreamable.h"

#include <Alepha/Testing/TableTest.h>
#include <Alepha/Testing/test.h>

#include <sstream>

#include <Alepha/auto_comparable.h>

#include <Alepha/IOStreams/OStreamable.h>

namespace
{
	template< typename= Alepha::Capabilities< Alepha::auto_comparable, Alepha::IOStreams::IStreamable, Alepha::IOStreams::OStreamable > >
	struct Agg_core
	{
		int x;
		int y;
		int z;
	};

	using Agg= Agg_core<>;
	static_assert( Alepha::Aggregate< Agg > );
	static_assert( Alepha::Capability< Agg, Alepha::IOStreams::IStreamable > );
	static_assert( Alepha::Capability< Agg, Alepha::IOStreams::OStreamable > );
}


auto
buildFromString( const std::string text )
{
	Agg rv;
	std::istringstream iss{ text };
	iss >> rv;
	return rv;
}

static auto init= Alepha::Utility::enroll <=[]
{
	using namespace Alepha::Testing::exports;
	using namespace Alepha::Testing::literals::test_literals;

	"Simple IStream"_test <=TableTest< buildFromString >
	::Cases
	{
		{ "smoke test", { "1\t2\t3" }, { 1, 2, 3 } },
	};
};
