static_assert( __cplusplus > 2020'00 );

#pragma once

#include <Alepha/Alepha.h>

#include <utility>

#include <Alepha/Mockination/MockMutex.h>
#include <Alepha/ScopedUsage.h>

namespace Alepha
{
	inline namespace Aluminum
	{
		namespace Mockination
		{
			class MockCondition
			{
				private:
					Alepha::Truss::mutex access;
					Alepha::Truss::condition condition;

				public:
					inline void
					notify_all()
					{
						this->condition.notify_all();
					}

					inline void
					notify_one()
					{
						this->condition.notify_one();
					}

					template< typename UniqueLock >
					inline void
					wait( UniqueLock &lock )
					{
						auto fake_lock= Alepha::use_unique( this->access );
						lock.unlock();
						this->condition.wait( fake_lock );
						lock.lock();
					}

					template< typename UniqueLock, typename Predicate >
					inline void
					wait( UniqueLock &lock, Predicate &&predicate )
					{
						while( !predicate() )
						{
							{
								auto fake_lock= Alepha::use_unique( this->access );
								lock.unlock();
								this->condition.wait( fake_lock );
							}
							lock.lock();
						}
						assert( predicate() );
					}
			};
		}
	}
}
