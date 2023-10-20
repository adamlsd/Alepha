static_assert( __cplusplus > 2020'00 );

#include "../StackableStreambuf.h"

#include <fstream>

#include <Alepha/Testing/test.h>
#include <Alepha/Testing/TableTest.h>
#include <Alepha/Utility/evaluation_helpers.h>

#include "../word_wrap.h"

namespace
{
	using namespace Alepha::Testing::literals::test_literals;;
	using Alepha::Testing::TableTest;
}

static auto init= Alepha::Utility::enroll <=[]
{
	"Simple stacked wordwrap case."_test <=[]
	{
		std::ostringstream oss;

		oss << Alepha::StartWrap{ 20 };
		oss << "First wrapping\n";
		oss << Alepha::StartWrap{ 10 };
		oss << "Second wrapping\n";
		oss << Alepha::EndWrap;
		oss << "Third wrapping more than 20\n";
		oss << Alepha::EndWrap;

		std::ofstream log{ "log.txt" };
		log << oss.str();
		log.close();
		std::cout << oss.str() << std::flush;
		assert( oss.str() == "First wrapping\nSecond \nwrapping\nThird wrapping more \nthan 20\n" );
	};
};
