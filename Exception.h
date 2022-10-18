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
		 * Exception grades -- Exceptions have different grades of severity.
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

		template< typename ... Bases >
		struct bases : virtual public Bases... {};

		template< typename unique, typename GradeType, typename Bases >
		class synthetic_any_tagged_type
		{
		};

		template< typename unique_handle, typename GradeType, typename ... Bases >
		class synthetic_exception
			: virtual public bases< GradeType, Bases... >
		{
			public:
				using grade_type= GradeType;

				class any_tagged_type
					: virtual public bases< synthetic_exception, typename GradeType::any_tagged_type, typename Bases::any_tagged_type... >
				{
					public:
						using grade_type= GradeType;
				};

				template< typename tag >
				class tagged_type
					: virtual public bases< synthetic_exception, any_tagged_type, typename GradeType::template tagged_type< tag >, typename Bases::template tagged_type< tag >... >
				{
					public:
						using grade_type= GradeType;
				};
		};

		template< typename unique_handle, typename GradeType, typename ... Bases >
		using create_exception= synthetic_exception< unique_handle, GradeType, Bases ... >;

		template< typename Exc >
		using AnyTagged= typename Exc::any_tagged_type;

		template< typename Exc, typename tag >
		using Tagged= typename Exc::template tagged_type< tag >;

		class Throwable
		{
			public:
				using grade_type= Throwable;

				class any_tagged_type;
				template< typename Tag > class tagged_type;

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
		class Throwable::any_tagged_type
			: virtual public grade_type
		{
			public:
				virtual std::type_index tag() const noexcept= 0;
		};
		template< typename Tag >
		class Throwable::tagged_type
			: virtual public grade_type, virtual public grade_type::any_tagged_type
		{
			public:
				std::type_index
				tag() const noexcept final
				{
					return typeid( std::type_identity< Tag > );
				}
		};
		using AnyTaggedThrowable= AnyTagged< Throwable >;
		template< typename tag >
		using TaggedThrowable= Tagged< Throwable, tag >;


		// `Notification`s are "events" or "interrupts" that
		// if ignored will gracefully terminate the current
		// thread, but not halt the program.
		class Notification
			: public virtual Throwable
		{
			public:
				using grade_type= Notification;

				class any_tagged_type;
				template< typename tag >
				class tagged_type;
		};
		class Notification::any_tagged_type
			: public virtual bases< grade_type, Throwable::any_tagged_type > {};
		template< typename tag >
		class Notification::tagged_type
			: public virtual bases< grade_type::any_tagged_type, Throwable::tagged_type< tag > > {};
		using AnyTaggedNotification= Notification::any_tagged_type;
		template< typename tag >
		using TaggedNotification= Notification::tagged_type< tag >;

		using Interrupt= Notification;
		using AnyTaggedInterrupt= AnyTaggedNotification;
		template< typename tag >
		using TaggedInterrupt= TaggedNotification< tag >;

		// `Exception`s are recoverable at any point.
		class ExceptionBridgeInterface
		{
			public:
				virtual const char *what() const noexcept= 0;
		};
		class Exception
			: virtual public bases< Throwable >, virtual private ExceptionBridgeInterface
		{
			public:
				using grade_type= Exception;
				using ExceptionBridgeInterface::what;

				class any_tagged_type;
				template< typename tag >
				class tagged_type;
		};
		class Exception::any_tagged_type : virtual public bases< grade_type, Throwable::any_tagged_type > {};
		template< typename tag >
		class Exception::tagged_type
			: virtual public bases< grade_type::any_tagged_type, Throwable::tagged_type< tag > > {};
		using AnyTaggedException= Exception::any_tagged_type;
		template< typename tag >
		using TaggedException= Exception::tagged_type< tag >;

		// `Error`s are only really recoverable by terminating
		// the major procedure underway.  Like terminating the
		// entire thread or similar.  Essentially, arbitrarily
		// localized recovery is impossible.
		class Error
			: virtual public bases< Throwable >
		{
			public:
				using grade_type= Error;

				class any_tagged_type;
				template< typename tag >
				class tagged_type;
		};
		class Error::any_tagged_type
			: virtual public bases< grade_type, Throwable::any_tagged_type > {};
		template< typename tag >
		class Error::tagged_type
			: virtual bases< grade_type::any_tagged_type, Throwable::tagged_type< tag > > {};
		using AnyTaggedError= Error::any_tagged_type;
		template< typename tag >
		using TaggedError= Error::tagged_type< tag >;

		// `Violation`s are unrecoverable events which happen to
		// the process.  They are impossible to recover from.
		// Handlers for this should be treated almost like
		// `[[noreturn]]` functions, such as `std::terminate_handler`.
		// Mostly one would catch this class if they intended to
		// perform a bit of local persistent state sanitization
		// which is somewhat different to the normal dtors in scope
		// and then continue the unwind process
		class Violation
			: virtual public bases< Throwable >
		{
			private:
				bool active= true;

			public:
				using grade_type= Violation;
				class any_tagged_type;
				template< typename tag >
				class tagged_type;

				~Violation() override { if( not active ) abort(); }

				Violation( const Violation &copy )= delete;
				Violation( Violation &copy ) : active( copy.active ) { copy.active= false; }
		};
		class Violation::any_tagged_type : virtual public bases< grade_type, Throwable::any_tagged_type > {};
		template< typename tag >
		class Violation::tagged_type : virtual public bases< grade_type::any_tagged_type, Throwable::tagged_type< tag > > {};
		using AnyTaggedViolation= Violation::any_tagged_type;
		template< typename tag >
		using TaggedViolation= Violation::tagged_type< tag >;

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
		class NamedResourceThrowable : public virtual synthetic_exception< struct named_resource_throwable, Throwable >, virtual public NamedResourceInterface {};
		using AnyTaggedNamedResourceThrowable= NamedResourceThrowable::any_tagged_type;
		template< typename tag >
		using TaggedNamedResourceThrowable= NamedResourceThrowable::tagged_type< tag >;

		using NamedResourceNotification= synthetic_exception< struct named_resource_notification, Notification, NamedResourceThrowable >;
		using AnyTaggedNamedResourceNotification= NamedResourceNotification::any_tagged_type;
		template< typename tag >
		using TaggedNamedResourceNotification= NamedResourceNotification::tagged_type< tag >;

		using NamedResourceException= synthetic_exception< struct named_resource_exception, Exception, NamedResourceThrowable >;
		using AnyTaggedNamedResourceException= NamedResourceException::any_tagged_type;
		template< typename tag >
		using TaggedNamedResourceException= NamedResourceException::tagged_type< tag >;

		using NamedResourceError= synthetic_exception< struct named_resource_error, Error, NamedResourceThrowable >;
		using AnyTaggedNamedResourceError= NamedResourceError::any_tagged_type;
		template< typename tag >
		using TaggedNamedResourceError= NamedResourceError::tagged_type< tag >;

		using NamedResourceViolation= synthetic_exception< struct named_resource_violation, Violation, NamedResourceThrowable >;
		using AnyTaggedNamedResourceViolation= NamedResourceViolation::any_tagged_type;
		template< typename tag >
		using TaggedNamedResourceViolation= NamedResourceViolation::tagged_type< tag >;


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
			: virtual public synthetic_exception< struct allocation_throwable, Throwable >, virtual public AllocationAmountInterface {};
		using AnyTaggedAllocationThrowable= AllocationThrowable::any_tagged_type;
		template< typename tag >
		using TaggedAllocationThrowable= AllocationThrowable::tagged_type< tag >;

		using AllocationException= synthetic_exception< struct allocation_exception, Exception, AllocationThrowable >;
		using AnyTaggedAllocationException= AllocationException::any_tagged_type;
		template< typename tag >
		using TaggedAllocationException= AllocationException::tagged_type< tag >;

		using AllocationError= synthetic_exception< struct allocation_error, Error, AllocationThrowable >;
		using AnyTaggedAllocationError= AllocationError::any_tagged_type;
		template< typename tag >
		using TaggedAllocationError= AllocationError::tagged_type< tag >;

		using AllocationViolation= synthetic_exception< struct allocation_violation, Violation, AllocationThrowable >;
		using AnyTaggedAllocationViolation= AllocationViolation::any_tagged_type;
		template< typename tag >
		using TaggedAllocationViolation= AllocationViolation::tagged_type< tag >;

		class MessageStorage
			: virtual public Throwable
		{
			protected:
				std::string storage;

				MessageStorage()= default;
				explicit MessageStorage( std::string storage ) : storage( std::move( storage ) ) {}

			public:
				const char *message() const noexcept { return storage.c_str(); }
		};

		template< typename std_exception >
		class GenericExceptionBridge
			: virtual public std_exception, public virtual ExceptionBridgeInterface, virtual public Exception
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
				class Undergird
					: virtual public Kind, virtual protected GenericExceptionBridge< std::bad_alloc >,
					virtual protected MessageStorage, virtual protected AllocationAmountStorage,
					virtual public std::bad_alloc
				{};

				class Exception
					: virtual private Undergird, virtual public Kind, public virtual std::bad_alloc
				{
					public:
						explicit Exception( std::string message ) : MessageStorage( std::move( message ) ) {}
				};

				return Exception{ std::move( message ) };
			}
			else if constexpr( std::is_base_of_v< Exception, Kind > )
			{
				class Undergird
					: virtual public Kind, virtual protected GenericExceptionBridge< std::exception >,
					virtual protected MessageStorage, virtual protected AllocationAmountStorage,
					virtual public std::exception
				{};

				class Exception
					: virtual private Undergird,
					virtual public Kind,
					virtual public std::exception
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
					public:
						explicit Thrown( std::string message ) : MessageStorage( std::move( message ) ) {}
				};

				return Thrown{ std::move( message ) };
			}
		}

		using FinishedException= synthetic_exception< struct finished_exception, Exception >;
		using AnyTaggedFinishedException= AnyTagged< FinishedException >;
		template< typename tag > using TaggedFinishedException= Tagged< FinishedException, tag >;
	}

	inline namespace exports {}
	namespace exports::inline exceptions
	{
		using namespace detail::exceptions::exports;
	}
}
