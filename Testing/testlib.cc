static_assert( __cplusplus > 2020'00 );

#include <Alepha/Testing/test.h>
#include <Alepha/ProgramOptions.h>

namespace
{
	namespace impl
	{
		int
		main( const int argcnt, const char *const *const argvec )
		{
			// TODO: Have test selection options here and more.
			const auto args= Alepha::handleOptions( argcnt, argvec );
			const auto result= Alepha::Testing::runAllTests( args );
			return result;
		}
	}
}

int
main( const int argcnt, const char *const *const argvec )
{
	return impl::main( argcnt, argvec );
}
