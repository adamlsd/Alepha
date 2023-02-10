static_assert( __cplusplus > 2020'00 );

#include <Alepha/Reflection/tuplizeAggregate.h>

#include <Alepha/Testing/test.h>
#include <Alepha/types.h>
#include <Alepha/Meta/product_type_decay.h>

using Alepha::argcnt_t, Alepha::argvec_t;

int
main( const argcnt_t argcnt, const argvec_t argvec )
{
	return Alepha::Testing::runAllTests( argcnt, argvec );
}

namespace
{
	using namespace Alepha::Testing::literals;
	struct instance
	{
		int a;
		float b;
		char c;
		double d;
	};

	static_assert( std::tuple_size_v< decltype( Alepha::Reflection::tuplizeAggregate( std::declval< instance >() ) ) > == 4 );
	static_assert( Alepha::Reflection::aggregate_empty_bases_v< instance > == 0 );
	static_assert( std::is_same_v
		<
			Alepha::Meta::product_type_decay_t< decltype( Alepha::Reflection::tuplizeAggregate( std::declval< instance >() ) ) >,
			std::tuple< int, float, char, double >
		> );

	struct instance2 : instance
	{
		int a;
		float b;
		char c;
		double d;
	};

	// Apparently decomposibility is not quite the same as aggregate.
	// We'll need a way to do this right?
	//static_assert( not std::is_aggregate_v< instance2 > );

	struct empty1 {};
	struct empty2 {};
	struct empty3 {};

	struct instance3 : empty1, empty2
	{
		empty3 e;
		int a;
		float b;
		char c;
		double d;
	};

	static_assert( Alepha::Reflection::aggregate_initializer_size_v< instance3 > == 7 );
	static_assert( Alepha::Reflection::aggregate_empty_bases_v< instance3 > == 2 );
	auto t= "test"_test <=[]
	{
		using namespace Alepha::Reflection::detail::aggregate_members;
		std::cout << Alepha::Reflection::aggregate_empty_bases_v< instance3 > << std::endl;

		static_assert( std::is_empty_v< empty1 > );
		static_assert( is_constructible_from_tuple_v< instance3, std::tuple< empty_base< instance3 >, empty_base< instance3 > > > );
	};
	static_assert( std::tuple_size_v< decltype( Alepha::Reflection::tuplizeAggregate( std::declval< instance3 >() ) ) > == 5 );
	static_assert( std::is_same_v
		<
			Alepha::Meta::product_type_decay_t< decltype( Alepha::Reflection::tuplizeAggregate( std::declval< instance3 >() ) ) >,
			std::tuple< empty3, int, float, char, double >
		> );
}
