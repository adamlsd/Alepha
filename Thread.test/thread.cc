static_assert( __cplusplus > 2020'00 );

#include <Alepha/Thread.h>

#include <type_traits>

#include <Alepha/Testing/test.h>
#include <Alepha/Testing/TableTest.h>
#include <Alepha/Utility/evaluation.h>

namespace
{
	using Alepha::exports::types::argcnt_t;
	using Alepha::exports::types::argvec_t;

	namespace util= Alepha::Utility;
	using namespace Alepha::Testing::exports;

	using MyNotification= Alepha::create_exception< struct my_notification, Alepha::Notification >;

	auto tests= Alepha::Utility::enroll <=[]
	{
		"smoke"_test <=[] () -> bool
		{
			std::cerr << "Smoke started..." << std::endl;
			Alepha::Mutex access;
			Alepha::ConditionVariable cv;
			auto threadMain= [&]
			{
				try
				{
					Alepha::unique_lock lock( access );
					std::cerr << "Child thread started..." << std::endl;
					cv.wait( lock );
					std::cerr << "Child thread awoken illegally!" << std::endl;
				}
				catch( const boost::thread_interrupted & )
				{
					std::cerr << "SHIT!  We didn't get intercepted!" << std::endl;
					throw;
				}
				catch( const MyNotification &n )
				{
					std::cerr << "I caught it: " << n.message() << "!" << std::endl;
					throw;
				}
			};

			access.lock();
			std::cerr << "Launching child thread..." << std::endl;
			Alepha::Thread thr( threadMain );
			std::cerr << "Child thread now launched..." << std::endl;
			::sleep( 1 );
			access.unlock();
			thr.interrupt( Alepha::build_exception< MyNotification >( "My message" ) );
			thr.join();

			return true;
		};
	};
}
