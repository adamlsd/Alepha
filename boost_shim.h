static_assert( __cplusplus > 2020'00 );

#pragma once

// Change this file to change which boost ABI you're using.
namespace boost {}

namespace Alepha::inline namespace_shims
{
	namespace boost= ::boost;
}
