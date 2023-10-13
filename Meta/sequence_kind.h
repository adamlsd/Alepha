static_assert( __cplusplus > 2020'00 );

#pragma once

#include <Alepha/Alepha.h>

#include <array>

#include <Alepha/Meta/is_sequence.h>

namespace Alepha::Hydrogen::Meta  ::detail::  type_traits::sequence_kind
{
	inline namespace exports {}

	template< typename T >
	constexpr bool is_std_array_v= false;

	template< typename T, std::size_t sz >
	constexpr bool is_std_array_v< std::array< T, sz > >{ true };

	template< typename T >
	concept SequenceOfKnownKind= is_sequence_v< T > or is_std_array_v< T >;

	template< SequenceOfKnownKind Seq >
	constexpr const char *
	sequence_kind_f() noexcept
	{
		if constexpr( is_std_array_v< Seq > ) return "array";
		if constexpr( is_vector_v< Seq > ) return "vector";
		if constexpr( is_deque_v< Seq > ) return "deque";
		if constexpr( is_list_v< Seq > ) return "list";
		if constexpr( is_string_v< Seq > ) return "string";
		if constexpr( is_forward_list_v< Seq > ) return "forward_list";
	}

	namespace exports
	{
		template< SequenceOfKnownKind Seq >
		constexpr const char *const sequence_kind_v= sequence_kind_f< Seq >();
	}
}

namespace Alepha::Hydrogen::Meta::inline exports::inline type_traits::inline sequence_kind
{
	using namespace detail::type_traits::sequence_kind::exports;
}

