static_assert( __cplusplus > 2020'00 );

#pragma once

#include <Alepha/Alepha.h>

#include <cassert>

#include <iostream>
#include <algorithm>
#include <functional>

#include <Alepha/Constexpr/algorithm.h>
#include <Alepha/Constexpr/array.h>

#include <Alepha/Meta/Container/vector.h>

namespace Alepha::Hydrogen::Meta
{
	inline namespace exports { inline namespace type_algorithm {} }

	namespace detail::type_algorithm
	{
		inline namespace exports
		{
			template< typename T > struct unique_for_type_binder;

			template< typename T > constexpr auto unique_for_type= unique_for_type_binder< T >::build();
		}

		using Container::vector;

		template< typename, typename > struct type_less;

		//template< typename A >
		//struct type_less< A, A > : std::false_type {};

		constexpr bool
		char_string_less( const char *left, const char *right )
		{
			// std::cerr << "Comparing..." << std::endl;
			// std::cerr << left << std::endl;
			// std::cerr << right << std::endl;
			while( *left and *right )
			{
				// std::cerr << "Comparing \"" << *left << "\" to \"" << *right << '"' << std::endl;
				if( *left < *right ) return true;
				if( *left > *right ) return false;
				assert( *left == *right );
				++left; ++right;
			}
			// std::cerr << "Ran out of chars..." << std::endl;
			if( *left == *right ) return false;
			if( *left ) return false;
			return true;
		}

		constexpr bool
		char_string_eq( const char *left, const char *right )
		{
			while( *left and *right )
			{
				if( *left++ != *right++ ) return false;
			}
			return *left == *right;
		}

		template< typename T >
		constexpr const char *
		single_type_value()
		{
			return __PRETTY_FUNCTION__;
		}

		template< typename A, typename B >
		constexpr bool
		compute()
		{
			return char_string_less( single_type_value< A >(), single_type_value< B >() );
		}

		template< typename A, typename B >
		struct type_less
		{
			static const bool value= compute< A, B >();
		};

		template< typename A, typename B >
		constexpr bool type_less_v= type_less< A, B >::value;

		template< typename T >
		constexpr void
		constexpr_swap( T &a, T &b ) noexcept
		{
			T tmp= std::move( a );
			a= std::move( b );
			b= std::move( tmp );
		}

		template< typename Iter, typename Comp >
		constexpr void
		constexpr_sort( Iter first, Iter last, Comp comp )
		{
			if( first == last ) return;
			const auto pos= std::min_element( first, last, comp );
			using std::swap;
			constexpr_swap( *pos, *first );
			constexpr_sort( ++first, last, comp );
		}

		namespace exports
		{
			template< typename ... Types >
			constexpr bool
			all_unique() noexcept
			{
				std::array< const char *, sizeof...( Types ) > magic_values= { single_type_value< Types >()... };
				using std::begin, std::end;

				const std::size_t amt= magic_values.size();
				if constexpr( amt <= 1 ) return true;
				else
				{
					constexpr_sort( begin( magic_values ), end( magic_values ), &char_string_less );
					for( auto pos= begin( magic_values ); pos < end( magic_values ) - 1; ++pos )
					{
						if( char_string_eq( *pos, *( pos + 1 ) ) ) return false;
					}
					return true;
				}
			}
		}

		template< typename ... Types >
		struct lowest_type_impl;

		template< typename Type >
		struct lowest_type_impl< Type >
		{
			using type= Type;
		};

		template< typename Type, typename ... Types >
		struct lowest_type_impl< Type, Types... >
		{
			using later_lowest= typename lowest_type_impl< Types... >::type;

			using type= std::conditional_t
			<
				type_less_v< Type, later_lowest >,
				Type,
				later_lowest
			>;
		};

		namespace exports
		{
			template< typename ... Types >
			using lowest_type_t= typename lowest_type_impl< Types... >::type;
		}

		template< typename Left, typename Right >
		struct concat_vectors;

		template< typename ... Left, typename ... Right >
		struct concat_vectors< vector< Left... >, vector< Right... > >
		{
			using type= vector< Left..., Right... >;
		};

		template< typename Left, typename Right >
		using concat_vectors_t= typename concat_vectors< Left, Right >::type;

		template< typename Type, typename Group >
		struct remove_type_impl;

		template< typename Type, typename ... Types >
		struct remove_type_impl< Type, vector< Type, Types... > >
		{
			using type= vector< Types... >;
		};

		template< typename Type, typename First, typename ... Types >
		struct remove_type_impl< Type, vector< First, Types... > >
		{
			using type= concat_vectors_t< vector< First >, typename remove_type_impl< Type, vector< Types... > >::type >;
		};

		template< typename Type, typename Vector >
		using remove_type_t= typename remove_type_impl< Type, Vector >::type;

		template< typename Type >
		struct type_sort_vector;

		template< typename Type >
		struct type_sort_vector< vector< Type > >
		{
			using type= vector< Type >;
		};

		template< typename ... Types >
		struct type_sort_vector< vector< Types... > >
		{
			using lowest= lowest_type_t< Types... >;
			using type= concat_vectors_t< vector< lowest >, remove_type_t< lowest, vector< Types... > > >;
		};

		namespace exports
		{
			template< typename ... Types >
			using type_sort_t= typename type_sort_vector< vector< Types... > >::type;

			using detail::type_algorithm::type_less_v;
			using detail::type_algorithm::single_type_value;
			using detail::type_algorithm::char_string_less;
		}
	}

	namespace exports::type_algorithm
	{
		using namespace detail::type_algorithm::exports;
	}
}
