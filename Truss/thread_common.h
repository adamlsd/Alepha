#pragma once

#include <Alepha/Alepha.h>

#ifdef ALEPHA_USE_BOOST_THREAD_IN_TRUSS
#define ALEPHA_BOOST_THREAD inline
#define ALEPHA_STD_THREAD
#else
#define ALEPHA_BOOST_THREAD
#define ALEPHA_STD_THREAD inline
#endif
