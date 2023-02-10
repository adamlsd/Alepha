#pragma once

#include <Alepha/Alepha.h>

#include <Alepha/Truss/thread_common.h>

#include <mutex>

#include <boost/thread/mutex.hpp>
#include <boost/thread/recursive_mutex.hpp>

namespace Alepha::Hydrogen::Truss
{
	ALEPHA_BOOST_THREAD namespace BoostThread
	{
		using boost::mutex;

		using boost::timed_mutex;
		using boost::recursive_mutex;
		using boost::recursive_timed_mutex;

		using std::lock_guard;
		using boost::unique_lock;

		using boost::defer_lock_t;
		using boost::try_to_lock_t;
		using boost::adopt_lock_t;

		using boost::defer_lock;
		using boost::try_to_lock;
		using boost::adopt_lock;

		using std::once_flag;
		using std::call_once;

		using std::try_lock;
		using std::lock;
	}

	ALEPHA_STD_THREAD namespace StdThread
	{
		using std::mutex;

		using std::timed_mutex;
		using std::recursive_mutex;
		using std::recursive_timed_mutex;

		using std::lock_guard;
		using std::unique_lock;

		using std::defer_lock_t;
		using std::try_to_lock_t;
		using std::adopt_lock_t;

		using std::defer_lock;
		using std::try_to_lock;
		using std::adopt_lock;

		using std::once_flag;
		using std::call_once;

		using std::try_lock;
		using std::lock;
	}
}
