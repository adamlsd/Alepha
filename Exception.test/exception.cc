static_assert( __cplusplus > 2020'00 );

#include <Alepha/Exception.h>

#include <type_traits>

#include <Alepha/Testing/test.h>
#include <Alepha/Testing/TableTest.h>
#include <Alepha/Utility/evaluation.h>

namespace
{
	using Alepha::exports::types::argcnt_t;
	using Alepha::exports::types::argvec_t;
}

int
main( const argcnt_t argcnt, const argvec_t argvec )
{
	return Alepha::Testing::runAllTests( argcnt, argvec );
}

namespace
{
	namespace util= Alepha::Utility;
	using namespace Alepha::Testing::exports;

	struct tag;

	template< typename CatchException >
	void
	testException()
	{
		try
		{
			throw Alepha::build_exception< Alepha::TaggedAllocationException< struct tag > >( "This is my allocation exception" );
		}
		catch( const CatchException &ex )
		{
			std::cout << "Caught as `" << boost::core::demangle( typeid( CatchException ).name() ) << "` : ";
			if constexpr( std::is_base_of_v< std::exception, CatchException > ) std::cout << ex.what();
			else std::cout << ex.message();
			std::cout << std::endl;

			if constexpr( std::is_base_of_v< Alepha::AnyTaggedThrowable, CatchException > )
			{
				std::cout << "Tag type:       `" << boost::core::demangle( ex.tag().name() ) << '`' << std::endl;
			}
			//std::cout << "Local tag type: `" << typeid( std::type_identity< struct tag > ).name() << '`' << std::endl;
		}
	}

	template< typename BuildException, typename CatchException >
	bool
	catchable()
	{
		try
		{
			throw Alepha::build_exception< BuildException >( "This is my allocation exception" );
		}
		catch( const CatchException &ex ) { std::cerr << "Caught" << std::endl; return true; }
		catch( ... ) { std::cerr << "Not caught" << std::endl; return false; }
	}

	auto tests= Alepha::Utility::enroll <=[]
	{
		"smoke"_test <=[] () -> bool
		{
			testException< std::bad_alloc >();
			testException< Alepha::TaggedException< tag > >();
			testException< Alepha::AnyTaggedException >();
			testException< std::exception >();
			testException< Alepha::Exception >();
			testException< Alepha::Throwable >();

			return true;
		};

		"catch"_test <=catchable< Alepha::TaggedAllocationException< tag >, std::bad_alloc >;
		"catch"_test <=catchable< Alepha::TaggedAllocationException< tag >, std::exception >;
		"catch"_test <=catchable< Alepha::TaggedAllocationException< tag >, Alepha::Throwable >;
		"catch"_test <=catchable< Alepha::TaggedAllocationException< tag >, Alepha::AnyTaggedThrowable >;
		"catch"_test <=catchable< Alepha::TaggedAllocationException< tag >, Alepha::TaggedThrowable< tag > >;
		"catch"_test <=catchable< Alepha::TaggedAllocationException< tag >, Alepha::Exception >;
		"catch"_test <=catchable< Alepha::TaggedAllocationException< tag >, Alepha::AnyTaggedException >;
		"catch"_test <=catchable< Alepha::TaggedAllocationException< tag >, Alepha::TaggedException< tag > >;
		"catch"_test <=catchable< Alepha::TaggedAllocationException< tag >, Alepha::AllocationThrowable >;
		"catch"_test <=catchable< Alepha::TaggedAllocationException< tag >, Alepha::AnyTaggedAllocationThrowable >;
		"catch"_test <=catchable< Alepha::TaggedAllocationException< tag >, Alepha::TaggedAllocationThrowable< tag > >;
		"catch"_test <=catchable< Alepha::TaggedAllocationException< tag >, Alepha::AllocationException >;
		"catch"_test <=catchable< Alepha::TaggedAllocationException< tag >, Alepha::AnyTaggedAllocationException >;
		"catch"_test <=catchable< Alepha::TaggedAllocationException< tag >, Alepha::TaggedAllocationException< tag > >;
	};
}
