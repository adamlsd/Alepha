static_assert( __cplusplus > 2020'00 );

#pragma once

#include <Alepha/Alepha.h>

#include <Alepha/Meta/type_value.h>

namespace Alepha::Hydrogen::Meta::Container
{
	inline namespace exports { inline namespace meta_container_vector {} }

	namespace detail::meta_container_vector
	{
		inline namespace exports
		{
			template< typename ... Members > struct vector;

			template< typename ... Members >
			constexpr auto
			template_for( vector< Members... > );
		}

		template< typename ... Members >
		struct template_for_binder
		{
			template< typename Body >
			constexpr void
			operator <=( Body b )
			{
				auto wrapper= [&]( auto element ) { b( element ); return nullptr; };
				std::nullptr_t loop[]= { wrapper( type_value< Members >{} )... };
			}
		};

		template< typename ... Members >
		constexpr auto
		exports::template_for( vector< Members... > )
		{
			return template_for_binder< Members... >{};
		}

		template< typename Vector >
		struct vector_iterator
		{
			int offset= 0;

			constexpr auto operator *() const;
			constexpr vector_iterator &operator ++() { ++offset; return *this; }
			constexpr vector_iterator operator++ ( int ) { auto rv= *this; ++offset; return rv; }

			friend constexpr bool
			operator == ( const vector_iterator &lhs, const vector_iterator &rhs )
			{
				return lhs.offset == rhs.offset;
			}

			friend constexpr bool
			operator != ( const vector_iterator &lhs, const vector_iterator &rhs )
			{
				return lhs.offset != rhs.offset;
			}
		};

		template< typename List >
		struct dereferenced_iterator
		{
			vector_iterator< List > iter;
		};

		template< typename MetaFunction, typename Arg1, typename First, typename ... Members >
		constexpr decltype( auto )
		invoke_call( MetaFunction func, type_value< Arg1 > arg1, dereferenced_iterator< vector< First, Members... > > deref )
		{
			if( deref.iter.offset == 0 ) return func( arg1, type_value< First >{} );
			else return invoke_call( func, arg1, dereferenced_iterator< vector< Members... > >{ deref.iter.offset - 1 } );
		}

		template< typename MetaFunction, typename Arg1, typename First >
		constexpr decltype( auto )
		invoke_call( MetaFunction func, type_value< Arg1 > arg1, dereferenced_iterator< vector< First > > deref )
		{
			if( deref.iter.offset == 0 ) return func( arg1, type_value< First >{} );
			else throw "Out of bounds iterator";
		}
		
		template< typename First, typename ... Members, typename Value >
		constexpr bool
		operator == ( const dereferenced_iterator< vector< First, Members... > > deref, type_value< Value > value )
		{
			if( deref.iter.offset == 0 ) return type_value< First >{} == value;
			else return dereferenced_iterator< vector< Members... > >{ deref.iter.offset - 1 } == value;
		}

		template< typename Value >
		constexpr bool
		operator == ( const dereferenced_iterator< vector<> > iter, type_value< Value > value )
		{
			return false;
		}

		template< typename Vector >
		constexpr auto
		vector_iterator< Vector >::operator *() const
		{
			return dereferenced_iterator< Vector >{ *this };
		}

		template< typename ... Members >
		struct exports::vector
		{
			using iterator= vector_iterator< vector >;
			using const_iterator= vector_iterator< vector >;

			constexpr auto begin() const { return vector_iterator< vector >{}; }
			constexpr auto end() const { return vector_iterator< vector >{ sizeof...( Members ) }; }

			template< typename Type >
			[[nodiscard]] constexpr auto
			push_back( type_value< Type > ) const
			{
				return vector< Members..., Type >{};
			}

			template< typename Type >
			[[nodiscard]] constexpr auto
			push_front( type_value< Type > ) const
			{
				return vector< Type, Members... >{};
			}

			[[nodiscard]] constexpr auto
			clear() const
			{
				return vector<>{};
			}

			constexpr bool empty() const { return size() == 0; }
			constexpr bool size() const { return sizeof...( Members ); }

		};

		template< typename ... LhsMembers, typename ... RhsMembers >
		constexpr bool
		operator == ( const vector< LhsMembers... > &lhs, const vector< RhsMembers... > &rhs )
		{
			return make_value_type( lhs ) == make_value_type( rhs );
		}

		template< typename ... LhsMembers, typename ... RhsMembers >
		constexpr bool
		operator != ( const vector< LhsMembers... > &lhs, const vector< RhsMembers... > &rhs )
		{
			return make_value_type( lhs ) != make_value_type( rhs );
		}
	}

	namespace exports::meta_container_vector
	{
		using namespace detail::meta_container_vector::exports;
	}
}
