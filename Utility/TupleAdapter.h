static_assert( __cplusplus > 2020'00 );

#pragma once

#include <Alepha/Alepha.h>

#include <Alepha/type_lisp.h>

#include <Alepha/Reflection/tuplizeAggregate.h>

namespace Alepha::Hydrogen::Utility  ::detail::  TupleAdapter_m
{
	inline namespace exports {}

	template< Aggregate, TypeListType >
	struct TupleAdapter_impl;

	namespace exports
	{
		template< Aggregate Agg >
		using TupleAdapter= TupleAdapter_impl
		<
			Agg,
			list_from_tuple_t< Reflection::aggregate_tuple_t< Agg > >
		>;
	}

	template< Aggregate Agg >
	struct TupleAdapter_impl< Agg, Nil >
		: Agg
	{
		TupleAdapter_impl()= default;

		protected:
			void set( Agg agg ) { static_cast< Agg & >( *this )= agg; }
	};

	template< Aggregate Agg, typename ... Args >
	struct TupleAdapter_impl< Agg, TypeList< Args... > >
		: TupleAdapter_impl< Agg, cdr_t< TypeList< Args... > > >
	{
		using Parent= TupleAdapter_impl< Agg, cdr_t< TypeList< Args... > > >;
		using Parent::Parent;

		TupleAdapter_impl( Args ... args )
		{
			this->set( { args... } );
		}
	};
}

namespace Alepha::Hydrogen::Utility::inline exports::inline TupleAdapter_m
{
	using namespace detail::TupleAdapter_m::exports;
}
