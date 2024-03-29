static_assert( __cplusplus > 2020'00 );

#include "../OStreamable.h"

#include <Alepha/Testing/TableTest.h>
#include <Alepha/Testing/test.h>

#include <sstream>

#include <Alepha/auto_comparable.h>

#include <Alepha/IOStreams/delimiters.h>
#include <Alepha/IOStreams/String.h>

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
	stringify_specific( const Agg &agg, const std::string delim )
	{
		std::ostringstream oss;
		using Alepha::IOStreams::fieldDelimiter;
		Alepha::IOStreams::setGlobalDelimiter( fieldDelimiter, "YOU SHOULD NOT SEE THIS" );
		oss << Alepha::IOStreams::setDelimiter( fieldDelimiter, delim );
		oss << agg;
		return std::move( oss ).str();
	}

	auto
	stringify_global( const Agg &agg, const std::string delim )
	{
		std::ostringstream oss;
		using Alepha::IOStreams::fieldDelimiter;
		Alepha::IOStreams::setGlobalDelimiter( fieldDelimiter, delim );
		oss << agg;
		return std::move( oss ).str();
	}

	auto
	stringify_default( const Agg &agg )
	{
		std::ostringstream oss;
		oss << agg;
		return std::move( oss ).str();
	}
}

static auto init= Alepha::Utility::enroll <=[]
{
	using namespace Alepha::Testing::exports;
	using namespace Alepha::Testing::literals::test_literals;
	using namespace Alepha::IOStreams::exports::string;

	"Simple OStream (default delimiter)"_test <=TableTest< stringify_default >
	::Cases
	{
		{ "smoke test", { { 1, 2, 3 } }, { "1\t2\t3" } },
	};

	"Simple OStream (specific delimiter)"_test <=TableTest< stringify_specific >
	::Cases
	{
		{ "smoke test", { { 1, 2, 3 }, "\t" }, { "1\t2\t3" } },
		{ "smoke test", { { 1, 2, 3 }, "," }, { "1,2,3" } },
		{ "smoke test", { { 1, 2, 3 }, ";;" }, { "1;;2;;3" } },
		{ "smoke test", { { 1, 2, 3 }, ", " }, { "1, 2, 3" } },
	};

	"Simple OStream (stream builder)"_test <=TableTest
	<
		[]( const Agg agg, const std::string delim )
		{
			using Alepha::IOStreams::String;
			using Alepha::IOStreams::fieldDelimiter;
			return String{} << setDelimiter( fieldDelimiter, delim ) << agg << FinishString;
		}
	>
	::Cases
	{
		{ "smoke test", { { 1, 2, 3 }, "\t" }, { "1\t2\t3" } },
		{ "smoke test", { { 1, 2, 3 }, "," }, { "1,2,3" } },
		{ "smoke test", { { 1, 2, 3 }, ";;" }, { "1;;2;;3" } },
		{ "smoke test", { { 1, 2, 3 }, ", " }, { "1, 2, 3" } },
	};

	"Simple OStream (global delimiter)"_test <=TableTest< stringify_global >
	::Cases
	{
		{ "smoke test", { { 1, 2, 3 }, "\t" }, { "1\t2\t3" } },
		{ "smoke test", { { 1, 2, 3 }, "," }, { "1,2,3" } },
		{ "smoke test", { { 1, 2, 3 }, ";;" }, { "1;;2;;3" } },
		{ "smoke test", { { 1, 2, 3 }, ", " }, { "1, 2, 3" } },
	};

};
