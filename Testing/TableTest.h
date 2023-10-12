static_assert( __cplusplus > 2020'00 );

#pragma once

#include <Alepha/Alepha.h>

#include <tuple>
#include <string>
#include <iostream>
#include <algorithm>
#include <typeinfo>

#include <boost/core/demangle.hpp>

#include <Alepha/Meta/is_vector.h>
#include <Alepha/Meta/product_type_decay.h>
#include <Alepha/Utility/evaluation.h>
#include <Alepha/console.h>

namespace Alepha::Hydrogen::Testing
{
	inline namespace exports { inline namespace table_test {} }

	namespace detail::table_test
	{
		inline namespace exports
		{
			template< auto > struct TableTest;
		}

		inline void breakpoint() {}

		namespace C
		{
			const bool debug= false;
			const bool debugCaseTypes= false or C::debug;
			using namespace Alepha::console::C;
		}

		using namespace Utility::exports::evaluation;

		template< typename RetVal, typename ... Args, RetVal (*function)( Args... ) >
		struct exports::TableTest< function >
		{
			using args_type= Meta::product_type_decay_t< std::tuple< Args... > >;

			struct Cases
			{
				using TestDescription= std::tuple< std::string, args_type, RetVal >;

				std::vector< TestDescription > tests;

				explicit
				Cases( std::initializer_list< TestDescription > initList )
					: tests( initList ) {}

				int
				operator() () const
				{
					int failureCount= 0;
					for( const auto &[ comment, params, expected ]: tests )
					{
						if( C::debugCaseTypes ) std::cerr << boost::core::demangle( typeid( params ).name() ) << std::endl;
						breakpoint();
						if( std::apply( function, params ) != expected )
						{
							std::cout << C::red << "  FAILURE" << C::normal << ": " << comment << std::endl;
							++failureCount;
						}
						else std::cout << C::green << "  SUCCESS" << C::normal << ": " << comment << std::endl;
					}

					return failureCount;
				}
			};

			//struct VectorCases;
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

	namespace exports::table_test
	{
		using namespace detail::table_test::exports;
	}
}
