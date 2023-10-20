static_assert( __cplusplus > 2020'00 );

#include <Alepha/Exception.h>

#include <type_traits>

#include <Alepha/Testing/test.h>
#include <Alepha/Testing/TableTest.h>
#include <Alepha/Utility/evaluation_helpers.h>

namespace
{
	using Alepha::Hydrogen::exports::types::argcnt_t;
	using Alepha::Hydrogen::exports::types::argvec_t;
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
			auto exc= Alepha::build_exception< Alepha::TaggedAllocationError< struct tag > >( "This is my allocation exception" );
			static_assert( std::is_same_v< decltype( exc )::grade_type, Alepha::Error > );

			static_assert( std::is_same_v< Alepha::AnyTaggedException::grade_type, Alepha::Exception > );

			static_assert( std::is_same_v< typename Alepha::TaggedException< struct bob >::grade_type, Alepha::Exception > );

			static_assert( std::is_same_v< typename Alepha::TaggedCondition< struct bob >::grade_type, Alepha::Condition > );
			static_assert( std::is_same_v< typename Alepha::TaggedNotification< struct bob >::grade_type, Alepha::Notification > );
			static_assert( std::is_same_v< typename Alepha::TaggedError< struct bob >::grade_type, Alepha::Error > );
			static_assert( std::is_same_v< typename Alepha::TaggedCriticalError< struct bob >::grade_type, Alepha::CriticalError > );
			static_assert( std::is_same_v< typename Alepha::TaggedViolation< struct bob >::grade_type, Alepha::Violation > );
			throw exc;
		}
		catch( const CatchException &ex )
		{
			std::cout << "Caught as `" << boost::core::demangle( typeid( CatchException ).name() ) << "` : ";
			if constexpr( std::is_base_of_v< std::exception, CatchException > ) std::cout << ex.what();
			else std::cout << ex.message();
			std::cout << std::endl;

			if constexpr( std::is_base_of_v< Alepha::AnyTaggedException, CatchException > )
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
			testException< Alepha::TaggedError< tag > >();
			testException< Alepha::AnyTaggedError >();
			testException< std::exception >();
			testException< Alepha::Exception >();
			testException< Alepha::Error >();

			return true;
		};

		"catch.std::bad_alloc"_test <=catchable< Alepha::TaggedAllocationError< tag >, std::bad_alloc >;
		"catch.std::exception"_test <=catchable< Alepha::TaggedAllocationError< tag >, std::exception >;
		"catch.Alepha::Exception"_test <=catchable< Alepha::TaggedAllocationError< tag >, Alepha::Exception >;
		"catch.Alepha::AnyTaggedException"_test <=catchable< Alepha::TaggedAllocationError< tag >, Alepha::AnyTaggedException >;
		"catch.Alepha::TaggedException"_test <=catchable< Alepha::TaggedAllocationError< tag >, Alepha::TaggedException< tag > >;
		"catch.Alepha::Error"_test <=catchable< Alepha::TaggedAllocationError< tag >, Alepha::Error >;
		"catch.Alepha::AnyTaggedError"_test <=catchable< Alepha::TaggedAllocationError< tag >, Alepha::AnyTaggedError >;
		"catch.Alepha::TaggedError"_test <=catchable< Alepha::TaggedAllocationError< tag >, Alepha::TaggedError< tag > >;
		"catch.Alepha::AllocationException"_test <=catchable< Alepha::TaggedAllocationError< tag >, Alepha::AllocationException >;
		"catch.Alepha::AnyTaggedAllocationException"_test <=catchable< Alepha::TaggedAllocationError< tag >, Alepha::AnyTaggedAllocationException >;
		"catch.Alepha::TaggedAllocationException"_test <=catchable< Alepha::TaggedAllocationError< tag >, Alepha::TaggedAllocationException< tag > >;
		"catch.Alepha::AllocationError"_test <=catchable< Alepha::TaggedAllocationError< tag >, Alepha::AllocationError >;
		"catch.Alepha::AnyTaggedAllocationError"_test <=catchable< Alepha::TaggedAllocationError< tag >, Alepha::AnyTaggedAllocationError >;
		"catch.Alepha::TaggedAllocationError"_test <=catchable< Alepha::TaggedAllocationError< tag >, Alepha::TaggedAllocationError< tag > >;

		"size_probe"_test <=[]
		{
			std::cout << "Size: " << sizeof( Alepha::build_exception< Alepha::TaggedAllocationError< tag > >( "Message" ) ) << std::endl;
		};
	};
}
