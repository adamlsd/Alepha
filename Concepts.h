static_assert( __cplusplus > 2020'00 );

#pragma once

#include <concepts>
#include <type_traits>
#include <iosfwd>

#include "meta.h"
#include "function_traits.h"

namespace Alepha::inline Cavorite  ::detail::  core_concepts
{
	inline namespace exports
	{
		// This section re-capitalizes the standard library concept constraints.
		inline namespace wrap_std
		{
			template< typename T, typename U >
			concept SameAs= std::same_as< T, U >;

			template< typename T, typename ... Args >
			concept ConstructibleFrom= std::constructible_from< T, Args... >;

			template< typename T, typename U >
			concept ConvertibleTo= std::convertible_to< T, U >;

			template< typename T, typename Base >
			concept DerivedFrom= std::derived_from< T, Base >;

			template< typename T >
			concept FloatingPoint= std::floating_point< T >;

			template< typename T >
			concept Integral= std::integral< T >;
		}

		template< typename T, typename U >
		concept ConvertibleFrom= ConvertibleTo< U, T >;

		template< typename T, typename NonBase >
		concept NotDerivedFrom= not DerivedFrom< T, NonBase >;

		// This is potentially useful since `std::is_base_of` can peer past privacy...
		template< typename T, typename Base >
		concept HasBase= std::is_base_of_v< Base, T >;

		template< typename T, typename NonBase >
		concept LacksBase= not HasBase< T, NonBase >;

		template< typename T, typename Target >
		concept ConvertibleToButNotSameAs= true
			and not SameAs< T, Target >
			and ConvertibleTo< T, Target >
		;


		// Stream related concepts...
		template< typename T >
		concept OStreamable=
		requires( const T &t, std::ostream &os )
		{
			{ os << t } -> SameAs< std::ostream & >;
		};

		template< typename T >
		concept IStreamable=
		requires( const T &t, std::istream &is )
		{
			{ is >> t } -> SameAs< std::istream & >;
		};

		template< typename T >
		concept Streamable= OStreamable< T > and IStreamable< T >;


		// Various operator possible concepts:
		template< typename T >
		concept Addable=
		requires( const T &x, const T &y )
		{
			{ x + y };
		};

		template< typename Lhs, typename Rhs >
		concept LhsAddableTo=
		requires( const Lhs &lhs, const Rhs &rhs )
		{
			{ lhs + rhs };
		};

		template< typename Rhs, typename Lhs >
		concept RhsAddableTo=
		requires( const Lhs &lhs, const Rhs &rhs )
		{
			{ lhs + rhs };
		};


		template< typename A, typename B >
		concept AdditionAssignable=
		requires( A &a, const B &b )
		{
			{ a+= b };
		};

		template< typename A, typename B >
		concept SubtractionAssignable=
		requires( A &a, const B &b )
		{
			{ a-= b };
		};


		template< typename T, typename I >
		concept IndexibleBy=
		requires( T &t, const I &i )
		{
			{ t[ i ] };
		};

		template< typename T >
		concept Dereferencible=
		requires( T &t )
		{
			{ *t };
		};


		// Relational operator concepts...

		template< typename T >
		concept LessThanComparable=
		requires( const T &lhs, const T &rhs )
		{
			{ lhs < rhs } -> ConvertibleTo< bool >;
		};

		template< typename T >
		concept GreaterThanComparable=
		requires( const T &lhs, const T &rhs )
		{
			{ lhs > rhs } -> ConvertibleTo< bool >;
		};

		template< typename T >
		concept LessEqualComparable=
		requires( const T &lhs, const T &rhs )
		{
			{ lhs <= rhs } -> ConvertibleTo< bool >;
		};

		template< typename T >
		concept GreaterEqualComparable=
		requires( const T &lhs, const T &rhs )
		{
			{ lhs >= rhs } -> ConvertibleTo< bool >;
		};

		template< typename T >
		concept EqualityComparable=
		requires( const T &lhs, const T &rhs )
		{
			{ lhs == rhs } -> ConvertibleTo< bool >;
		};

		template< typename T >
		concept InequalityComparable=
		requires( const T &lhs, const T &rhs )
		{
			{ lhs != rhs } -> ConvertibleTo< bool >;
		};

		template< typename T >
		concept TotalOrderComparable=
		requires( const T &lhs, const T &rhs )
		{
			{ total_order( lhs, rhs ) } -> SameAs< std::strong_ordering >;
		};

		template< typename T >
		concept Comparable= true
			and LessThanComparable< T >
			and GreaterThanComparable< T >
			and LessEqualComparable< T >
			and GreaterEqualComparable< T >
			and EqualityComparable< T >
			and InequalityComparable< T >
		;

		template< typename T >
		concept Primitive= FloatingPoint< T > or Integral< T >;

		template< typename T >
		concept Aggregate= std::is_aggregate_v< T >;

		template< typename T >
		concept Functional=
		requires( const T &t )
		{
			{ std::function{ t } };
		};

		template< typename T >
		concept NotFunctional= not Functional< T >;

		template< typename T >
		concept UnaryFunction= Functional< T > and function_traits< T >::args_size == 1;

		template< typename T >
		concept StandardLayout= std::is_standard_layout_v< T >;

		template< typename T >
		concept StandardLayoutAggregate= StandardLayout< T > and Aggregate< T >;


		template< typename T >
		concept Array= is_array_v< T >;

		template< typename T, template< typename ... > class Ref >
		concept SpecializationOf= is_specialization_of_v< T, Ref >;

		template< typename T, template< typename ... > class Ref >
		concept NotSpecializationOf= not SpecializationOf< T, Ref >;

		template< typename T >
		concept Vector= SpecializationOf< T, std::vector >;

		template< typename T >
		concept List= SpecializationOf< T, std::list >;

		template< typename T >
		concept Deque= SpecializationOf< T, std::deque >;

		template< typename T >
		concept Sequence= false
			or Array< T >
			or Vector< T >
			or List< T >
			or Deque< T >
		;

		template< typename T, typename Member >
		concept SpecializedOn= is_specialized_on_v< T, Member >;

		template< typename Member, typename Seq >
		concept SequenceOf= Sequence< Seq > and SpecializedOn< Seq, Member >;

		template< typename T >                            
		concept Beginable=                                
		requires( const T &t )                            
		{                                                 
			{ begin( t ) };                               
		}                                                 
		or                                                
		requires( const T &t )                            
		{                                                 
			{ std::begin( t ) };                          
		};                                                
									  
		template< typename T >                            
		concept Endable=                                  
		requires( const T &t )                            
		{                                                 
			{ end( t ) };                                 
		}                                                 
		or                                                
		requires( const T &t )                            
		{                                                 
			{ std::end( t ) };                            
		};                                                
									  
		template< typename T >                            
		concept Iterable= Beginable< T > and Endable< T >;

		template< typename T >
		concept Tuple= is_tuple_v< T >;
	}
}

namespace Alepha::Cavorite::inline exports::inline core_concepts
{
	using namespace detail::core_concepts::exports;
}
