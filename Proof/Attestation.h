/*!
 * @file
 * @brief The `Attestation` framework permits code which can provide limited compiletime guarantees of conditions.
 */
#pragma once

#include <cassert>
#include <cstdlib>

#include <utility>
#include <type_traits>


namespace Alepha::Proof
{
	template< typename Attestation >
	Attestation
	impute()
	{
		return Attestation{};
	}

	template< typename Permission >
	auto
	attest( Permission )
	{	
		return impute< typename Permission::attestation_type >();
	}

	#if 0
	namespace attestation_detail
	{
		template< typename ... Args > struct typelist {};

		template< typename T > struct make_typelist;

		template< typename ... Args >
		struct make_typelist< std::tuple< Args... > >
		{
			using type= typelist< Args... >;
		};

		template< typename Policy, typename = void >
		struct conjugates
		{
			using type= typelist< Policy >;
		};

		template< typename Policy >
		struct conjugates< Policy, std::void_t< decltype( typename Policy::conjunction_type{} ) > >
		{
			using type= typename make_typelist< typename Policy::conjunction_type >::type;
		};


		template< typename value, typename Conjugates >
		struct in_set : std::false_type {};

		template< typename value >
		struct in_set< value, typelist< value > > : std::true_type {};

		template< typename value, typename ... Conjugates >
		struct in_set< value, typelist< value, Conjugates... > > : std::true_type {};

		template< typename value, typename X, typename ... Conjugates >
		struct in_set< value, typelist< X, Conjugates... > > : in_set< value, Conjugates... > {};

		template< typename value, typename X >
		struct in_set< value, typelist< X > > : std::false_type {};

		template< typename A, typename B >
		struct is_subset_of : std::false_type {};

		template< typename T, typename ... Left, typename ... Right >
		struct is_subset_of< typelist< T, Left... >, typelist< Right... > >
				: std::integral_constant< bool, in_set< T, Right... >::value
						&& is_subset_of< Left..., Right... > > {};

		template< typename ... Right >
		struct is_subset_of< typelist<>, typelist< Right... > >
				: std::true_type {};

		template< typename ... Left >
		struct is_subset_of< typelist< Left... >, typelist<> >
				: std::false_type {};

		template<>
		struct is_subset_of< typelist<>, typelist<> > : std::true_type {};

		template< typename ... lists >
		struct set_union;

		template< typename ... elements >
		struct set_union< typelist< elements... > > : typelist< elements... > {};

		template< typename ... elements >
		struct set_union< typelist< elements... > > : typelist< elements... > {};
	}
	#endif

	/*!
	 * @brief An Attestation is a representation of some particular fact about runtime code known at compiletime.
	 *
	 * An Attestation is a copyable, zero-storage type which represents at compiletime a fact about runtime code.
	 * Attestations can be used as compiletime predicates for a function by making that function take an attestation
	 * as an argument.  Every attestation is created only by the limited set of classes as indicated by the `averant`
	 * member of the `Policy` for an Attestation.  These policies are also tag types which distinguish the kind of
	 * Attestation.
	 *
	 * Attestations can be copied freely, as any fact once true is considered to continue being true.  Attestations
	 * should not be held over the long term -- they are best used as temporaries, such as ephemeral arguments to
	 * a function requiring that fact to be true.
	 */
	template< typename Policy >
	class Attestation
	{
		private:
			struct permission_t { using attestation_type= Attestation; };
			static inline permission_t permission;

			Attestation()= default;

			template< typename A > friend A impute();

			//template< typename A, typename P > friend A attest( P );

			friend typename Policy::averant;

			//using conjugates= typename attestation_detail::conjugates< Policy >::type;

			template< typename > friend class Attestation;

		public:
			template< typename > class Witness;

		#if 0
			template< typename ... Policies >
			Attestation( Attestation< Policies >... )
			{
				static_assert( attestation_detail::is_subset_of< conjugates,
						typename attestation_detail::set_union< Policies >::type >::value,
						"Cannot make an attestation without sufficient proof of existance for "
						"required facts." );
			}

			template< typename ForeignPolicy >
			Attestation( Attestation< ForeignPolicy > )
					//: Attestation( attestation_detail::expand< ForeignPolicy >::
			{
				static_assert( attestation_detail::is_subset_of< conjugates,
						typelist< Attestations... > >::value, "Cannot make an attestation without "
						"sufficient proof of existance for required facts." );
			}
		#endif

			template< typename T >
			auto 
			aver( const T &r )
			{
				return Witness< const T & >( r );
			}

			template< typename T >
			auto 
			averCopy( T r )
			{
				return Witness< T >( std::move( r ) );
			}
	};

	/*!
	 * @brief A witness knows a fact (Attestation) to be true about some particular value.
	 *
	 * In the Attestation framework, a Witness is an attestation type which binds a particular fact (a regular Attestation) to the
	 * value for which that fact is true.  For example, one can create a `Witness` to the fact that a particular `std::vector` is
	 * sorted.  This is extremely useful as it cuts down on the need to create specialized "constrained" types for specific use cases.
	 *
	 * Witnesses are not actually a proxy for the value for which the fact is true.  A witness must `testify` as to which value the
	 * fact is actually true for.  The `testify` function returns the actual value for which the `Attestation` is known to hold.
	 */
	template< typename Policy >
	template< typename T >
	class Attestation< Policy >::Witness
	{
		private:
			T value;

			friend Attestation< Policy >;

			explicit Witness( T i_v ) : value( std::move( i_v ) ) {}

		public:
			inline friend T &testify( Witness &w ) noexcept { return w.value; }
			inline friend T const &testify( const Witness &w ) noexcept { return w.value; }
			inline friend Attestation< Policy > fact( Witness w ) noexcept
					{ return Attestation< Policy >{}; }
	};

	template< typename Policy >
	template< typename T >
	class Attestation< Policy >::Witness< T & >
	{
		private:
			T *p;

			friend Attestation< Policy >;

			explicit Witness( T &r ) : p( &r ) {}

		public:
			friend T &testify( Witness w ) noexcept { return *w.p; }
			friend Attestation< Policy > fact( Witness w ) noexcept { return Attestation< Policy >{}; }
	};
}
