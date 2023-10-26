static_assert( __cplusplus > 2020'00 );

#pragma once

#include <Alepha/Alepha.h>

#include <cassert>

#include <iostream>
#include <string>
#include <utility>
#include <vector>
#include <functional>
#include <memory>

#include <Alepha/Console.h>
#include <Alepha/types.h>

#include <Alepha/Utility/evaluation_helpers.h>
#include <Alepha/Utility/StaticValue.h>

#include "colors.h"

namespace Alepha::Hydrogen::Testing
{
	inline namespace exports { inline namespace testing {} }

	namespace detail::testing
	{
		inline namespace exports {}

		namespace C
		{
			const bool debug= false;
			const bool debugTestRegistration= false or C::debug;
			const bool debugTestRun= false or C::debug;

			using namespace testing_colors::C::Colors;
		}

		using namespace std::literals::string_literals;
		using namespace Utility::exports::evaluation_helpers;
		using namespace Utility::exports::static_value;

		struct TestName
		{
			std::string name;
			bool disabled= false;

			[[nodiscard]] TestName
			operator -() const
			{
				auto rv= *this;

				rv.disabled= true;

				return rv;
			}
		};

		namespace exports
		{
			struct TestFailureException;

			inline namespace literals
			{
				[[nodiscard]] inline auto
				operator""_test( const char name[], std::size_t amount )
				{
					return TestName{ std::string{ name, name + amount } };
				}
			}
		}

		StaticValue< std::vector< std::tuple< std::string, bool, std::function< void() > > > > registry;
		auto initRegistry= enroll <=registry;

		// It is okay to discard this, if making tests in an enroll block.
		inline auto
		operator <= ( TestName name, std::function< void () > test )
		{
			struct TestRegistration {} rv;
			if( C::debugTestRegistration ) std::cerr << "Attempting to register: " << name.name << std::endl;

			registry().emplace_back( name.name, name.disabled, test );
			assert( not registry().empty() );
			assert( std::get< 1 >( registry().back() ) == name.disabled );

			return rv;
		};

		struct exports::TestFailureException
		{
			int failureCount= -1;

			explicit TestFailureException( const int failureCount ) : failureCount( failureCount ) {}
		};

		template< Integral Integer >
		inline auto
		operator <= ( TestName name, std::function< Integer () > test )
		{
			if constexpr( std::is_same_v< Integer, bool > )
			{
				auto wrapper= [test]
				{
					if( not test() )
					{
						throw TestFailureException{ 1 };
					}
				};

				return name <= wrapper;
			}
			else
			{
				auto wrapper= [test]
				{
					const int failures= test();
					if( failures > 0 ) throw TestFailureException{ failures };
				};

				return name <= wrapper;
			}
		}

		namespace exports
		{
			struct TestStateCore
			{
				public:
					std::vector< std::string > failures;

				public:
					void
					expect( const bool state, const std::string test= "" )
					{
						if( not state ) failures.push_back( test );
					}

					void
					demand( const bool state, const std::string test= "" )
					{
						if( not state ) throw TestFailureException( failures.size() + 1 );
					}
			};

			using TestState= TestStateCore &;
		}

		inline auto
		operator <= ( TestName name, std::function< void ( TestState ) > test )
		{
			auto wrapper= [test]
			{
				TestStateCore state;
				test( state );
				return state.failures.size();
			};
			return name <= std::function{ wrapper };
		};

		template< typename TestFunc >
		inline auto
		operator <= ( TestName name, TestFunc test )
		{
			return name <= std::function{ test };
		}

		namespace exports
		{
			[[nodiscard]] inline int
			runAllTests( const std::vector< std::string > selections= {} )
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
							catch( const TestFailureException &fail ) { std::cout << " -- " <<  fail.failureCount << " failures."; }
							catch( ... ) { std::cout << " --  unknown failure count"; }
							std::cout << std::endl;
						}

						std::cout << C::testInfo << "FINISHED" << resetStyle << ": " << name << std::endl;
					}
				}
				
				return failed ? EXIT_FAILURE : EXIT_SUCCESS;
			}

			[[nodiscard]] inline int
			runAllTests( const argcnt_t argcnt, const argvec_t argvec )
			{
				return runAllTests( { argvec + 1, argvec + argcnt } );
			}
		}
	}

	namespace exports::testing
	{
		using namespace detail::testing::exports;
	}

	namespace exports::inline literals::inline test_literals
	{
		using namespace detail::testing::exports::literals;
	}
}
