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
			struct TestFailure;

			inline namespace literals
			{
				[[nodiscard]] inline auto
				operator""_test( const char name[], std::size_t amount )
				{
					return TestName{ std::string{ name, name + amount } };
				}
			}
		}

		// It is okay to discard this, if making tests in an enroll block.
		inline namespace impl
		{
			struct TestRegistration {};
			TestRegistration operator <= ( TestName name, std::function< void () > test );
		}

		struct exports::TestFailure
		{
			int failureCount= -1;
			std::string message_;

			explicit TestFailure( const int failureCount )
				: failureCount( failureCount ) {}
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
						throw TestFailure{ 1 };
					}
				};

				return name <= wrapper;
			}
			else
			{
				auto wrapper= [test]
				{
					const int failures= test();
					if( failures > 0 ) throw TestFailure{ failures };
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
						if( not state ) throw TestFailure( failures.size() + 1 );
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
			[[nodiscard]] int runAllTests( const std::vector< std::string > selections= {} );

			[[nodiscard]] int runAllTests( const argcnt_t argcnt, const argvec_t argvec );
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
