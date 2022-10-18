static_assert( __cplusplus > 2020'00 );

#pragma once

#include <Alepha/Alepha.h>

#include <Alepha/boost_path/thread.hpp>
#include <Alepha/boost_path/thread/mutex.hpp>
#include <Alepha/boost_path/thread/condition_variable.hpp>

#include <Alepha/Exception.h>

namespace Alepha::Hydrogen
{
	namespace detail::thread
	{
		inline namespace exports {}
		namespace exports
		{
			using CrossThreadNotificationRethrowError= synthetic_exception< struct cross_thread_notification_failure, Error >;
		}

		class NotificationInfo
		{
			private:
				std::mutex access;
				std::exception_ptr notification;

			public:
				//template( Concepts::DerivedFrom< Notification > Exc )
				void
				setNotification( std::exception_ptr &&exception )
				{
					std::lock_guard lock( access );
					notification= std::move( exception );
				}

				template< typename Callable >
				void
				check_interrupt( Callable &&callable )
				try
				{
					callable();
				}
				catch( const boost_ns::thread_interrupted & )
				{
					std::lock_guard lock( access );
					if( not notification ) throw;
					try
					{
						std::rethrow_exception( std::move( notification ) );
					}
					catch( const std::bad_alloc & )
					{
						throw build_exception< CrossThreadNotificationRethrowError >( "`std::bad_alloc` encountered in trying to "
								"raise a cross-thread notification" );
					}
				}
		};
			
		inline thread_local NotificationInfo notification;

		namespace exports
		{
			class ConditionVariable
				: private boost_ns::condition_variable
			{
				public:
					using condition_variable::notify_all;
					using condition_variable::notify_one;

					template< typename Lock >
					void
					wait( Lock &&lock )
					{
						notification.check_interrupt( [&]{ condition_variable::wait( std::forward< Lock >( lock ) ); } );
					}

					template< typename Lock, typename Predicate >
					void
					wait( Lock &&lock, Predicate &&predicate )
					{
						notification.check_interrupt( [&]{ condition_variable::wait( std::forward< Lock >( lock ),
								std::forward< Predicate >( predicate ) ); } );
					}
			};

			namespace this_thread
			{
				template< typename Clock, typename Duration >
				void
				sleep_until( const boost_ns::chrono::time_point< Clock, Duration > &abs_time )
				{
					notification.check_interrupt( [&]{ boost_ns::this_thread::sleep_until( abs_time ); } );
				}
					
#if 0
				template< typename Rep, typename Period >
				void
				sleep_for( const boost_ns::chrono::duration< Rep, Period > &rel_time )
				{
					notification.check_interrupt( [&]( boost_ns::this_thread::sleep_until( rel_time ); } );
				}
#endif
			}
		}

		struct ThreadNotification
		{
			NotificationInfo *myNotification= nullptr;
		};
	
		namespace exports
		{
			class Thread
				: ThreadNotification, boost_ns::thread
			{
				public:
					template< typename Callable >
					explicit
					Thread( Callable &&callable )
						: thread
						(
							[this, callable= std::forward< Callable >( callable )]
							{
								myNotification= &notification;
								try { callable(); }
								catch( const Notification & )
								{
									// Notifications are not fatal.
								}
							}
						)
					{}

					using thread::join;
					using thread::detach;
					
					using thread::interrupt;

					//template( Concepts::DerivedFrom< Notification > Exc )
					template< typename Exc >
					void
					interrupt( Exc &&exception )
					try
					{
						throw std::forward< Exc >( exception );
					}
					catch( const Notification & )
					{
						myNotification->setNotification( std::current_exception() );
						interrupt();
					}
			};

			using Mutex= boost_ns::mutex;
			using boost_ns::mutex;
			using boost_ns::unique_lock;
			using boost_ns::lock_guard;
		}
	}

	inline namespace exports {}
	namespace exports::inline thread
	{
		using namespace detail::thread::exports;
	}
}
