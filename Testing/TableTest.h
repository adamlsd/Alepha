static_assert( __cplusplus > 2020'00 );

#pragma once

#include <Alepha/Alepha.h>

#include <cstdint>

#include <tuple>
#include <string>
#include <iostream>
#include <algorithm>
#include <typeinfo>
#include <numeric>
#include <iomanip>

#include <boost/core/demangle.hpp>
#include <boost/lexical_cast.hpp>

#include <Alepha/function_traits.h>
#include <Alepha/template_for_each.h>

#include <Alepha/IOStreams/String.h>

#include <Alepha/Utility/evaluation_helpers.h>
#include <Alepha/Utility/TupleAdapter.h>

#include <Alepha/Reflection/tuplizeAggregate.h>

#include <Alepha/Console.h>

#include "colors.h"
#include "printDebugging.h"

namespace Alepha::Hydrogen::Testing  ::detail::  table_test
{
	inline namespace exports {}

	inline void breakpoint() {}

	namespace C:: inline Colors
	{
		using namespace testing_colors::C::Colors;
	}

	enum class TestResult { Passed, Failed };

	struct BlankBase {};

	template< typename T >
	static consteval auto
	compute_base_f() noexcept
	{
		if constexpr ( Aggregate< T > ) return std::type_identity< Utility::TupleAdapter< T > >{};
		else if constexpr( std::is_class_v< T > ) return std::type_identity< T >{};
		else return std::type_identity< BlankBase >{};
	}

	template< typename T >
	using compute_base_t= typename decltype( compute_base_f< std::decay_t< T > >() )::type;

	template< typename return_type, OutputMode outputMode >
	struct BasicUniversalHandler;

	template< Primitive return_type, OutputMode outputMode >
	struct BasicUniversalHandler< return_type, outputMode >
	{
		using Invoker= std::function< return_type () >;
		std::function< TestResult ( Invoker, const std::string & ) > impl;

		TestResult
		operator() ( Invoker invoker, const std::string &comment ) const
		{
			return impl( invoker, comment );
			//if constexpr( std::is_base_of_v< std::decay_t< return_type >, ComputedBase > )
		}

#if 1
		BasicUniversalHandler( const return_type expected )
		: impl
		{
			[expected]( Invoker invoker, const std::string &comment )
			{
				const auto witness= Utility::evaluate <=[&]() -> std::optional< return_type >
				{
					try
					{
						return invoker();
					}
					catch( ... )
					{
						return std::nullopt;
					}
				};
				const auto result= witness == expected ? TestResult::Passed : TestResult::Failed;
				
				if( result == TestResult::Failed )
				{
					if( witness.has_value() )
					{
						std::cout << "    " << C::testFail << "FAILED CASE" << resetStyle << ": " << comment << std::endl;
						printDebugging< outputMode >( witness.value(), expected );
					}
					else std::cout << "    " << C::testFail << "FAILED CASE" << resetStyle << ": Unexpected exception in \"" << comment << '"' << std::endl;
				}
				else std::cout << "    " << C::testPass << "PASSED CASE" << resetStyle << ": " << comment << std::endl;
				return result;
			}
		}
		{}
#endif

#if 1
		template< typename T >
		requires( not SameAs< T, void > )
		BasicUniversalHandler( std::type_identity< T > ) : impl
		{
			[]( Invoker invoker, const std::string &comment )
			{
				try
				{
					std::ignore= invoker();
					std::cout << "    " << C::testFail << "FAILED CASE" << resetStyle << ": " << comment << std::endl;
					return TestResult::Failed;
				}
				catch( const T & )
				{
					std::cout << "    " << C::testPass << "PASSED CASE" << resetStyle << ": " << comment << std::endl;
					return TestResult::Passed;
				}
			}
		}
		{}

		template< typename T >
		requires( SameAs< T, std::type_identity< void > > or SameAs< T, std::nothrow_t > )
		BasicUniversalHandler( T ) : impl
		{
			[]( Invoker invoker, const std::string &comment )
			{
				try
				{
					std::ignore= invoker();
					std::cout << "    " << C::testPass << "PASSED CASE" << resetStyle << ": " << comment << std::endl;
					return TestResult::Passed;
				}
				catch( ... )
				{
					std::cout << "    " << C::testFail << "FAILED CASE" << resetStyle << ": " << comment << std::endl;
					return TestResult::Failed;
				}
			}
		}
		{}

		template< DerivedFrom< std::exception > T >
		BasicUniversalHandler( const T exemplar ) : impl
		{
			[expected= std::string{ exemplar.what() }]( Invoker invoker, const std::string &comment )
			{
				try
				{
					invoker();
					std::cout << "    " << C::testFail << "FAILED CASE" << resetStyle << ": " << comment << std::endl;
					std::cout << "  " << C::testInfo << "NOTE" << resetStyle << ": expected exception `"
							<< typeid( T ).name()
							<< "` wasn't thrown." << std::endl;
					return TestResult::Failed;
				}
				catch( const T &ex )
				{
					const std::string witness= ex.what();
					const TestResult rv= witness == expected ? TestResult::Passed : TestResult::Failed;
					if( rv == TestResult::Failed )
					{
						std::cout << "    " << C::testFail << "FAILED CASE" << resetStyle << ": " << comment << std::endl;
						std::cout << "  " << C::testInfo << "NOTE" << resetStyle << ": expected message did not match." << std::endl;
						printDebugging< outputMode >( witness, expected );
					}
					return rv;
				}
			}
		}
		{}
#endif
	};

	template< Aggregate return_type, OutputMode outputMode >
	struct BasicUniversalHandler< return_type, outputMode >
		: compute_base_t< return_type >
	{
		using ComputedBase= compute_base_t< return_type >;
		using ComputedBase::ComputedBase;

		using Invoker= std::function< return_type () >;

		std::function< TestResult ( Invoker, const std::string & ) > impl;

		TestResult
		operator() ( Invoker invoker, const std::string &comment ) const
		{
			if( impl != nullptr ) return impl( invoker, comment );
			//if constexpr( std::is_base_of_v< std::decay_t< return_type >, ComputedBase > )
			if constexpr( true )
			{
				const return_type *const expected_p= this;
				const auto expected= *expected_p;
				breakpoint();
				const auto witness= Utility::evaluate <=[&]() -> std::optional< return_type >
				{
					try
					{
						return invoker();
					}
					catch( ... )
					{
						return std::nullopt;
					}
				};
				const auto result= witness == expected ? TestResult::Passed : TestResult::Failed;
				
				if( result == TestResult::Failed )
				{
					if( witness.has_value() )
					{
						std::cout << "    " << C::testFail << "FAILED CASE" << resetStyle << ": " << comment << std::endl;
						printDebugging< outputMode >( witness.value(), expected );
					}
					else std::cout << "    " << C::testFail << "FAILED CASE" << resetStyle << ": Unexpected exception in \"" << comment << '"' << std::endl;
				}
				else std::cout << "    " << C::testPass << "PASSED CASE" << resetStyle << ": " << comment << std::endl;
				return result;
			}
			else throw std::logic_error( "Somehow we didn't setup impl, and it's not an adapted case!" );
		}

#if 1
		template< typename T >
		requires( not SameAs< T, void > )
		BasicUniversalHandler( std::type_identity< T > ) : impl
		{
			[]( Invoker invoker, const std::string &comment )
			{
				try
				{
					std::ignore= invoker();
					breakpoint();
					std::cout << "    " << C::testFail << "FAILED CASE" << resetStyle << ": " << comment << std::endl;
					return TestResult::Failed;
				}
				catch( const T & )
				{
					std::cout << "    " << C::testPass << "PASSED CASE" << resetStyle << ": " << comment << std::endl;
					return TestResult::Passed;
				}
			}
		}
		{}

		template< typename T >
		requires( SameAs< T, std::type_identity< void > > or SameAs< T, std::nothrow_t > )
		BasicUniversalHandler( T ) : impl
		{
			[]( Invoker invoker, const std::string &comment )
			{
				try
				{
					std::ignore= invoker();
					std::cout << "    " << C::testPass << "PASSED CASE" << resetStyle << ": " << comment << std::endl;
					return TestResult::Passed;
				}
				catch( ... )
				{
					std::cout << "    " << C::testFail << "FAILED CASE" << resetStyle << ": " << comment << std::endl;
					return TestResult::Failed;
				}
			}
		}
		{}

		template< DerivedFrom< std::exception > T >
		BasicUniversalHandler( const T exemplar ) : impl
		{
			[expected= std::string{ exemplar.what() }]( Invoker invoker, const std::string &comment )
			{
				try
				{
					invoker();
					std::cout << "    " << C::testFail << "FAILED CASE" << resetStyle << ": " << comment << std::endl;
					std::cout << "  " << C::testInfo << "NOTE" << resetStyle << ": expected exception `"
							<< typeid( T ).name()
							<< "` wasn't thrown." << std::endl;
					return TestResult::Failed;
				}
				catch( const T &ex )
				{
					const std::string witness= ex.what();
					const TestResult rv= witness == expected ? TestResult::Passed : TestResult::Failed;
					if( rv == TestResult::Failed )
					{
						std::cout << "    " << C::testFail << "FAILED CASE" << resetStyle << ": " << comment << std::endl;
						std::cout << "  " << C::testInfo << "NOTE" << resetStyle << ": expected message did not match." << std::endl;
						printDebugging< outputMode >( witness, expected );
					}
					return rv;
				}
			}
		}
		{}
#endif
	};

	template< typename return_type, OutputMode outputMode >
	struct BasicUniversalHandler
		: return_type
	{
		using return_type::return_type;

		BasicUniversalHandler( return_type rt ) : return_type( rt ) {}

		using Invoker= std::function< return_type () >;

		std::function< TestResult ( Invoker, const std::string & ) > impl;

		TestResult
		operator() ( Invoker invoker, const std::string &comment ) const
		{
			if( impl != nullptr ) return impl( invoker, comment );
			//if constexpr( std::is_base_of_v< std::decay_t< return_type >, ComputedBase > )
			if constexpr( true )
			{
				const return_type *const expected_p= this;
				const auto expected= *expected_p;
				const auto witness= Utility::evaluate <=[&]() -> std::optional< return_type >
				{
					try
					{
						return invoker();
					}
					catch( ... )
					{
						return std::nullopt;
					}
				};
				const auto result= witness == expected ? TestResult::Passed : TestResult::Failed;
				
				if( result == TestResult::Failed )
				{
					if( witness.has_value() )
					{
						std::cout << "    " << C::testFail << "FAILED CASE" << resetStyle << ": " << comment << std::endl;
						printDebugging< outputMode >( witness.value(), expected );
					}
					else std::cout << "    " << C::testFail << "FAILED CASE" << resetStyle << ": Unexpected exception in \"" << comment << '"' << std::endl;
				}
				else std::cout << "    " << C::testPass << "PASSED CASE" << resetStyle << ": " << comment << std::endl;
				return result;
			}
			else throw std::logic_error( "Somehow we didn't setup impl, and it's not an adapted case!" );
		}

#if 1
		template< typename T >
		requires( not SameAs< T, void > )
		BasicUniversalHandler( std::type_identity< T > ) : impl
		{
			[]( Invoker invoker, const std::string &comment )
			{
				try
				{
					std::ignore= invoker();
					std::cout << "    " << C::testFail << "FAILED CASE" << resetStyle << ": " << comment << std::endl;
					return TestResult::Failed;
				}
				catch( const T & )
				{
					std::cout << "    " << C::testPass << "PASSED CASE" << resetStyle << ": " << comment << std::endl;
					return TestResult::Passed;
				}
			}
		}
		{}

		template< typename T >
		requires( SameAs< T, std::type_identity< void > > or SameAs< T, std::nothrow_t > )
		BasicUniversalHandler( T ) : impl
		{
			[]( Invoker invoker, const std::string &comment )
			{
				try
				{
					std::ignore= invoker();
					std::cout << "    " << C::testPass << "PASSED CASE" << resetStyle << ": " << comment << std::endl;
					return TestResult::Passed;
				}
				catch( ... )
				{
					std::cout << "    " << C::testFail << "FAILED CASE" << resetStyle << ": " << comment << std::endl;
					return TestResult::Failed;
				}
			}
		}
		{}

		template< DerivedFrom< std::exception > T >
		BasicUniversalHandler( const T exemplar ) : impl
		{
			[expected= std::string{ exemplar.what() }]( Invoker invoker, const std::string &comment )
			{
				try
				{
					invoker();
					std::cout << "    " << C::testFail << "FAILED CASE" << resetStyle << ": " << comment << std::endl;
					std::cout << "  " << C::testInfo << "NOTE" << resetStyle << ": expected exception `"
							<< typeid( T ).name()
							<< "` wasn't thrown." << std::endl;
					return TestResult::Failed;
				}
				catch( const T &ex )
				{
					const std::string witness= ex.what();
					const TestResult rv= witness == expected ? TestResult::Passed : TestResult::Failed;
					if( rv == TestResult::Failed )
					{
						std::cout << "    " << C::testFail << "FAILED CASE" << resetStyle << ": " << comment << std::endl;
						std::cout << "  " << C::testInfo << "NOTE" << resetStyle << ": expected message did not match." << std::endl;
						printDebugging< outputMode >( witness, expected );
					}
					return rv;
				}
			}
		}
		{}
#endif
	};



	template< typename F >
	concept FunctionVariable=
	requires( const F &f )
	{
		{ std::function{ f } };
	};


	namespace exports
	{
		template< FunctionVariable auto, OutputMode outputMode= OutputMode::All > struct TableTest;
	}

	namespace C
	{
		const bool debug= false;
		const bool debugCaseTypes= false or C::debug;
	}

	using std::begin, std::end;
	using namespace Utility::exports::evaluation_helpers;

	template< template< typename, typename... > class Sequence, typename ... TupleArgs >
	auto
	withIndex( const Sequence< std::tuple< TupleArgs... > > &original )
	{
		auto indices= evaluate <=[&]
		{
			std::vector< int > v{ std::distance( begin( original ), end( original ) ) };
			std::iota( begin( v ), end( v ), 0 );
			return v;
		};

		auto bindIndex= []( const auto i, const auto e ) { return std::tuple_cat( i, e ); };
		using indexed_table_entry= decltype( bindIndex( indices.front(), original.front() ) );
		std::vector< indexed_table_entry > rv;
		std::transform( begin( indices ), end( indices ), begin( original ), std::back_inserter( rv ), bindIndex );
		return rv;
	}

	template< FunctionVariable auto function, OutputMode outputMode >
	struct exports::TableTest
	{
		using function_traits_type= function_traits< decltype( function ) >;

		using args_type= Meta::product_type_decay_t< typename function_traits_type::args_type >;
		using return_type= typename function_traits_type::return_type;

		using ComputedBase= compute_base_t< return_type >;

		struct UniversalCases
		{
			using RunDescription= std::tuple< std::string, args_type, return_type >;
			using Invoker= std::function< return_type () >;

			using UniversalHandler= BasicUniversalHandler< return_type, outputMode >;

			using TestDescription= std::tuple< std::string, args_type, UniversalHandler >;
			std::vector< TestDescription > tests;

			UniversalCases( std::initializer_list< TestDescription > initList )
				: tests( initList )
			{
				for( const auto &desc: initList )
				{
					if constexpr( Aggregate< return_type > )
					{
						std::cerr << "Case: " << std::get< 0 >( desc );
						const return_type &v= std::get< 2 >( desc );
						std::cerr << " (" << v << ")" << std::endl;
						tests.push_back( desc );
					}
				}
			}

			int
			operator() () const
			{
				int failureCount= 0;
				for( const auto &[ comment, params, checker ]: tests )
				{
					if( C::debugCaseTypes ) std::cerr << boost::core::demangle( typeid( params ).name() ) << std::endl;
					auto invoker= [&]
					{
						breakpoint();
						return std::apply( function, params );
					};
					const TestResult result= checker( invoker, comment );
					if( result == TestResult::Failed )
					{
						std::cout << "    " << C::testFail << "FAILED CASE" << resetStyle << ": " << comment << std::endl;
						++failureCount;
					}
					else std::cout << "    " << C::testPass << "PASSED CASE" << resetStyle << ": " << comment << std::endl;
					breakpoint();
				}

				return failureCount;
			}
		};

		// When the `UniversalCases` impl is ready to go, then this alias shim can be redirected to that form.  Then I can
		// retire the `ExceptionCases` and `ExecutionCases` forms and replace them with an alias to `UniversalCases`.
		//using Cases= ExecutionCases;
		using Cases= UniversalCases;

		//using ExceptionCases= ExceptionCases_real;
		using ExceptionCases= UniversalCases;
	};
}

namespace Alepha::Hydrogen::Testing::inline exports::inline table_test
{
	using namespace detail::table_test::exports;
}
