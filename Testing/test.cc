static_assert( __cplusplus > 2020'00 );

#include "test.h"

namespace Alepha::Hydrogen::Testing::detail::testing
{
	StaticValue< std::vector< std::tuple< std::string, bool, std::function< void() > > > > registry;

	TestRegistration
	impl::operator <= ( TestName name, std::function< void () > test )
	{
		if( C::debugTestRegistration ) std::cerr << "Attempting to register: " << name.name << std::endl;

		registry().emplace_back( name.name, name.disabled, test );
		assert( not registry().empty() );
		assert( std::get< 1 >( registry().back() ) == name.disabled );

		return {};
	};

	[[nodiscard]] int
	exports::runAllTests( const std::vector< std::string > selections )
	{
		if( C::debugTestRun )
		{
			std::cerr << "Going to run all tests.  (I see " << registry().size() << " tests.)" << std::endl;
		}
		bool failed= false;
		const auto selected= [ selections ]( const std::string test )
		{
			for( const auto &selection: selections )
			{
				if( test.find( selection ) != std::string::npos ) return true;
			}
			return empty( selections );
		};

		const auto explicitlyNamed= [ selections ]( const std::string s )
		{
			return std::find( begin( selections ), end( selections ), s ) != end( selections );
		};

		for( const auto &[ name, disabled, test ]: registry() )
		{
			if( C::debugTestRun ) std::cerr << "Trying test " << name << std::endl;

			if( explicitlyNamed( name ) or not disabled and selected( name ) )
			{
				std::cout << C::testInfo << "BEGIN" << resetStyle << "   : " << name << std::endl;
				try
				{
					test();
					std::cout << "  " << C::testPass << "SUCCESS" << resetStyle << ": " << name << std::endl;
				}
				catch( ... )
				{
					try
					{
						failed= true;
						std::cout << "  " << C::testFail << "FAILURE" << resetStyle << ": " << name;
						throw;
					}
					catch( const TestFailure &fail ) { std::cout << " -- " <<  fail.failureCount << " failures."; }
					catch( const std::exception &ex ) { std::cout << " --  unknown failure count (mesg: " << ex.what() << ")"; }
					catch( ... ) { std::cout << " --  unknown failure count"; }
					std::cout << std::endl;
				}

				std::cout << C::testInfo << "FINISHED" << resetStyle << ": " << name << std::endl;
			}
		}
		
		return failed ? EXIT_FAILURE : EXIT_SUCCESS;
	}

	[[nodiscard]] int
	exports::runAllTests( const argcnt_t argcnt, const argvec_t argvec )
	{
		return runAllTests( { argvec + 1, argvec + argcnt } );
	}
}
