#pragma once

#include <Alepha/Alepha.h>

#include <Alepha/Truss/thread_common.h>

#include <thread>

#include <boost/thread.hpp>

#include <Alepha/Truss/function.h>

namespace Alepha::Hydrogen::Truss
{
	ALEPHA_BOOST_THREAD namespace BoostThread
	{
		// If you decide to use Alepha threading primitives, you'll get the boost ones.
		// Eventually we'd like to add interrupt-with-reason semantics.  That will be in
		// Alepha::Thread.

		// I'd like to map Alepha::Truss's thread to be only std:: thread eventually.
		// There will remain "boost::thread" semantics for Alepha, at present.
		using boost::thread;

		namespace this_thread= boost::this_thread;

		namespace detail_thread
		{
			template< typename Ex >
			auto
			make_thrower( Ex &&exception )
			{
				return [exception]{ throw exception; };
			}
		}
	}

	ALEPHA_STD_THREAD namespace StdThread
	{
		// If you decide to use Alepha threading primitives, you'll get the boost ones.
		// Eventually we'd like to add interrupt-with-reason semantics.  That will be in
		// Alepha::Thread.

		// I'd like to map Alepha::Truss's thread to be only std:: thread eventually.
		// There will remain "boost::thread" semantics for Alepha, at present.
		using ::std::thread;

		namespace this_thread= ::std::this_thread;

		namespace detail_thread
		{
			template< typename Ex >
			auto
			make_thrower( Ex &&exception )
			{
				return [exception]{ throw exception; };
			}
		}
	}

	namespace under_construction
	{
		// Built on top of std::thread
		class thread
		{
			private:
				Alepha::Truss::function< void () > interruption;

				::std::thread thread;
		};
	}
}
