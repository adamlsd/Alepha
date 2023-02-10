#include <Alepha/Atomic/Turnstile.h>
using Alepha::Atomic::Turnstile;

int
main()
{
}

namespace
{
namespace example1
{
//! [TurnstileExamples example1]
// Assume that the below code will run multithreaded
Turnstile myArena{ 8 };
		 
void
mainLoop()
{
	Turnstile::ScopedUsage active( myArena );
	printf( "I am running in the arena, not everyone can." );
	sleep( 100 );
}
//! [TurnstileExamples example1]
} // namespace example1


namespace example2
{
//! [TurnstileExamples example2]
Turnstile myArena{ 8 };

class Worker
{
	public:
		Worker()
		{
			myArena.enter();
		}

		// Assume some useful functionality

		// The worker will destroy his access upon leaving
		~Worker()
		{
			myArena.egress();
		}
};
//! [TurnstileExamples example2]
} // namespace example2
} // namespace
