static_assert( __cplusplus > 2020'00 );

#pragma once

#include "Thread.h"

namespace Alepha::inline Cavorite  ::detail::  gate
{
	inline namespace exports
	{
		/*!
		 * A synchronization primitive to block entry to code in a thread until another thread has performed an action.
		 *
		 * Specifically, a common use case is for a controlling thread to do certain things before the child thread
		 * actually starts doing something.
		 *
		 * @code
		 * std::vector< std::unique_ptr< std::thread > > globalThreads;
		 *
		 * void
		 * example()
		 * {
		 * 	Alepha::Gate gate;
		 *
		 * 	auto threadCode= [&gate]
		 * 	{ 
		 *		gate.enter();
		 *		// Main thread code here.  Might reference `globalThreads`.
		 * 	}
		 *
		 * 	gate.openAfter( [&]
		 *	{
		 *		auto thread= std::make_unique< std::thread >( threadCode );
		 *		globalThreads.push_back( std::move( thread ) );
		 *	} );
		 * }
		 * @endcode 
		 */
		class Gate;
	}

	class exports::Gate
	{
		private:
			std::mutex access;
			condition_variable ready;
			bool opened= false;
			bool started= false;

		public:
			void
			enter()
			{
				std::unique_lock lock{ access };
				while( not opened ) ready.wait( lock );

				started= true;
				ready.notify_one();
			}

			void
			openAfter( Function auto function )
			{
				std::unique_lock lock{ access };
				function();
				opened= true;
				ready.notify_one();
				while( not started ) ready.wait( lock );
			}
	};
}

namespace Alepha::Cavorite::inline exports::inline gate
{
	using namespace detail::gate::exports;
}
