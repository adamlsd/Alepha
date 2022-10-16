static_assert( __cplusplus > 2020'00 );

#pragma once

#include <Alepha/Alepha.h>

#include <cstdlib>

#include <string>
#include <exception>
#include <string_view>
#include <typeindex>

namespace Alepha::Hydrogen
{
	namespace detail::exceptions::inline exports
	{
		/*!
		 * @file
		 *
		 * Exception grades.
		 *
		 * In Alepha, there are several "grades" of exception types which can be thrown.  The grade of
		 * an exception is "branded" into its name as the last word in camel case.
		 *
		 *  * `Throwable`: All exceptions inherit from this interface.  Catching this will
		 *    catch anything which is part of this system.  Normally you just ignore this grade.
		 *
		 *  * `Notification`: When a thread is cancelled using interrupts, all exceptions thrown
		 *    to do so are derived from this type.  Alepha threads are setup to catch and discard
		 *    exceptions of this grade in the thread start function.
		 *
		 *  * `Exception`: This is the exception grade you would typically want to recover from.
		 *    Catch this grade, typically.  They should contain sufficient information in their
		 *    data pack to facilitate a proper programmatic recovery.  All exceptions of this grade
		 *    will also have `std::exception` as one of its bases, thus code which is unaware of
		 *    Alepha exceptions, but handles basic standard exceptions cleanly, will work just fine.
		 *
		 *  * `Error`: This exception grade represents a form of moderately-unrecoverable condition.
		 *    The `Error` grade typically indicates a condition that prevents the current thread or
		 *    program state from being easily transitioned to a recovered state.  However, a very
		 *    large state transform, such as a top-of-thread-callstack handler, may be able to
		 *    recover.  For example, no more available operating system file handles.  Tearing down
		 *    several client handlers in a multi-client server might alleviate this condition.
		 *
		 *  * `Violation`: This exception grade represents an impossible to recover from condition.
		 *    Think of the handler for `Violation` as-if it were a `[[noreturn]]` function.  Catch
		 *    handlers for this grade should only clean up resources which might cause persistent
		 *    state corruption.  A program probably SHOULD ignore these and allow a core dump to
		 *    be emitted.  An example of this might be: Runtime detection of dereference of a
		 *    raw pointer set to `nullptr`.  This would typically indicate some kind of program
		 *    bug.  `Violation::~Violation` is setup to call `abort()` when called.
		 */

		class Throwable
		{
			public:
				virtual ~Throwable()= default;
				virtual const char *message() const noexcept= 0;

				template< typename Target >
				const Target &
				as() const
				{
					if( not is_a< const Target * >() )
					{
						// TODO: Structured exception recovery here...
					}

					return dynamic_cast< const Target & >( *this );
				}

				template< typename Target >
				bool
				is_a() const noexcept
				{
					return dynamic_cast< const Target * >( this );
				}
		};

		class AnyTaggedThrowable
			: virtual public Throwable
		{
			public:
				virtual std::type_index tag() const noexcept= 0;
		};

		template< typename Tag >
		class TaggedThrowable
			: virtual public AnyTaggedThrowable
		{
			public:
				std::type_index
				tag() const noexcept final
				{
					return typeid( std::type_identity< Tag > );
				}
		};

		template< typename ... Bases >
		struct bases : virtual public Bases... {};

		// `Notification`s are "events" or "interrupts" that
		// if ignored will gracefully terminate the current
		// thread, but not halt the program.
		class Notification : public virtual Throwable {};
		class AnyTaggedNotification
			: public virtual bases< Notification, AnyTaggedThrowable > {};
		template< typename tag >
		class TaggedNotification
			: public virtual bases< TaggedThrowable< tag >, AnyTaggedNotification > {};

		using Interrupt= Notification;
		using AnyTaggedInterrupt= AnyTaggedNotification;
		template< typename tag >
		using TaggedInterrupt= TaggedNotification< tag >;

		// `Exception`s are recoverable at any point.
		class ExceptionBridgeInterface
		{
			public:
				virtual ~ExceptionBridgeInterface()= default;

				virtual const char *what() const noexcept= 0;
		};
		class Exception : virtual public bases< Throwable >, virtual private ExceptionBridgeInterface { public: using ExceptionBridgeInterface::what; };
		class AnyTaggedException : virtual public bases< Exception, AnyTaggedThrowable > {};
		template< typename tag >
		class TaggedException : virtual public bases< TaggedThrowable< tag >, AnyTaggedException > {};

		// `Error`s are only really recoverable by terminating
		// the major procedure underway.  Like terminating the
		// entire thread or similar.  Essentially, arbitrarily
		// localized recovery is impossible.
		class Error: virtual public bases< Throwable > {};
		class AnyTaggedError : virtual public bases< Error, AnyTaggedThrowable > {};
		template< typename tag >
		class TaggedError : virtual bases< AnyTaggedError, TaggedThrowable< tag > > {};

		// `Violation`s are unrecoverable events which happen to
		// the process.  They are impossible to recover from.
		// Handlers for this should be treated almost like
		// `[[noreturn]]` functions.  Mostly one would catch
		// this class if they intended to perform a bit of local
		// persistent state sanitization and then continue the
		// unwind process
		class Violation
			: virtual public bases< Throwable >
		{
			private:
				bool active= true;

			public:
				~Violation() override { if( not active ) abort(); }

				Violation( const Violation &copy )= delete;
				Violation( Violation &copy ) : active( copy.active ) { copy.active= false; }
		};
		class AnyTaggedViolation : virtual public bases< Violation, AnyTaggedThrowable > {};
		template< typename tag >
		class TaggedViolation : virtual public bases< AnyTaggedViolation, TaggedThrowable< tag > > {};

		template< typename T >
		concept DerivedFromException= std::is_base_of_v< Exception, T >;

		class NamedResourceStorage;
		class NamedResourceInterface
		{
			public:
				using storage_type= NamedResourceStorage;

				virtual ~NamedResourceInterface()= default;
				virtual std::string_view resourceName() const noexcept= 0;
		};
		class NamedResourceStorage
			: virtual public NamedResourceInterface
		{
			private:
				std::string storage;

			public:
				std::string_view resourceName() const noexcept final { return storage; }
		};
		class NamedResourceThrowable : virtual public bases< Throwable, NamedResourceInterface > {};
		class AnyTaggedNamedResourceThrowable
			: virtual public bases< NamedResourceThrowable, AnyTaggedThrowable > {};
		template< typename tag >
		class TaggedNamedResourceThrowable
			: virtual public bases< AnyTaggedNamedResourceThrowable, TaggedThrowable< tag > > {};

		class NamedResourceNotification
			: public virtual bases< Notification, NamedResourceThrowable > {};
		class AnyTaggedNamedResourceNotification
			: public virtual bases< NamedResourceNotification, AnyTaggedNotification, AnyTaggedNamedResourceThrowable > {};
		template< typename tag >
		class TaggedNamedResourceNotification
			: public virtual bases< AnyTaggedNamedResourceNotification, TaggedNotification< tag >, TaggedNamedResourceThrowable< tag > > {};

		class NamedResourceException
			: public virtual bases< Exception, NamedResourceThrowable > {};
		class AnyTaggedNamedResourceException
			: public virtual bases< NamedResourceException, AnyTaggedException, AnyTaggedNamedResourceThrowable > {};
		template< typename tag >
		class TaggedNamedResourceException
			: public virtual bases< AnyTaggedNamedResourceException, TaggedException< tag >, TaggedNamedResourceThrowable< tag > > {};

		class NamedResourceError
			: public virtual bases< Error, NamedResourceThrowable > {};
		class AnyTaggedNamedResourceError
			: public virtual bases< NamedResourceError, AnyTaggedError, AnyTaggedNamedResourceThrowable > {};
		template< typename tag >
		class TaggedNamedResourceError
			: public virtual bases< AnyTaggedNamedResourceError, TaggedError< tag >, TaggedNamedResourceThrowable< tag > > {};

		class NamedResourceViolation
			: public virtual bases< Violation, NamedResourceThrowable > {};
		class AnyTaggedNamedResourceViolation
			: public virtual bases< NamedResourceViolation, AnyTaggedViolation, AnyTaggedNamedResourceThrowable > {};
		template< typename tag >
		class TaggedNamedResourceViolation
			: public virtual bases< AnyTaggedNamedResourceViolation, TaggedViolation< tag >, TaggedNamedResourceThrowable< tag > > {};


		class AllocationAmountStorage;
		class AllocationAmountInterface
		{
			public:
				using storage_type= AllocationAmountStorage;
				virtual ~AllocationAmountInterface()= default;
				virtual std::size_t allocationAmount() const noexcept= 0;
		};
		class AllocationAmountStorage
			: virtual public AllocationAmountInterface
		{
			private:
				std::size_t amount;

			public:
				std::size_t allocationAmount() const noexcept final { return amount; }
		};
		class AllocationThrowable
			: public virtual bases< Throwable, AllocationAmountInterface > {};
		class AnyTaggedAllocationThrowable
			: public virtual bases< AllocationThrowable, AnyTaggedThrowable > {};
		template< typename tag >
		class TaggedAllocationThrowable
			: public virtual bases< AnyTaggedAllocationThrowable, TaggedThrowable< tag > > {};

		class AllocationException
			: virtual public bases< Exception, AllocationThrowable > {};
		class AnyTaggedAllocationException
			: virtual public bases< AllocationException, AnyTaggedAllocationThrowable, AnyTaggedException > {};
		template< typename tag >
		class TaggedAllocationException
			: virtual public bases< AnyTaggedAllocationException, TaggedAllocationThrowable< tag >, TaggedException< tag > > {};

		class AllocationError
			: virtual public bases< Error, AllocationThrowable > {};
		class AnyTaggedAllocationError
			: virtual public bases< AllocationError, AnyTaggedAllocationThrowable, AnyTaggedError > {};
		template< typename tag >
		class TaggedAllocationError
			: virtual public bases< AnyTaggedAllocationError, TaggedAllocationThrowable< tag >, TaggedError< tag > > {};

		class AllocationViolation
			: virtual public bases< Violation, AllocationThrowable > {};
		class AnyTaggedAllocationViolation
			: virtual public bases< AllocationViolation, AnyTaggedAllocationThrowable, AnyTaggedViolation > {};
		template< typename tag >
		class TaggedAllocationViolation
			: virtual public bases< AnyTaggedAllocationViolation, TaggedAllocationThrowable< tag >, TaggedViolation< tag > > {};

		class MessageStorage
			: virtual public Throwable
		{
			private:
				std::string storage;

			public:
				explicit MessageStorage( std::string storage ) : storage( std::move( storage ) ) {}

				const char *message() const noexcept { return storage.c_str(); }
		};

		class AllocationExceptionBridge
			: virtual public std::bad_alloc, public virtual ExceptionBridgeInterface, virtual public Exception
		{
			public:
				const char *what() const noexcept override { return message(); }
		};

		class RegularExceptionBridge
			: virtual public std::exception, public virtual ExceptionBridgeInterface, virtual public Exception
		{
			public:
				const char *what() const noexcept override { return message(); }
		};

		template< typename Kind >
		auto
		build_exception( std::string message )
		{
			if constexpr( false ) {}
			else if constexpr( std::is_base_of_v< AllocationException, Kind > )
			{
				class Exception
					: virtual public Kind, virtual private AllocationExceptionBridge, virtual private MessageStorage, virtual private AllocationAmountStorage
				{
					public:
						explicit Exception( std::string message ) : MessageStorage( std::move( message ) ) {}
				};

				return Exception{ std::move( message ) };
			}
			else if constexpr( std::is_base_of_v< Exception, Kind > )
			{
				class Exception
					: virtual public Kind, virtual private RegularExceptionBridge, virtual private MessageStorage
				{
					public:
						explicit Exception( std::string message ) : MessageStorage( std::move( message ) ) {}
				};

				return Exception{ std::move( message ) };
			}
			else if constexpr( true )
			{
				class Thrown
					: virtual public Kind, virtual private MessageStorage
				{
					explicit Thrown( std::string message ) : MessageStorage( std::move( message ) ) {}
				};

				return Thrown{ std::move( message ) };
			}
		}
	}

	inline namespace exports {}
	namespace exports::inline exceptions
	{
		using namespace detail::exceptions::exports;
	}
}
