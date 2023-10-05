static_assert( __cplusplus > 2020'00 );

#pragma once

#include <iostream>
#include <mutex>

namespace Alepha::inline Cavorite  ::detail::  error_stream
{
	inline namespace exports {}

	struct SecretMutex : std::recursive_mutex {};

	namespace exports
	{
		/*!
		 * Returns a locked reference to the global debug error stream.
		 *
		 * The stream returned by this function may not be self-flushing, so `std::endl` should be
		 * used at the end of debug-printing lines.  This function causes (through some C++ magic)
		 * a global mutex lock to be held for the duration of the "line" wherein it is executed.
		 * This lock prevents multiple debug streams from interlacing with each other, when
		 * streaming various data together.  For instance:
		 *
		 * ```
		 * Alepha::error() << "The number of calls was: " << count << std::endl;
		 * ```
		 *
		 * Under normal circumstances, `std::cerr` (or other streams) could cause interlacing of
		 * output among threads to show up between the string and the `count` value being printed.
		 * The lock held (until the semicolon) means that such print lines will appear atomically,
		 * in output.  This can help in reading debug messages in threaded programs.
		 *
		 * @note The lock parameter should NOT ever be passed by the caller.  Instead, AAAA
		 * (ADAM's Anonymous Argument Allocation) is responsible for taking the lock.
		 *
		 * @note The resulting stream is just a normal `std::ostream`, the lock is only held for
		 * the duration of the temporary constructed in this call.  Capturing that stream to a
		 * local named variable is going to cause the lock to be released.  While this is not
		 * a fatal problem, it can result in torn error lines, once again.
		 */
		std::ostream &error( const std::lock_guard< SecretMutex > & );

		// Not threadsafe.  Set in or before main, before starting any threads.
		void setErrorStream( std::ostream &os );
	}

	inline SecretMutex access;

	// The default is `std::cerr`, but any `std::ostream` will do.
	inline std::ostream *errorStream= &std::cerr;

	inline std::ostream &
	exports::error( const std::lock_guard< SecretMutex > & = std::lock_guard{ access } )
	{
		return errorStream;
	}
}

namespace Alepha::Cavorite::inline exports::inline error_stream
{
	using namespace detail::error_stream::exports;
}
