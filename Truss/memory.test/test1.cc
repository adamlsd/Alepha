#include <Alepha/Truss/memory.h>

#include <Alepha/assert.h>

#include <Alepha/Mockination/MockFunction.h>



namespace
{
	inline namespace Test1
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
	unique_ptr_to_ref_ptr()
	{
		auto p= Alepha::Truss::make_unique< std::string >( "Hello" );
		Alepha::Truss::ref_ptr< std::string > r= p.get();

		Alepha::Truss::ref_ptr< std::string > r2= r;
	}

	static void
	Test1::runTests()
	{
		unique_ptr_to_ref_ptr();
	}
}
