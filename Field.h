static_assert( __cplusplus > 2020'00 );

#pragma once

#include <string>
#include <type_identity>

#include "Concepts.h"
#include "TotalOrder.h"
#include "ConstexprString.h"

namespace Alepha::inline Cavorite  ::detail:: field
{
	inline namespace exports
	{
		// To facilitate the auto-generation of key-value style entries, you can declare a struct of with
		// many `Field` objcts.  A lens can be generated, via `reflection`, which builds a serializer,
		// using the compile time names given to these fields.
		template< typename T, ConstexprString name >
		struct Field;

		template< typename ... Args > using FieldGroup= std::type_identity< std::tuple< Args > >;
	}

	template< typename T >
	struct get_field_name_impl;
	

	template< typename T, ConstexprString name >
	struct get_field_name_impl< Field< T, name > >
	{
		static constexpr ConstexprString value= name;
	}

	namespace exports
	{
		template< typename T >
		constexpr ConstexprString get_field_name_v= get_field_name_impl< T >::value;
	}


	template< typename T >
	struct get_field_type_impl
	{
		using type= T;
	};

	template< typename T, ConstexprString name >
	struct get_field_type_impl< Field< T, name > >
	{
		using type= T;
	};

	namespace exports
	{
		template< typename T >
		using get_field_type_t= typename get_field_type_impl< T >::type;
	}

	// Standin for string type:
	template< ConstexprString name >
	struct exports::Field< std::string, name >
		// It might be worth moving to a forwardin model, in the future.
		// but this "is-a" relationship covers most of the common use cases.
		: std::string
	{
		using std::string::string;

		// Non-conversion, non-deducing constructor, which takes `std::string`.
		template< typename T= std::string >
		constexpr Field( std::type_identity_t< T > s ) noexcept : std::string( std::move( s ) ) {}
	};

	// Numeric like field:
	template< Primitive Numeric, ConstexprString name >
	struct exports::Field< Numeric, name >
	{
		public:
			// Pretend that this is private.
			// Being public means that the initializers for this
			// structure work more cleanly.
			Numeric data;

			constexpr Field &
			operator= ( const Numeric number ) noexcept
			{
				data= number;
				return *this;
			}

			#define ALEPHA_FIELD_BUILD_FIELD_ASSIGNMENT_OPERATOR( OP ) \
			constexpr Field & \
			operator OP ( const Numeric rhs ) noexcept \
			{ \
				data OP rhs; \
				return *this; \
			} \

			// Presently there are ambiguity conflicts to resolve here.
			// They may need to be rewritten to defer conversion?
			#if 0
			ALEPHA_FIELD_BUILD_FIELD_ASSIGNMENT_OPERATOR( += )
			ALEPHA_FIELD_BUILD_FIELD_ASSIGNMENT_OPERATOR( -= )

			ALEPHA_FIELD_BUILD_FIELD_ASSIGNMENT_OPERATOR( *= )
			ALEPHA_FIELD_BUILD_FIELD_ASSIGNMENT_OPERATOR( /= )
			ALEPHA_FIELD_BUILD_FIELD_ASSIGNMENT_OPERATOR( %= )

			ALEPHA_FIELD_BUILD_FIELD_ASSIGNMENT_OPERATOR( ^= )
			ALEPHA_FIELD_BUILD_FIELD_ASSIGNMENT_OPERATOR( &= )
			ALEPHA_FIELD_BUILD_FIELD_ASSIGNMENT_OPERATOR( |= )

			ALEPHA_FIELD_BUILD_FIELD_ASSIGNMENT_OPERATOR( <<= )
			ALEPHA_FIELD_BUILD_FIELD_ASSIGNMENT_OPERATOR( >>= )
			#endif

			#undef ALEPHA_FIELD_BUILD_FIELD_ASSIGNMENT_OPERATOR( OP )

			#define ALEPHA_FIELD_BUILD_BUMP_OPERATOR( OP ) \
			constexpr Field & \
			operator OP () noexcept \
			{ \
				OP( data ); \
				return *this; \
			}\
			\
			constexpr Field \
			operator OP( int ) noexcept \
			{ \
				Field rv{ *this }; \
				OP( *this ); \
				return rv; \
			}

			ALEPHA_FIELD_BUILD_BUMP_OPERATOR( -- )
			ALEPHA_FIELD_BUILD_BUMP_OPERATOR( ++ )

			#undef ALEPHA_FIELD_BUILD_BUMP_OPERATOR

			constexpr TotalOrder operator <=> ( const Field & ) const noexcept= default;

			// These may wind up being more trouble than they're worth?
			constexpr operator const Num & () const noexcept { return data; }
			constexpr operator Num & () noexcept { return data; }
	};

	namespace exports
	{
		template< typename T >
		constexpr bool is_field_v= false;

		template< typename T, ConstexprString name >
		constexpr bool is_field_v< Field< T, name > >{ true };

		template< typename T >
		concept IsField= is_field_v< T >;
	}

	template< typename T, ConstexprString name >
	struct exports::Field< std::vector< T >, name >
		// Again, it might be worth reconsidering the forwarding cases?
		: std::vector< T >
	{
		using std::vector< T >::vector;
		constexpr Field( std::vector< T > v ) noexcept : std::vector< T >( std::move( v ) ) {}
	};

	template< typename Type, ConstexprString name >
	class exports::Field< std::optional< Type >, name >
	{
		private:
			std::optional< Type > store;

			struct Evil {};

		public:
			constexpr Field()= default;

			constexpr
			Field( Type v ) noexcept( noexcept( std::optional< Type >{ std::move( v ) } ) )
				: store( std::move( v ) ) {}

			constexpr
			Field( std::optional< Type > v ) noexcept( noexcept( std::optional< Type >{ std::move( v ) } ) )
				: store( std::move( v ) ) {}

			friend decltype( auto ) get_base( const Field &f ) { return f.store; }

			friend auto &
			unwrap( const Field &f ) noexcept
			requires( OStreamable< Type > )
			{
				return f.store;
			}

			friend auto &
			unwrap( Field &f ) noexcept
			requires( IStreamable< Type > )
			{
				return f.store;
			}

			friend auto &
			equality_lens( const Field &f )
			requires( EqualityComparable< Type > )
			{
				return f.store;
			}

			template< typename T >
			requires
			(
				ConstructibleFrom< std::optional< Type >, T >
					and
				not ConstructibleFrom< Evil, T >
			)
			explicit( not ConvertibleTo< T, std::optional< Type > > ) constexpr
			Field( T v ) noexcept( noexcept( std::optional< Type >{ std::move( v ) } ) )
				: store( std::move( v ) ) {}

			constexpr Field &
			operator= ( ConvertibleToButNotSameAs< std::optional< Type > > auto val )
			{
				store= val;
				return *this;
			}

			constexpr bool has_value() const noexcept{ return store.has_value(); }

			constexpr decltype( auto ) value() { return store.value(); }
			constexpr decltype( auto ) value() const { return store.value(); }

			template< typename= void > // This has to be a template, for the `requires` clauses to work.
			friend bool
			operator == ( const Field &lhs, const Field &rhs )
			noexcept( noexcept( lhs.store == rhs.store ) )
			requires( requires{ { lhs.store == rhs.store } -> ConvertibleTo< bool > } )
			{
				return lhs.store == rhs.store;
			}

			template< typename= void > // This has to be a template, for the `requires` clauses to work.
			friend bool
			operator == ( const Field &lhs, const Field &rhs )
			noexcept( noexcept( lhs.store < rhs.store ) )
			requires( requires{ { lhs.store < rhs.store } -> ConvertibleTo< bool > } )
			{
				return lhs.store < rhs.store;
			}

			template< typename= void > // This has to be a template, for the `requires` clauses to work.
			friend bool
			operator <= ( const Field &lhs, const Field &rhs )
			noexcept( noexcept( lhs.store <= rhs.store ) )
			requires( requires{ { lhs.store <= rhs.store } -> ConvertibleTo< bool > } )
			{
				return lhs.store == rhs.store;
			}
	};

	// TODO: Flesh out the key-value streamable system (JSON, XML, etc.)
	template< typename T >
	concept KeyValueStreamable= false;

	template< typename T >
	concept StreamableNonAggregate= not Aggregate< T > and KeyValueStreamable< T > and std::is_class_v< T >;

	template< typename T >
	concept ValidFieldType= StreamableNonAggregate< T > or Aggregate< T > or is_optional_v< T >;


	template< typename Type, ConstexprString s >
	const Type &
	get_base( const Field< Type, s > &f )
	{
		return static_cast< const Type & >( f );
	}

	template< ValidFieldType Type, ConstexprString name >
	struct exports::Field< Type, name > : Type
	{
		using Type::Type;
		constexpr Field( Type v ) noexcept( std::is_nothrow_move_constructible_v< Type > ) : Type( std::move( v ) ) {}
	};

	// Cross-field equality.
	template< typename Type, ConstexprString name_lhs, ConstexprString name_rhs >
	constexpr bool
	operator == ( const Field< Type, name_lhs > &lhs, const Field< Type, name_rhs > &rhs )
	noexcept( noexcept( get_base( lhs ) == get_base( rhs ) ) )
	requires( FieldType< Type > and EqualityComparable< Type > )
	{
		return get_base( lhs ) == get_base( rhs );
	}

	// C++20 rules allow for this to be flipped, so we don't need the
	// reverse order declaration.
	template< typename Type, ConstexprString name >
	constexpr bool
	operator == ( const Type &lhs, const Field< Type, name > &rhs )
	noexcept( noexcept( get_base( lhs ) == get_base( rhs ) ) )
	requires( FieldType< Type > and EqualityComparable< Type > )
	{
		return get_base( lhs ) == get_base( rhs );
	}

	// Cross-field less-than.
	template< typename Type, ConstexprString name_lhs, ConstexprString name_rhs >
	constexpr bool
	operator < ( const Field< Type, name_lhs > &lhs, const Field< Type, name_rhs > &rhs )
	noexcept( noexcept( get_base( lhs ) < get_base( rhs ) ) )
	requires( FieldType< Type > and EqualityComparable< Type > )
	{
		return get_base( lhs ) < get_base( rhs );
	}

	// While C++20 rules allow for this to be flipped, we still need the
	// reverse order declaration, because the types are inversed...
	template< typename Type, ConstexprString name >
	constexpr bool
	operator < ( const Type &lhs, const Field< Type, name > &rhs )
	noexcept( noexcept( get_base( lhs ) < get_base( rhs ) ) )
	requires( FieldType< Type > and EqualityComparable< Type > )
	{
		return get_base( lhs ) < get_base( rhs );
	}

	template< typename Type, ConstexprString name >
	constexpr bool
	operator < ( const Field< Type, name > &lhs, const Type &rhs )
	noexcept( noexcept( get_base( lhs ) < get_base( rhs ) ) )
	requires( FieldType< Type > and EqualityComparable< Type > )
	{
		return get_base( lhs ) < get_base( rhs );
	}


	// Cross-field less-equal.
	template< typename Type, ConstexprString name_lhs, ConstexprString name_rhs >
	constexpr bool
	operator <= ( const Field< Type, name_lhs > &lhs, const Field< Type, name_rhs > &rhs )
	noexcept( noexcept( get_base( lhs ) <= get_base( rhs ) ) )
	requires( FieldType< Type > and EqualityComparable< Type > )
	{
		return get_base( lhs ) <= get_base( rhs );
	}

	// While C++20 rules allow for this to be flipped, we still need the
	// reverse order declaration, because the types are inversed...
	template< typename Type, ConstexprString name >
	constexpr bool
	operator <= ( const Type &lhs, const Field< Type, name > &rhs )
	noexcept( noexcept( get_base( lhs ) <= get_base( rhs ) ) )
	requires( FieldType< Type > and EqualityComparable< Type > )
	{
		return get_base( lhs ) <= get_base( rhs );
	}

	template< typename Type, ConstexprString name >
	constexpr bool
	operator <= ( const Field< Type, name > &lhs, const Type &rhs )
	noexcept( noexcept( get_base( lhs ) <= get_base( rhs ) ) )
	requires( FieldType< Type > and EqualityComparable< Type > )
	{
		return get_base( lhs ) <= get_base( rhs );
	}
}

namespace Alepha::Cavorite::inline exports::inline field
{
	using namespace detail::field::exports;
}
