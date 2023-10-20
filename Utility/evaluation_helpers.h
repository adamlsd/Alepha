static_assert( __cplusplus > 2020'00 );

#pragma once

#include <Alepha/Alepha.h>

#include <tuple>
#include <utility>

namespace Alepha::Hydrogen::Utility  ::detail::  evaluation_helpers
{
	inline namespace exports
	{
		/*!
		 * Mechanism to clarify immediately-invoked lambdas.
		 *
		 * Immediately invoked lambdas are a very useful tool.
		 *
		 * ```
		 * const auto mySortedArray= [&]
		 * {
		 *	std::vector< std::string > rv;
		 *	// Populate it...
		 *	std::sort( begin( rv ), end( rv ) );
		 *	return rv;
		 * }();
		 * ```
		 *
		 * The above code defines a sorted vector as const by immediately invoking a lambda to do it.  The problem,
		 * however, is that it is entirely unclear whether a lambda is being invoked or defined until the very end
		 * of the lambda's definition is reached.  This can become very confusing when there are several nested
		 * levels of lambdas and they get a bit large.
		 *
		 * `evaluate` creates a hook that makes it more clear that it is an immediately-invoked lambda:
		 *
		 * ```
		 * const auto mySortedArray= evaluate <=[&]
		 * {
		 * 	std::vector< std::string > rv;
		 * 	// Populate it...
		 * 	std::sort( begin( rv ), end( rv ) );
		 * 	return rv;
		 * };
		 * ```
		 * 
		 * Because `evaluate <=` precedes the lambda definition, it is quite clear that something else is going on
		 * here.  The `<=` in this case should be thought of as a `fat left arrow` -- the lambda is being put
		 * through an evaluation operation.
		 */
		inline struct evaluate_t {} evaluate;

		/*!
		 * Mechanism to define initializer blocks.
		 *
		 * C++ does not have initializer blocks, but it does have the ability to define variables that are set to
		 * values before `main` is invoked.  It is possible to use constructors or functions to leverage this fact
		 * and make blocks of code that run before main:
		 *
		 * ```
		 * struct MyThing
		 * {
		 *	MyThing()
		 *	{
		 *		std::cout << "Hello World, before main!" << std::endl;
		 *	}
		 * } beforeMain;
		 * ```
		 *
		 * The above code works such that, as a side-effect of the construction of `beforeMain`, the desired code is
		 * run.  However, it is rather cumbersome.  There are ways of simplifying this:
		 *
		 * ```
		 * int beforeMainFunction() { std::cout << "Hello World, before main!" << std::endl; return 42; }
		 * const int beforeMain= beforeMainFunction();
		 * ```
		 *
		 * While this is a bit less confusing, there's still a need to define a function and return a dummy
		 * variable.  Lambdas with immediate invocation syntax (see `evaluate`) can make this nicer still:
		 *
		 * ```
		 * const int beforeMain= evaluate <=[] { std::cout << "Hello World, before main!" << std::endl; return 0; };
		 * ```
		 *
		 * That is a bit better, but that pesky dummy value is still there.  The fact that this is an int is going
		 * to be a potential source of confusion.  Instead, we want to keep the reader focused on the fact that code
		 * is being run, and disguise the fact that there's a variable involved.
		 *
		 * `enroll` creates a hook by which the above techniques can be made more clear that it one is running
		 * pre-main code.
		 *
		 * ```
		 * auto myInitBlock= enroll <=[]
		 * {
		 * 	std::cout << "Hello World, before main!" << std::endl;
		 * };
		 * ```
		 *
		 * Because `enroll <=` precedes the lambda definition it is quite clear that something else is going on
		 * here.  The `<=` in this case should be thought of as a `fat left arrow` -- the lambda is being given
		 * to `enroll` to be used as an initializer block.
		 */
		inline struct enroll_t {} enroll;

		/*!
		 * Mechanism to define lambda capture of a value.
		 *
		 * Sometimes it's useful or necessary to take a known value and wrap it in a function-like interface.  Some
		 * APIs let programmers provide functions which act as customization points.  Sometimes the value is already
		 * at hand, and it needs to be wrapped in a lambda.
		 *
		 * For example:
		 *
		 * ```
		 * auto wrapped= [myValue] { return myValue; };
		 * ```
		 *
		 * While the above is perfectly adequate, it is a bit cumbersome.  `myValue` is used twice, there's a lot of
		 * mechanical syntax for lambda function definition, etc.  `lambaste` provides a simpler alternative:
		 *
		 * ```
		 * auto wrapped= lambaste <=myValue;
		 * ```
		 *
		 * Because `lambaste <=` precedes the variable, it is clear that something is going on here.  The `<=` in
		 * this case should be thought of as a `fat left arrow` -- the value is being given to `lambaste` to be used
		 * in constructing a function-object.
		 *
		 * @note Lambaste is a stupid pun -- a better name might be in order.
		 */
		inline struct lambaste_t {} lambaste;
	}

	template< typename Function >
	constexpr decltype( auto )
	operator <=( evaluate_t, Function &&func ) noexcept( noexcept( std::forward< Function >( func )() ) )
	{
		return std::forward< Function >( func )();
	}

	template< typename Function >
	struct registration
	{
		explicit registration( Function f ) { evaluate <=f; }
	};

	template< typename Function >
	constexpr auto
	operator <=( enroll_t, Function &&func ) noexcept
	{
		return registration{ std::forward< Function >( func ) };
	}

	template< typename ValueType >
	constexpr auto
	as_func( ValueType value ) noexcept( std::is_nothrow_move_constructible_v< ValueType > )
	{
		return [value= std::move( value )]() -> std::decay_t< ValueType > { return value; };
	}

	template< typename ValueType >
	constexpr decltype( auto )
	operator <=( lambaste_t, ValueType value ) noexcept( noexcept( as_func( std::move( value ) ) ) )
	{
		return as_func( std::move( value ) );
	}
}

namespace Alepha::Hydrogen::Utility::inline exports::inline evaluation_helpers
{
	using namespace detail::evaluation_helpers::exports;
}
