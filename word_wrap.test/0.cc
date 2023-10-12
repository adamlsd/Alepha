static_assert( __cplusplus > 2020'00 );

#include "../word_wrap.h"

#include <Alepha/Testing/test.h>
#include <Alepha/Testing/TableTest.h>
#include <Alepha/Utility/evaluation.h>

namespace
{
	using namespace Alepha::Testing::literals::program_option_literals;
	using Alepha::Testing::TableTest;
}

static auto init= Alepha::Utility::enroll <=[]
{
	"Does word wrap with no-indent do sensible things?"_test <=TableTest< Alepha::wordWrap >::Cases
	{
		{ "Three word with break after first?", { "Goodbye cruel world!", 12, 0 }, "Goodbye \ncruel world!" },
	};
};
