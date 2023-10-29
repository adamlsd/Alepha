static_assert( __cplusplus > 2020'00 );

#include "../streamable.h"

#include <Alepha/Testing/TableTest.h>
#include <Alepha/Testing/test.h>

#include <sstream>

#include <Alepha/auto_comparable.h>

#include <Alepha/IOStreams/OStreamable.h>

namespace
{
	template< typename= Alepha::Capabilities< Alepha::auto_comparable, Alepha::IOStreams::streamable > >
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


std::string
roundTripString( const std::string text, const std::string delim )
{
	using namespace Alepha::IOStreams::exports::delimiters;
	std::istringstream iss{ text };
	
	Agg agg;
	iss >> setFieldDelimiter( delim ) >> agg;

	std::ostringstream oss;
	oss << setFieldDelimiter( delim ) << agg;

	return oss.str();
}

static auto init= Alepha::Utility::enroll <=[]
{
	using namespace Alepha::Testing::exports;
	using namespace Alepha::Testing::literals::test_literals;

	"Simple IStream"_test <=TableTest< roundTripString >
	::Cases
	{
		{ "smoke test", { "1\t2\t3", "\t" }, { "1\t2\t3" } },
		{ "smoke test", { "1,2,3", "," }, { "1,2,3" } },
	};
};
