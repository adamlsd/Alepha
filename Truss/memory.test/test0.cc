#include <Alepha/Truss/memory.h>

#include <Alepha/assert.h>

#include <Alepha/Mockination/MockFunction.h>



namespace
{
	inline namespace Test0
	{
		static void runTests();
	}
}

int
main()
{
	runTests();
}

namespace
{
	static void
	simple_unique_ptr_test()
	{
		auto p= Alepha::Truss::make_unique< std::string >( "Hello" );
		Alepha::Truss::unique_ptr< std::string > p2= std::move( p );

		p= Alepha::Truss::make_unique< std::string >( "Hello" );

		using std::swap;
		swap( p, p2 );
	}

	static void
	unique_ptr_usage_test()
	{
		auto p= Alepha::Truss::make_unique< std::string >( "Hello" );

		std::string &s= *p;
		std::size_t len= p->size();
	}

	static void
	unique_ptr_capture_test()
	{
		auto p= Alepha::Truss::make_unique< std::string >( "Hello" );

		Alepha::single_ptr< std::string > s= p.get_raw();

		Alepha::Truss::ref_ptr< std::string > sp= p.get();
	}

	static void
	Test0::runTests()
	{
		simple_unique_ptr_test();
		unique_ptr_usage_test();
		unique_ptr_capture_test();
	}
}
