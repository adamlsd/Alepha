static_assert( __cplusplus > 2020'00 );

#pragma once

#include <tuple>

#include <boost/noncoyable.hpp>

namespace Alepha::inline Cavorite  ::detail::  static_value
{
	inline namespace exports
	{
		template< typename T, typename init_helper >
		class StaticValue;
	}

	template< typename T >
	struct default_init
	{
		constexpr decltype( auto )
		operator() () const noexcept
		{
			return new T{};
		}
	};

	/*!
	 * Static value with low access overhead.
	 *
	 * In C++, function-static values in functions can have significant overhead to access, because they use an
	 * atomic check to guarantee threadsafe initialization.  Alternatively, namespace-scope static values have their
	 * own initialization problems, because of the initialization order rules in C++.  `Alepha::StaticValue` is a
	 * mechanism to provide a halfway-between solution.  `Alepha::StaticValue` objects should be defined at
	 * namespace-scope.  Unlike normal namespace-scope variables, they are initialized at first access.  Unlike
	 * function-static values, the initialization is not guarded by an atomic access.
	 *
	 * `StaticValue` objects solve the initialization order issues of normal namespace-statics because it will
	 * initialize on first use.  They solve the performance issues of function-statics, because of the
	 * non-atomic initialization technique.  This means that the most appropriate use pattern is to guarantee
	 * initialization during a single-threaded portion of program execution.  The idiom is meant to be
	 * a drop-in replacement (in usage syntax) for a function which returns a static value.  However, it now has
	 * its own set of limitations -- it is not possible to initialize in a threadsafe manner.
	 *
	 * Example usage:
	 *
	 * ```
	 * namespace MyComponent
	 * {
	 *     StaticValue< std::string > globalMessage;
	 *
	 *     auto initGlobalMessage= globalMessage.init();
	 *
	 *     void
	 *     printGlobalMessage()
	 *     {
	 *         std::cout << globalMessage() << std::endl;
	 *     }
	 * }
	 * ```
	 *
	 * If `printGlobalMessage` is called from any static initializer, before `initGlobalMessage` is initialized,
	 * then that first use will initialize the value.  If `initGlobalMessage` is initialized before any usage
	 * of `globalMessage`, then the value will be initialized by that initializer.
	 *
	 * For any program which does not start any threads before `main()` starts, this pattern (declare a
	 * `StaticValue` and declare an initializer step) will always be threadsafe and proper.  For a program
	 * which can guarantee that a `StaticValue` is used in static initializers before `main()` or threads start,
	 * it is possible to decline making a static initializer call to `StaticValue::init`.
	 *
	 * Exploring this further, here's a typical function-static used with initializers:
	 * ```
	 * namespace MyComponent
	 * {
	 *     auto &
	 *     registry()
	 *     {
	 *         static std::vector< std::string > registrations;
	 *         return registrations;
	 *     }
	 *
	 *     auto registration= Alepha::enroll<=[]
	 *     {
	 *         registry().push_back( "Hello World" );
	 *     };
	 * }
	 * ```
	 * The above could be rewritten to have the effects of `StaticValue` thus:
	 * ```
	 * namespace MyComponent
	 * {
	 *     namespace storage
	 *     {
	 *         std::vector< std::string > *registrations= nullptr;
	 *     }
	 *
	 *     auto &
	 *     registry()
	 *     {
	 *         using namespace storage;
	 *         if( registrations == nullptr ) registrations= new decltype( *registrations );
	 *         return *registrations;
	 *     }
	 *
	 *     auto registration= Alepha::enroll<=[]
	 *     {
	 *         registry().push_back( "Hello World" );
	 *     };
	 * }
	 * ```
	 * However, that is complicated and difficult to get right.  Thus `StaticValue` helps
	 * us by radically simplifying the declaration syntax:
	 * ```
	 * namespace MyComponent
	 * {
	 *     StaticValue< std::vector< std::string > > registrations;
	 *
	 *     auto registration= Alepha::enroll<=[]
	 *     {
	 *         registrations().push_back( "Hello World" );
	 *     };
	 * }
	 * ```
	 */
	template< typename T, typename init_helper= default_init >
	class exports::StaticValue : boost::noncopyable
	{
		private:
			T *storage= nullptr;

		public:
			~StaticValue()
			{
				delete storage;
			}

			constexpr decltype( auto )
			get() noexcept
			{
				if( storage == nullptr ) [[unlikely]] storage= init_helper{}();
				assert( storage != nullptr );
				return *storage;
			}

			constexpr auto
			operator() () noexcept
			{
				return get();
			}
	};
}

namespace Alepha::Cavorite::inline exports::inline static_value
{
	using namespace detail::static_value::exports;
}
