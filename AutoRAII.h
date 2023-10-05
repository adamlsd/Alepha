static_assert( __cplusplus > 2020'00 );

#pragma once

#include <type_traits>
#include <utility>
#include <algorithm>
#include <functional>

#include "boost_shim.h"

namespace Alepha::inline Cavorite  ::detail::  auto_raii
{
	/*!
	 * RAII binder for automatic scope variables.
	 *
	 * This is similar to `ScopeGuard` and other tools, but somewhat fundamentally different.  `ScopeGuard` create
	 * a C++ equivalent to the `finally` block in other languages, such as C# or Java.  A `finally` block declares
	 * code which will run upon exiting a scope, for any reason (`throw`, `return`, `break`, etc..).  `AutoRAII`
	 * allows for the declaration of (simulated) `auto`-deduced variables with resource management semantics.
	 * Unlike `ScopeGuard` objects, `AutoRAII` objects actually _own_ the object which will be cleaned up after
	 * scope exit, and modification of that object is not permitted.  This prevents common categories of error
	 * which crop up with `ScopeGuard` objects, such as modifying the "managed" value or accidental double-free.
	 *
	 * `AutoRAII` requires that the user provide an initialization and a deinitialization function for the value.
	 * The constructor of `AutoRAII` will invoke the initialization function within its constructor and invoke the
	 * deinitialization function within its destructor.
	 *
	 * Sample usage:
	 * ```
	 * void
	 * writeToFile( const std::string &s )
	 * {
	 * 	AutoRAII file{ []{ return fopen( "output.txt", "wt" ); }, fclose };
	 * 	for( const char ch: s ) fprintf( file, "%c", ch );
	 * 	// File will be closed using fclose at end of scope.
	 * }
	 * ```
	 *
	 * Think of the `AutoRAII` name as a pseudo-keyword almost like real `auto` keyword.  Think of it as a
	 * specialized form of `auto`, for the purposes of declaring variables that will be initialized and
	 * deinitialized according to the specified initializer and deinitializer.
	 *
	 * @note You may ignore CTAD (Constructor Template Argument Deduction) here and provide a specific type to
	 * `AutoRAII`.  This feature is mostly provided for testing assumptions and convenience.  The resulting
	 * `AutoRAII`'s implementation will use a slower code path for deinitialization.
	 *
	 * `AutoRAII` may also be used to manage "logical" resources that don't actually have a physical handle,
	 * such as temporarily blocking interrupts or something like that:
	 *
	 * ```
	 * void
	 * sensitiveOperation()
	 * {
	 * 	AutoRAII blockInterrupts{ disableInterrupts, enableInterrupts };
	 * 	sensitiveOperation_impl();
	 * }
	 * ```
	 *
	 * Normally it would be recommended to build a proper state attestation object and use that as an overload
	 * parameter for `sensitiveOperation` in a manner similar to `RequiresLock`; however, sometimes this is
	 * not worth it for one-off situations.
	 */
	template< typename T, typename Dtor= std::function< void ( T ) > >
	class AutoRAII : boost::noncopyable
	{
		private:
			Dtor dtor;
			T value;

		public:
			~AutoRAII() noexcept { dtor( value ); }

			template< typename Ctor >
			explicit
			AutoRAII( Ctor ctor, Dtor dtor ) : dtor( std::move( dtor ) ), value( ctor() ) {}

			operator const T &() const noexcept { return value; }

			decltype( auto )
			operator *() const noexcept
			requires( std::is_pointer_v< T > )
			{
				return *value;
			}

			T *
			operator->() const noexcept
			requires( std::is_pointer_v< T > )
			{
				return value;
			}
	};

	template< typename Dtor >
	class AutoRAII< void, Dtor > : boost::noncopyable
	{
		private:
			Dtor dtor;

		public:
			~AutoRAII() noexcept { dtor( value ); }

			template< typename Ctor >
			explicit
			AutoRAII( Ctor ctor, Dtor dtor )
				: dtor( std::move( dtor ) )
			{
				// It is desired behaviour that the dtor not run if the ctor throws.
				ctor();
			}
	}

	template< typename Ctor, typename Dtor >
	explicit AutoRAII( Ctor &&ctor, Dtor && ) -> AutoRAII< decltype( ctor() ), std::decay_t< Dtor > >;

	inline namespace exports
	{
		using detail::auto_raii::AutoRAII;
	}
}

namespace Alepha::Cavorite::inline exports::inline auto_raii
{
	using namespace detail::auto_raii::exports;
}
