static_assert( __cplusplus > 2020'00 );

#pragma once

#include <Alepha/Alepha.h>

#include <Alepha/Truss/thread_common.h>

#include <condition_variable>

#include <boost/thread/condition_variable.hpp>

namespace Alepha::Hydrogen::Truss
{
	ALEPHA_BOOST_THREAD namespace BoostThread
	{
		using boost::condition_variable_any;

		using condition_variable= condition_variable_any;

		using condition= condition_variable;
	}

	ALEPHA_STD_THREAD namespace StdThread
	{
		using std::condition_variable_any;

		using condition_variable= condition_variable_any;

		using condition= condition_variable;
	}
}
