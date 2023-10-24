#include <Alepha/Mockination/MockMutex.h>
#include <Alepha/Testing/test.h>

#include <unistd.h>

#include <iostream>
#include <atomic>
#include <unordered_map>
#include <mutex>
#include <shared_mutex>

#include <Alepha/Truss/thread.h>

using namespace Alepha::Testing;

namespace
{
	static std::shared_timed_mutex access;
	static std::unordered_map< Alepha::Truss::thread::id, int > idMap;
	static std::atomic< int > next{ 0 };

	static int
	getSimpleThreadId()
	{
		{
			std::shared_lock< std::shared_timed_mutex > lock( access );
			auto found= idMap.find( Alepha::Truss::this_thread::get_id() );
			if( found != end( idMap ) ) return found->second;
		}

		std::unique_lock< std::shared_timed_mutex > lock( access );
		return idMap[ Alepha::Truss::this_thread::get_id() ]= next++;
	}
		
		
	static std::ostream &
	print()
	{
		const auto id= getSimpleThreadId();
		return std::cerr << "Thread " << id << ": ";
	}

	static auto test1= "smoke"_test <=[]
	{
		Alepha::Mockination::MockMutexImpl mtx;

		print() << "Creating thread." << std::endl;

		auto lockingCode= [&mtx]
		{
			print() << "Going to lock" << std::endl;
			mtx.lock();
			print() << "Locked" << std::endl;
			mtx.unlock();
			print() << "Unlocked" << std::endl;
		};

		// Test allow in first order
		auto thread1= Alepha::Truss::thread{ lockingCode };
		auto thread2= Alepha::Truss::thread{ lockingCode };

		print() << "Control will now wait for thread 1 to bump into the lock." << std::endl;
		mtx.waitForWaiter( thread1.get_id() );
		print() << "Control will now wait for thread 2 to bump into the lock." << std::endl;
		mtx.waitForWaiter( thread2.get_id() );

		print() << "Control will now permit thread 1 into the lock." << std::endl;
		mtx.allowWait( thread1.get_id() );

		print() << "Control will now permit thread 2 into the lock." << std::endl;
		mtx.allowWait( thread2.get_id() );

		thread1.join();
		thread2.join();

		// Test reverse order
		print() << "Control will now wait for thread 1 to bump into the lock." << std::endl;
		thread1= Alepha::Truss::thread{ lockingCode };
		print() << "Control will now wait for thread 2 to bump into the lock." << std::endl;
		thread2= Alepha::Truss::thread{ lockingCode };

		mtx.waitForWaiter( thread1.get_id() );
		mtx.waitForWaiter( thread2.get_id() );

		print() << "Control will now permit thread 2 into the lock." << std::endl;
		mtx.allowWait( thread2.get_id() );

		print() << "Control will now permit thread 1 into the lock." << std::endl;
		mtx.allowWait( thread1.get_id() );

		thread1.join();
		thread2.join();
	};
}
