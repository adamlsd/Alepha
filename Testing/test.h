static_assert( __cplusplus > 201700, "C++17 Required" );

#pragma once

#include <Alepha/Alepha.h>

#include <cassert>

#include <iostream>
#include <string>
#include <utility>
#include <vector>
#include <functional>
#include <memory>

#include <Alepha/console.h>

namespace Alepha::Hydrogen::Testing
{
	inline namespace exports { inline namespace testing {} }

	namespace detail::testing
	{
		inline namespace exports {}

		using namespace std::literals::string_literals;

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

		inline auto &
		registry()
		{
			static std::vector< std::tuple< std::string, bool, std::function< void() > > > registry;
			return registry;
		}

		// It is okay to discard this, if making tests in an enroll block.
		inline auto
		operator <= ( TestName name, std::function< void () > test )
		{
			struct TestRegistration {} rv;

			registry().emplace_back( name.name, name.disabled, test );
			assert( not registry().empty() );
			assert( std::get< 1 >( registry().back() ) == name.disabled );

			return rv;
		};

		struct exports::TestFailureException
		{
			int failureCount= -1;
		};

		inline auto
		operator <= ( TestName name, std::function< int () > test )
		{
			auto wrapper= [test]
			{
				const int failures= test();
				if( failures > 0 ) throw TestFailureException{ failures };
			};

			return name <= wrapper;
		}

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
				try
				{
					if( explicitlyNamed( name ) or not disabled and selected( name ) )
					{
						test();
						std::cout << C::green << "SUCCESS" << C::normal << ": " << name << std::endl;
					}
				}
				catch( ... )
				{
					try
					{
						failed= true;
						std::cout << C::red << "FAILURE" << C::normal << ": " << name;
						throw;
					}
					catch( const TestFailureException &fail ) { std::cout << " -- " <<  fail.failureCount << " failures."; }
					catch( ... ) { std::cout << " --  unknown failure count"; }
					std::cout << std::endl;
				}
				
				return failed ? EXIT_FAILURE : EXIT_SUCCESS;
			}

			[[nodiscard]] inline int
			runAllTests( const unsigned argcnt, const char *const *const argvec )
			{
				return runAllTests( { argvec + 1, argvec + argcnt } );
			}
		}
	}

	namespace exports::testing
	{
		using namespace detail::testing::exports;
	}
}
