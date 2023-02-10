#pragma once

#include <Alepha/Alepha.h>

#include <cassert>

#include <iostream>

#include <map>
#include <vector>
#include <utility>
#include <iterator>
#include <algorithm>

#include <Alepha/Truss/mutex.h>
#include <Alepha/Truss/thread.h>
#include <Alepha/Truss/condition_variable.h>

namespace Alepha
{
	inline namespace Aluminum
	{
		namespace Mockination
		{
			class MockMutexImpl
			{
				private: 
					using thread_type= Alepha::Truss::thread::id;
					using mutex_type= Alepha::Truss::mutex;
					using lock_type= Alepha::Truss::unique_lock< mutex_type >;
					using condition_type= Alepha::Truss::condition_variable;

					static inline thread_type
					get_this_thread()
					{
						return Alepha::Truss::this_thread::get_id();
					}

					class Waiter
					{
						private:
							MockMutexImpl *const this_;
							condition_type condition;

							inline Waiter( const Waiter & )= delete;
							inline Waiter &operator= ( const Waiter & )= delete;

						public:
							inline
							~Waiter()
							{
								this->this_->waiters.erase( get_this_thread() );
							}

							explicit inline
							Waiter( MockMutexImpl *const i_t, const lock_type & )
								: this_( i_t )
							{
								this->this_->waiters[ get_this_thread() ]= this;
							}

							void
							wait( lock_type &lock )
							{
								this->condition.wait( lock );
							}

							void
							unblock( lock_type &lock )
							{
								this->condition.notify_all();
							}
					};

					mutable mutex_type internal_mutex;
					mutable mutex_type access;
					mutable std::map< thread_type, Waiter * > waiters;
					mutable condition_type lockReleased;

					mutable bool unlockWaiterReady= false;
					mutable condition_type lockWaited;

					mutable condition_type lockEntered;

					mutable condition_type waiterAvailable;

					thread_type holder_;
					std::exception_ptr interruption;

				private: // Internal impls, unlocked
					inline lock_type
					lockAccess() const
					{
						return lock_type{ this->access };
					}

					inline bool
					locked( const lock_type & ) const
					{
						return this->holder_ != thread_type{};
					}

					inline thread_type
					holder( const lock_type & ) const
					{
						return this->holder_;
					}

					inline void
					waitLocked( lock_type &lock ) const
					{
						this->lockEntered.wait( lock, [this, &lock] { return this->locked( lock ); } );
					}

					inline void waitLocked( lock_type &&lock ) const { this->waitLocked( lock ); }

					inline void
					waitUnlocked( lock_type &lock ) const
					{
						this->unlockWaiterReady= true;
						this->lockWaited.notify_all();
						this->lockReleased.wait( lock, [this, &lock]{ return !this->locked( lock ); } );
					}

					inline void waitUnlocked( lock_type &&lock ) const { this->waitUnlocked( lock ); }

					void
					allowFirst( lock_type &lock )
					{
						assert( !this->waiters.empty() );
						this->waiters.begin()->second->unblock( lock );
					}

					void allowFirst( lock_type &&lock ) { this->allowFirst( lock ); }

					void
					allow( const thread_type next, lock_type &lock )
					{
						assert( !this->waiters.empty() );
						const auto found= this->waiters.find( next );
						assert( found != this->waiters.end() );
						found->second->unblock( lock );
					}

					void
					allow( const thread_type next, lock_type &&lock )
					{
						this->allow( next, lock );
					}
					
				public:
					inline ~MockMutexImpl()= default;

					explicit inline MockMutexImpl( const MockMutexImpl & )= delete;
					inline MockMutexImpl &operator= ( const MockMutexImpl & )= delete;

					explicit inline MockMutexImpl( MockMutexImpl && )= delete;
					inline MockMutexImpl &operator= ( MockMutexImpl && )= delete;

					explicit inline MockMutexImpl()= default;

					// TODO: Consider breaking this part of the API out into a management handle
					// object that can be used to avoid having client threads work with the management
					// interface

					/*!
					 * @brief Returns an observation of the waiting state.
					 * @returns A list of the number of threads
					 * @note That the returned value may become out of date, as soon as consumed.
					 */
					inline std::vector< thread_type >
					getWaiters() const
					{
						lock_type lock( this->access );
						std::vector< thread_type > rv;
						rv.reserve( this->waiters.size() );
						std::transform( begin( this->waiters ), end( this->waiters ),
								back_inserter( rv ), []( const auto &w ) { return w.first; } );
						return rv;
					}

					inline void
					waitForWaiter( const thread_type waiter ) const
					{ 
						lock_type lock( this->access );
						waiterAvailable.wait( lock, [this, waiter]
								{ return this->waiters.find( waiter ) != end( this->waiters ); } );
					}

					/*!
					 * @brief Returns an observation of the waiting state.
					 * @returns The number of threads waiting for entry to the lock.
					 * @note That the returned value may become out of date, as soon as consumed.
					 */
					inline bool
					hasWaiters() const
					{
						lock_type lock( this->access );
						return !this->waiters.empty();
					}

					/*!
					 * @brief Returns an observation of the locked-state.
					 * @returns True if the lock is in the locked state.
					 * @returns False if the lock is not in the locked state.
					 */
					inline bool locked() const { return this->locked( lock_type{ this->access } ); }

					/*!
					 * @brief Returns an observation of the current holder of the lock.
					 * @returns The `Alepha::Truss::thread::id` of the thread holding the lock.
					 * @note A default constructed thread-id indicates no current holder.
					 */
					inline thread_type holder() const { return this->holder( this->lockAccess() ); }

					/*!
					 * @brief Blocks the caller until the lock transitions to the locked state.
					 * @pre The lock has been previously pumped with a request to transition a
					 *      waiter into the lock-held state.
					 * @post The most recently requested waiter to enter the lock will transition into
					 *       the lock-held state.
					 */
					void waitLocked() const { this->waitLocked( this->lockAccess() ); }

					/*!
					 * @brief Blocks the caller until the lock transitions to the unlocked state.
					 * @pre The lock was previously pumped with a request to transition a
					 *      waiter into the lock-held state. (The lock is in the locked or
					 *      indeterminate state.)
					 * @post The most recently requested waiter will have completed its critical
					 *       section.
					 */
					void waitUnlocked() const { this->waitUnlocked( this->lockAccess() ); }

					/*!
					 * @brief Pumps the first waiter in the internal wait list to transition to the
					 *        locked state.
					 * @pre The lock is in the unlocked state.
					 * @post The lock is in an indeterminate state.
					 * @note A call to `waitUnlocked` must be made before a subsequent call to
					 *       `allowFirst` or `allow` can be made -- otherwise the lock is in an
					 *       indeterminate state.
					 * @note The "first waiter" is an unspecified waiting thread, and not necessarily
					 *       the longest waiting thread -- threads are not pumped in "fifo" order by
					 *       this operation.
					 */
					void allowFirst() { this->allowFirst( this->lockAccess() ); }

					/*!
					 * @brief Pumps the first waiter in the internal wait list to transition to the
					 *        locked state.
					 * @pre The lock is in the unlocked state.
					 * @invariant A single thread will enter the lock, and then release it.
					 * @post The lock has returned to the unlocked state, after a single thread entered.
					 * @note `allowFirstWait` can be called multiple times in succession without
					 *       intervening calls.
					 * @note The "first waiter" is an unspecified waiting thread, and not necessarily
					 *       the longest waiting thread -- threads are not pumped in "fifo" order by
					 *       this operation.
					 */
					void
					allowFirstWait()
					{
						lock_type lock( this->access );
						this->allowFirst( lock );
						this->waitLocked( lock );
						this->waitUnlocked( lock );
					}

					/*!
					 * @brief Pumps the specified waiter in the internal wait list to transition to the
					 *        locked state.
					 * @param next The thread to permit into the lock next.
					 * @pre The lock is in the unlocked state.
					 * @post The lock is in an indeterminate state.
					 * @note A call to `waitUnlocked` must be made before a subsequent call to
					 *       `allowFirst` or `allow` can be made -- otherwise the lock is in an
					 *       indeterminate state.
					 */
					void
					allow( const thread_type next )
					{
						this->allow( next, lock_type{ this->access } );
					}

					/*!
					 * @brief Pumps the specified waiter in the internal wait list to transition to the
					 *        locked state.
					 * @param next The thread to permit into the lock next.
					 * @pre The lock is in the unlocked state.
					 * @invariant A single, specified thread will enter the lock, and then release it.
					 * @post The lock has returned to the unlocked state, after the specified thread
					 *       entered.
					 * @note `allowWait` can be called multiple times in succession without
					 *       intervening calls.
					 */
					void
					allowWait( const thread_type next )
					{
						lock_type lock( this->access );
						this->allow( next, lock );
						this->waitLocked( lock );
						assert( this->holder( lock ) == next );
						this->waitUnlocked( lock );
					}

#if 0
					template< typename Exception >
					void
					interruptFirst( Exception exc )
					{
						lock_type lock( this->access );
						assert( !this->waiters.empty() );
						this->interruption= std::make_exception_ptr( std::move( exc ) );
						this->waiters.begin()->second->unblock( lock );
					}

					template< typename Exception >
					void
					interrupt( const thread_type next, Exception exc )
					{
						lock_type lock( this->access );
						const auto found= this->waiters.find( next );
						assert( found != this->waiters.end() );
						this->interruption= std::make_exception_ptr( std::move( exc ) );
						found->second->unblock( lock );
					}
#endif

					// Mutex interface:

					/*!
					 * @brief Attempt to assert the lock.
					 * @pre  The current thread does NOT hold the lock.
					 * @post The lock is in the locked state and `Alepha::Truss::this_thread::get_id()`
					 *       is registered as the current holder of this thread.
					 */
					inline void
					lock()
					{
						lock_type lock( this->access );
						assert( this->holder_ != get_this_thread() );
						// Waiter needs to stick around until exiting this function -- it indicates
						// an actual waiting state.  Exiting this function removes that state.
						Waiter waiter( this, lock );

						waiterAvailable.notify_all(); // unblock anyone waiting for new waiters.
						waiter.wait( lock );
						if( this->interruption )
						{
							auto interrupt= this->interruption;
							this->interruption= nullptr;
							std::rethrow_exception( interrupt );
						}
						this->holder_= get_this_thread();
						this->lockEntered.notify_all();
						this->internal_mutex.lock();
					}

					/*!
					 * @brief Release control over this lock.
					 * @pre  The current thread holds the lock.
					 * @post The lock transitions to the unlocked state and this thread is deregistered
					 *       as the current holder of this thread.
					 *
					 * @note Threads do not actually release the lock until permitted to do so by
					 *       a `waitUnlocked` operation from an external thread, or if the thread
					 *       entered this lock by means of an `allowWait` or `allowFirstWait`
					 *       operation.
					 */
					// Unlocking is techincally a thread synchronization point, but we will
					// crash the program at present, since it gets called from destructors.
					// TODO: Use a universal interrupt block here.
					inline void
					unlock() noexcept
					{
						lock_type lock( this->access );
						assert( this->holder_ == get_this_thread() );
						this->internal_mutex.unlock();
						while( !this->unlockWaiterReady )
						{
							this->lockWaited.wait( lock );
						}
						this->unlockWaiterReady= false;
						this->holder_= {};
						this->lockReleased.notify_all();
					}
			};

			template< int id >
			class MockMutex
			{
				public:
					static MockMutexImpl impl;

					inline void
					unlock() noexcept
					{
						impl.unlock();
					}

					inline void
					lock() noexcept
					{
						impl.lock();
					}
			};

			template< int id > MockMutexImpl MockMutex< id >::impl;
		}
	}
}
