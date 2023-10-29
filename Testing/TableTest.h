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

#include <Alepha/Meta/is_vector.h>
#include <Alepha/Meta/is_optional.h>
#include <Alepha/Meta/is_streamable.h>
#include <Alepha/Meta/is_sequence.h>
#include <Alepha/Meta/is_product_type.h>

#include <Alepha/Meta/product_type_decay.h>
#include <Alepha/Meta/sequence_kind.h>

#include <Alepha/function_traits.h>
#include <Alepha/template_for_each.h>

#include <Alepha/IOStreams/String.h>

#include <Alepha/Utility/evaluation_helpers.h>

#include <Alepha/Reflection/tuplizeAggregate.h>

#include <Alepha/TotalOrder.h>
#include <Alepha/Console.h>

#include "colors.h"

namespace Alepha::Hydrogen::Testing  ::detail::  table_test
{
	inline namespace exports
	{
		enum class OutputMode { All, Relaxed };
	}

	inline void breakpoint() {}

	namespace C
	{
		inline namespace Colors
		{
			using namespace testing_colors::C::Colors;
		}
	}

	template< OutputMode outputMode, typename T >
	void printDebugging( const T &witness, const T &expected );

	template< Aggregate Agg, TypeListType >
	struct TupleSneak;

	template< Aggregate Agg >
	struct TupleSneak< Agg, Nil >
		: Agg
	{
		TupleSneak() { std::cerr << "The inherited default ctor was called." << std::endl; }

		protected:
			void set( Agg agg ) { static_cast< Agg & >( *this )= agg; }
	};

	template< Aggregate Agg, typename ... Args >
	struct TupleSneak< Agg, TypeList< Args... > >
		: TupleSneak< Agg, cdr_t< TypeList< Args... > > >
	{
		using Parent= TupleSneak< Agg, cdr_t< TypeList< Args... > > >;
		using Parent::Parent;

		TupleSneak( Args ... args )
		{
			std::cerr << "I was the ctor called, with " << sizeof...( Args ) << " arguments." << std::endl;
			tuple_for_each( std::tuple{ args... } ) <=
			[]( const auto element )
			{
				std::cerr << "Element: " << element << std::endl;
			};
			this->set( { args... } );
		}
	};

	enum class TestResult { Passed, Failed };

	struct BlankBase {};

	template< typename T >
	static consteval auto
	compute_base_f() noexcept
	{
		if constexpr ( Aggregate< T > ) return std::type_identity< TupleSneak< T, list_from_tuple_t< Reflection::aggregate_tuple_t< T > > > >{};
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

#if 0
		template< typename ... Args >
		requires ConstructibleFrom< return_type, std::decay_t< Args >... >
		BasicUniversalHandler( Args &&... expected_init )
			: BasicUniversalHandler( return_type{ std::forward< Args >( expected_init )... } )
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

#if 0
		template< typename T_= return_type, typename= std::enable_if_t< not std::is_class_v< std::decay_t< T_ > > > >
		BasicUniversalHandler( const T_ expected )
		: impl
		{
			[expected]( Invoker invoker, const std::string &comment )
			{
				static_assert( not Aggregate< T_ > );
				static_assert( not std::is_class_v< T_ > );
				const return_type witness= invoker();
				const auto result= witness == expected ? TestResult::Passed : TestResult::Failed;
				
				if( result == TestResult::Failed )
				{
					std::cout << "    " << C::testFail << "FAILED CASE" << resetStyle << ": " << comment << std::endl;
					printDebugging< outputMode >( witness, expected );
				}
				else std::cout << "    " << C::testPass << "PASSED CASE" << resetStyle << ": " << comment << std::endl;
				return result;
			}
		}
		{}
#endif

#if 0
		template< typename ... Args >
		requires ConstructibleFrom< return_type, std::decay_t< Args >... >
		BasicUniversalHandler( Args &&... expected_init )
			: BasicUniversalHandler( return_type{ std::forward< Args >( expected_init )... } )
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

#if 0
		template< typename T_= return_type, typename= std::enable_if_t< not std::is_class_v< std::decay_t< T_ > > > >
		BasicUniversalHandler( const T_ expected )
		: impl
		{
			[expected]( Invoker invoker, const std::string &comment )
			{
				static_assert( not Aggregate< T_ > );
				static_assert( not std::is_class_v< T_ > );
				const return_type witness= invoker();
				const auto result= witness == expected ? TestResult::Passed : TestResult::Failed;
				
				if( result == TestResult::Failed )
				{
					std::cout << "    " << C::testFail << "FAILED CASE" << resetStyle << ": " << comment << std::endl;
					printDebugging< outputMode >( witness, expected );
				}
				else std::cout << "    " << C::testPass << "PASSED CASE" << resetStyle << ": " << comment << std::endl;
				return result;
			}
		}
		{}
#endif

#if 0
		template< typename ... Args >
		requires ConstructibleFrom< return_type, std::decay_t< Args >... >
		BasicUniversalHandler( Args &&... expected_init )
			: BasicUniversalHandler( return_type{ std::forward< Args >( expected_init )... } )
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
	using namespace std::literals::string_literals;

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

	template< OutputMode outputMode, typename T >
	std::string
	stringifyValue( const T &v )
	{
		std::ostringstream oss;
		if constexpr( false ) ; // To keep the rest of the clauses regular
		else if constexpr( std::is_same_v< std::uint8_t, std::decay_t< T > > )
		{
			oss << std::hex << std::setw( 2 ) << std::setfill( '0' ) << int( v );
		}
		else if constexpr( std::is_same_v< bool, std::decay_t< T > > )
		{
			oss << std::boolalpha << v;
		}
		else if constexpr( std::is_same_v< std::string, std::decay_t< T > > )
		{
			oss << "(String with " << v.size() << " chars)";
			oss << '\n' << R"(""")" << '\n';
			for( const char ch: v )
			{
				if( ch == '\n' ) oss << "<EOL>\n";
				else if( std::isalnum( ch ) or std::ispunct( ch ) or ( ch == ' ' ) ) oss << ch;
				else oss << "<\\0x" << std::hex << std::setw( 2 ) << std::setfill( '0' ) << unsigned( ch ) << '>';
			}
			oss << '\n' << R"(""")";
		}
		else if constexpr( Meta::is_ostreamable_v< T > )
		{
			return IOStreams::stringify( v );
		}
		else if constexpr( Meta::is_optional_v< T > )
		{
			return v.has_value() ? stringifyValue< outputMode >( v.value() ) : "<noopt>"s;
		}
		else if constexpr( Meta::is_sequence_v< T > )
		{
			if constexpr( outputMode == OutputMode::Relaxed and not Meta::is_ostreamable_v< typename T::value_type > )
			{
				oss << "<Unstreamable sequence of " << v.size() << " elements.>";
			}
			else
			{
				oss << Meta::sequence_kind_v< T > << "(" << v.size() << " elements):\n{" << std::endl;

				int index= 0;
				for( const auto &elem: v ) oss << "\t" << index++ << ":  " << stringifyValue< outputMode >( elem ) << "," << std::endl;
				oss << "}" << std::endl;
			}
		}
		else if constexpr( Meta::is_pair_v< T > )
		{
			const auto &[ first, second ]= v;
			return stringifyValue< outputMode >( std::tie( first, second ) );
		}
		else if constexpr( Meta::is_tuple_v< T > )
		{
			oss << '[';
			tuple_for_each( v ) <=[&oss, first= true]( const auto &elem ) mutable
			{
				if( not first ) oss << ", ";
				first= false;
				oss << std::endl << stringifyValue< outputMode >( elem );
			};
			oss << std::endl << ']' << std::endl;
		}
		else if constexpr( std::is_same_v< T, TotalOrder > )
		{
			if( false ) ; // For alignment
			else if( v == TotalOrder::less ) oss << "less";
			else if ( v == TotalOrder::equal ) oss << "equal";
			else if( v == TotalOrder::greater ) oss << "greater";
			else throw std::logic_error( "Impossible `TotalOrder` condition." );
		}
		else
		{
			static_assert( dependent_value< false, T >, "One of the types used in the testing table does not support stringification." );
		}
		return std::move( oss ).str();
	}

	inline void
	printDebuggingForStrings( const std::string &witness, const std::string &expected )
	{
		const std::size_t amount= std::min( witness.size(), expected.size() );
		if( witness.size() != expected.size() )
		{
			std::cout << "Witness string size did not match the expected string size.  Only mismatches found in the first "
					<< amount << " characters will be printed." << std::endl;
		}

		for( int i= 0; i < amount; ++i )
		{
			if( witness.at( i ) == expected.at( i ) ) continue;
			std::cout << "Mismatch at index: " << i << std::endl;
			std::cout << "witness: " << witness.at( i ) << std::endl;
			std::cout << "expected: " << expected.at( i ) << std::endl;
		}
	}

	template< OutputMode outputMode, typename T >
	void
	printDebugging( const T &witness, const T &expected )
	{
		if constexpr( std::is_same_v< std::string, std::decay_t< T > > )
		{
			printDebuggingForStrings( witness, expected );
		}
		else if constexpr( Meta::is_sequence_v< T > )
		{
			if constexpr( std::is_same_v< std::string, typename T::value_type > )
			{
				if( witness.size() == expected.size() ) for( std::size_t i= 0; i < witness.size(); ++i )
				{
					if( witness.at( i ) != expected.at( i ) ) printDebuggingForStrings( witness.at( i ), expected.at( i ) );
				}
			}
			else
			{
				if( witness.size() != expected.size() )
				{
					std::cout << "Witness sequence size of " << witness.size() << " did not match the expected sequence size of "
							<< expected.size() << std::endl;
				}

				auto next= std::make_pair( begin( witness ), begin( expected ) );
				bool first= true;
				while( next.first != end( witness ) and next.second != end( expected ) )
				{
					if( not first )
					{
						std::cout << "Mismatch at witness index " << std::distance( begin( witness ), next.first ) << " and "
								<< "expected index " << std::distance( begin( expected ), next.second ) << std::endl;
						++next.first; ++next.second;
					}
					first= false;
					next= std::mismatch( next.first, end( witness ), next.second, end( expected ) );
				}
			}
		}

		std::cout << std::endl
				<< "computed: " << stringifyValue< outputMode >( witness ) << std::endl
				<< "expected: " << stringifyValue< outputMode >( expected ) << std::endl << std::endl;
	}

	template< FunctionVariable auto function, OutputMode outputMode >
	struct exports::TableTest
	{
		using function_traits_type= function_traits< decltype( function ) >;

		using args_type= Meta::product_type_decay_t< typename function_traits_type::args_type >;
		using return_type= typename function_traits_type::return_type;

		// The classic table-test engine would only support `Cases` which were run-and-test-value
		// without the ability to test exceptions.  The `ExceptionCases` construct was used to
		// test throwing cases.
		//
		// A unified `Cases` type is forthcoming, and thus `ExecutionCases` exists for backwards
		// compatibility.
		struct ExecutionCases
		{
			using TestDescription= std::tuple< std::string, args_type, return_type >;

			std::vector< TestDescription > tests;

			explicit
			ExecutionCases( std::initializer_list< TestDescription > initList )
				: tests( initList ) {}

			int
			operator() () const
			{
				int failureCount= 0;
				for( const auto &[ comment, params, expected ]: tests )
				{
					if( C::debugCaseTypes ) std::cerr << boost::core::demangle( typeid( params ).name() ) << std::endl;
					breakpoint();
					const auto witness= std::apply( function, params );
					const auto result= witness == expected;
					if( not result )
					{
						std::cout << "    " << C::testFail << "FAILED CASE" << resetStyle << ": " << comment << std::endl;
						++failureCount;
						printDebugging< outputMode >( witness, expected );
					}
					else std::cout << "    " << C::testPass << "PASSED CASE" << resetStyle << ": " << comment << std::endl;
				}

				return failureCount;
			}
		};

		struct ExceptionCases_real
		{
			using Invoker= std::function< void () >;
			struct ExceptionHandler
			{
				std::function< bool ( Invoker ) > impl;

				bool operator() ( Invoker invoker ) const { return impl( invoker ); }

				ExceptionHandler() : impl
				{
					[]( Invoker invoker )
					{
						try
						{
							invoker();
							return true;
						}
						catch( ... ) { return false; }
					}
				}
				{}

				template< typename T >
				requires( SameAs< T, std::type_identity< void > > or SameAs< T, std::nothrow_t > )
				ExceptionHandler( T ) : impl
				{
					[]( Invoker invoker )
					{
						try
						{
							invoker();
							return true;
						}
						catch( ... ) { return false; }
					}
				}
				{}

				template< typename T >
				requires( not SameAs< T, void > )
				ExceptionHandler( std::type_identity< T > ) : impl
				{
					[]( Invoker invoker )
					{
						try
						{
							invoker();
							return false;
						}
						catch( const T & ) { return true; }
					}
				}
				{}

				template< typename T >
				ExceptionHandler( const T exemplar ) : impl
				{
					[expected= std::string{ exemplar.what() }]( Invoker invoker )
					{
						try
						{
							invoker();
							std::cerr << "  " << C::testInfo << "NOTE" << resetStyle << ": expected exception `"<< typeid( T ).name()
									<< "` wasn't thrown." << std::endl;
							return false;
						}
						catch( const T &ex )
						{
							const std::string witness= ex.what();
							const bool rv= witness == expected;
							if( not rv )
							{
								std::cerr << "  " << C::testInfo << "NOTE" << resetStyle << ": expected exception `"<< typeid( T ).name()
										<< "` wasn't thrown." << std::endl;
								printDebugging< outputMode >( witness, expected );
							}
							return rv;
						}
					}
				}
				{}

				// This checker is invoked during `catch( ... )`
				ExceptionHandler( const std::function< bool () > checker ) : impl
				{
					[=]( Invoker invoker )
					{
						try
						{
							invoker();
							return false;
						}
						catch( ... ) { checker(); } // The `checker` can use `throw` to run any complex checks it needs.
					}
				}
				{}
			};

			using TestDescription= std::tuple< std::string, args_type, ExceptionHandler >;

			std::vector< TestDescription > tests;


			explicit
			ExceptionCases_real( std::initializer_list< TestDescription > initList )
				: tests( initList ) {}

			int
			operator() () const
			{
				int failureCount= 0;
				for( const auto &[ comment, params, checker ]: tests )
				{
					if( C::debugCaseTypes ) std::cerr << boost::core::demangle( typeid( params ).name() ) << std::endl;
					breakpoint();
					auto invoker= [&]{ std::apply( function, params ); };
					const auto result= checker( invoker );
					if( not result )
					{
						std::cout << "    " << C::testFail << "FAILED CASE" << resetStyle << ": " << comment << std::endl;
						++failureCount;
					}
					else std::cout << "    " << C::testPass << "PASSED CASE" << resetStyle << ": " << comment << std::endl;
				}

				return failureCount;
			}
		};
			
		using ComputedBase= compute_base_t< return_type >;

		struct UniversalCases
		{
			using RunDescription= std::tuple< std::string, args_type, return_type >;
			using Invoker= std::function< return_type () >;

			using UniversalHandler= BasicUniversalHandler< return_type, outputMode >;

			using TestDescription= std::tuple< std::string, args_type, UniversalHandler >;
			std::vector< TestDescription > tests;

			UniversalCases( std::initializer_list< TestDescription > initList )
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

#ifdef DISABLED
	template< typename RetVal, typename ... Args, RetVal (*function)( Args... ) >
	struct TableTest< function >::VectorCases
	{
		static_assert( sizeof...( Args ) == 1 );
		static_assert( Meta::is_vector_v< RetVal > );
		static_assert( Meta::is_vector_v< std::tuple_element_t< 0, std::tuple< Args... > > > );

		using TestDescription= std::tuple< std::string,
				std::vector< std::pair< typename std::tuple_element_t< 0, std::tuple< Args... > >::value_type, typename RetVal::value_type > > >;

		std::vector< TestDescription > tests;

		explicit
		VectorCases( std::initializer_list< TestDescription > initList )
			: tests( initList ) {}

		int
		operator() () const
		{
			int failureCount= 0;
			for( const auto &[ comment, productions ]: tests )
			{
				const auto expected= evaluate <=[&]
				{
					std::vector< RetVal > rv;
					std::transform( begin( productions ), end( productions ), back_inserter( rv ),
						[]( const auto &prod ) { return prod.second; } );
					return rv;
				};

				const auto params= evaluate <=[&]
				{
					std::vector< RetVal > rv;
					std::transform( begin( productions ), end( productions ), back_inserter( rv ),
						[]( const auto &prod ) { return prod.first; } );
					return rv;
				};

				if( std::apply( function, std::tuple{ params } ) != expected )
				{
					std::cout << "  FAILURE: " << comment << std::endl;
					++failureCount;
				}
				else std::cout << "  SUCCESS: " << comment << std::endl;
			}

			return failureCount;
		}
	};
#endif
}

namespace Alepha::Hydrogen::Testing::inline exports::inline table_test
{
	using namespace detail::table_test::exports;
}
