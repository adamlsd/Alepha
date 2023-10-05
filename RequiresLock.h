static_assert( __cplusplus > 2020'00 );

#pragma once

#include <type_traits>

#include <mutex>
#include <iostream>
#include <exception>

#include <boost/thread.hpp>

#include "type_lisp.h"
#include "Concepts.h"

namespace Alepha::inline Cavorite  ::detail::  requires_lock
{
	inline namespace exports
	{
		class RequiresLock;
		class RequiresExclusiveLock;
	}

	template< template< typename ... > class Generic >
	struct WrapGeneric;

	template< template< typename ... > class ... Generics >
	using GenericTypeList= TypeList< WrapGeneric< Generics >... >;

	template< typename List, template< typename ... > class Element >
	constexpr bool generic_list_contains_v= list_contains_v< List, WrapGeneric< Element > >;

	using LockTypes= GenericTypeList
	<
		std::lock_guard,
		std::unique_lock,
		std::scoped_lock,
		boost::lock_guard,
		boost::unique_lock,
		boost::shared_lock,
		boost::upgrade_lock,
		boost::upgrade_to_unique_lock
	>;

	// These locks cannot be unlocked
	using FixedLockTypes= GenericTypeList
	<
		std::lock_guard,
		boost::lock_guard
	>;

	// These locks are shared
	using SharedLockTypes= GenericTypeList
	<
		boost::shared_lock,
		boost::upgrade_lock
	>;

	template< template< typename ... > class Lock >
	constexpr bool is_lock_v= generic_list_contains_v< LockTypes, Lock >;
	
	template< typename< typename ... > class Lock >
	constexpr bool is_unlockable_v=
		not generic_list_contains_v< FixedLockTypes, Lock >
			and
		is_lock_v< Lock >
	;

	template< typename< typename ... > class Lock >
	constexpr bool is_exclusive_lock_v=
		not generic_list_contains_v< SharedLockTypes, Lock >
			and
		is_lock_v< Lock >
	;

	template< typename >
	constexpr bool is_exclusive_lock_object_v= false;

	template< template< typename ... > class Lock, typename Mutex >
	constexpr bool is_exclusive_lock_object_v< Lock< Mutex > >
	{
		is_exclusive_lock_v< Lock >;
	};

	template< typename T >
	concept ExclusiveLockObject= is_exclusive_lock_object_v< T >;

	template< typename Object >
	concept Unlockable=
	requires( const Object &o )
	{
		{ o.owns_lock() } -> ConvertibleTo< bool >
	};

	template< typename Object >
	constexpr bool is_lock_object= false;

	template< typename Mutex, template< typename ... > class PotentialLock >
	constexpr bool is_lock_object< PotentialLock< Mutex > >{ is_lock_v< PotentialLock >

	template< typename Object >
	concept ExclusiveLockObject= is_lock_object< Object > and is_exclusive_lock<

	class exports::RequiresLock
	{
		public:
			template< LockObject Object >
			RequiresLock( Object o ) noexcept
			{
				if constexpr( Unlockable< Object > ) if( not lock.owns_lock() )
				{
					std::cerr << "Lock " << &lock << " was not locked!" << std::endl;
					std::terminate();
				}
			}
	};

	class exports::RequiresExclusiveLock
	{
		public:
			template< ExclusiveLockObject Object >
			RequiresExclusiveLock( const Object &lock ) noexcept
			{
				if constexpr( Unlockable< Object > ) if( not lock.owns_lock() )
				{
					std::cerr << "Lock " << &lock << " was not locked!" << std::endl;
					std::terminate();
				}
			}
	};
}

namespace Alepha::inline exports::inline requires_lock
{
	using namespace detail::requires_lock::exports;
}
