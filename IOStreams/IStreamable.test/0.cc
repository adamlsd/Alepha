static_assert( __cplusplus > 2020'00 );

#include "../IStreamable.h"

#include <Alepha/Testing/TableTest.h>
#include <Alepha/Testing/test.h>

#include <sstream>

namespace
{
	template< typename= Alepha::Capabilities< Alepha::IOStreams::IStreamable > >
	struct Agg_core
	{
		int x;
		int y;
		int z;

		friend bool operator == ( const Agg_core &lhs, const Agg_core &rhs )= default;
		friend std::ostream &
		operator << ( std::ostream &os, const Agg_core &agg )
		{
			return os << "{ " << agg.x << ", " << agg.y << ", " << agg.z << " }";
		}
	};

	using Agg= Agg_core<>;
	static_assert( Alepha::Aggregate< Agg > );
	static_assert( Alepha::Capability< Agg, Alepha::IOStreams::IStreamable > );
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
