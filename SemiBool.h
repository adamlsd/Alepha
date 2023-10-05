static_assert( __cplusplus > 2020'00 );

#pragma once

namespace Alepha::inline Cavorite  ::detail::  semi_bool
{
	inline namespace exports { class SemiBool; }

	/*!
	 * A semi-indeterminite bool.
	 *
	 * A semibool is a boolean-like value which exhibits a reduced subset of
	 * the algebra that booleans exhibit.
	 *
	 * Semibools have two states -- Indeterminite (I for short) and Determinite (D for short).
	 * 
	 * Just like boolean algebra, these equality and assignment properties hold:
	 *
	 *  * `a == b and b == c` implies `a == c`
	 *  * `a == b and b != c` implies `a != c`
	 *  * `a == b` and `c= b` implies that `a == c`
	 *  * `a != b` and `c= b` implies that `a != c`
	 *
	 * However, unlike boolean algebra the following implication is weakened in
	 * semibool algebras (it does entirely hold in boolean algebras, however).
	 * `a != b and b != c` does not imply `a == c`
	 *
	 * One cannot check the "truth" value of a semi-bool.  They model a field
	 * of infinite values, where one special value is equal to itself, and all
	 * other values are universally unique -- it is not possible to know the value
	 * of copies unless they're definite.
	 *
	 * This kind of algebra comes up a lot when working with stateful objects that
	 * transition to a terminal state.  Comparing such objects to each other should
	 * return `true` when both objects are in the terminal state.  Thus we can use
	 * `SemiBool` as a valid lens type for this kind of algebra.  `SemiBool( ended )`
	 * is a suitable for use in an `equality_lens` for such types.
	 */ 
	class SemiBool
	{
		private:
			enum State : bool { Indeterminate= false, Definite= true, };
			State state;

		public:
			explicit SemiBool( bool state ) : state( State( state ) ) {}

			friend bool
			operator == ( const SemiBool &lhs, const SemiBool &rhs )
			{
				return lhs.state and rhs.state;
			}
	};
}

namespace Alepha::Cavorite::inline exports::inline semi_bool
{
	using namespace detail::semi_bool::exports;
}
