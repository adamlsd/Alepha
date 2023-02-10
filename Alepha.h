static_assert( __cplusplus > 2020'00 );

#pragma once

#ifndef ALEPHA_OVERRIDE_BOOST_NAMESPACE
namespace boost {}
#else
namespace ALEPHA_OVERRIDE_BOOST_NAMESPACE {}
#endif

namespace Alepha::inline Hydrogen
{
	#ifndef ALEPHA_OVERRIDE_BOOST_NAMESPACE
	namespace boost_ns= ::boost;
	#else
	namespace boost_ns= ALEPHA_OVERRIDE_BOOST_NAMESPACE;
	#endif
}
