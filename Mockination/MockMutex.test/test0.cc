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

	auto test= "basic locking smoke test"_test <=[]
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

		// Test allow/waitLocked/waitUnlocked
		auto thread1= Alepha::Truss::thread{ lockingCode };

		print() << "Control will now sleep for 2 seconds to let slave into lock." << std::endl;
		sleep( 2 );

		print() << "Control will now permit thread 1 into the lock, in 2 seconds." << std::endl;

		sleep( 2 );

		mtx.allowFirst();
		mtx.waitLocked();
		mtx.waitUnlocked();

		thread1.join();

		// Test allow/waitUnlocked
		thread1= Alepha::Truss::thread{ lockingCode };

		print() << "Control will now sleep for 2 seconds to let slave into lock." << std::endl;
		sleep( 2 );

		print() << "Control will now permit thread 1 into the lock, in 2 seconds." << std::endl;

		sleep( 2 );

		mtx.allowFirst();
		mtx.waitUnlocked();

		thread1.join();

		// Test allowFirstWait
		thread1= Alepha::Truss::thread{ lockingCode };

		print() << "Control will now sleep for 2 seconds to let slave into lock." << std::endl;
		sleep( 2 );

		print() << "Control will now permit thread 1 into the lock, in 2 seconds." << std::endl;

		sleep( 2 );

		mtx.allowFirstWait();

		thread1.join();
	};

	auto test2= "basic locking correctness"_test <=[]
	{
		Alepha::Mockination::MockMutexImpl mtx;

		assert( !mtx.hasWaiters() );
		assert( !mtx.locked() );

		print() << "Creating thread." << std::endl;
		auto thread1= Alepha::Truss::thread
		{
			[&mtx]
			{
				print() << "Going to lock" << std::endl;
				mtx.lock();
				print() << "Locked" << std::endl;
				assert( mtx.locked() );
				auto waiters= mtx.getWaiters();
				assert( !mtx.hasWaiters() );
				assert( mtx.getWaiters().size() == 0 );
				assert( waiters.empty() );
				assert( waiters.size() == 0 );
				mtx.unlock();
				print() << "Unlocked" << std::endl;
			}
		};
		assert( !mtx.locked() );
		while( !mtx.hasWaiters() ); // Wait for some waiters...

		assert( mtx.getWaiters().size() == 1 );
		assert( mtx.getWaiters().at( 0 ) == thread1.get_id() );

		print() << "Control will now permit thread 1 into the lock." << std::endl;

		mtx.allowFirst();

		mtx.waitLocked();
		assert( mtx.locked() );
		assert( !mtx.hasWaiters() );
		assert( mtx.getWaiters().size() == 0 );

		mtx.waitUnlocked();
		assert( !mtx.locked() );

		auto waiters= mtx.getWaiters();
		assert( !mtx.hasWaiters() );
		assert( mtx.getWaiters().size() == 0 );
		assert( waiters.empty() );
		assert( waiters.size() == 0 );

		thread1.join();

		assert( !mtx.locked() );
		assert( mtx.getWaiters().size() == 0 );
	};
}
