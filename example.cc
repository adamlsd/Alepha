static_assert( __cplusplus > 2020'00 );

#include "ProgramOptions.h"

namespace
{
	using namespace Alepha::literals::option_literals;
	using namespace std::literals::string_literals;

	int optionA= 42;
	std::optional< std::string > optionB;

	auto init= Alepha::enroll <=[]
	{
		--"set-a"_option << optionA << "The option is an integer.  !default!";
		--"set-b"_option << optionB << "The option is a string, no defaults.";
	};
}

int
main( const int argcnt, const char *const *const argvec )
try
{
	const auto args= Alepha::handleOptions( argcnt, argvec );

	std::cout << "A is set to: " << optionA << std::endl;
	std::cout << "B is set to: " << ( optionB.has_value() ? optionB.value() : "nullopt"s ) << std::endl;

	return EXIT_SUCCESS;
}
catch( const std::exception &ex )
{
	std::cerr << "Error: " << ex.what() << std::endl;
	return EXIT_FAILURE;
}
