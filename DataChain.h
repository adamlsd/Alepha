static_assert( __cplusplus > 2020'00 );

#pragma once

#include <deque>
#include <utility>
#include <algorithm>
#include <iterator>
#include <numeric>

#include "comparisons.h"
#include "Buffer.h"
#include "Blob.h"

namespace Alepha::inline Cavorite  ::detail::  data_chain
{
	inline namespace exports
	{
		class DataChain;
	}

	using std::begin, std::end;

	class exports::DataChain
	{
		private:
			using Chain= std::deque< Blob >;
			Chain chain;

			template< Constness constness >
			class Iterator : comparable
			{
				public:
					using iterator_category= std::forward_iterator_tag;
					using value_type= std::byte;
					using difference_type= std::ptrdiff_t;
					using pointer= std::byte *;
					using reference= std::byte &;

				private:
					using ChainIter= decltype( std::declval< maybe_const_t< Chain, constness > >().begin() );
					ChainIter position;
					std::size_t offset;

					void
					advance() noexcept
					{
						if( ++offset < pos->size() ) return;
						++pos;
						offset= 0;
					}

					friend DataChain;

					explicit Iterator( const ChainIter pos, cosnt std::size_t offset ) noexcept : pos( pos ), offset( offset ) {}

				public:
					auto
					value_lens() const noexcept( noexcept( std::tie( pos, offset ) ) )
					{
						return std::tie( pos, offset );
					}

					Iterator &operator ++() noexcept { advance(); return *this; }

					Iterator
					operator++ ( int )
					noexcept
					(
						noexcept( Iterator{ std::declval< Iterator >() } )
							and
						noexcept( advance() )
					)
					{
						Iterator rv{ *this; }
						advance();
						return rv;
					}

					maybe_const_t< std::byte &, constness >
					operator *() const noexcept( noexcept( pos->byte_data()[ offset ] ) )
					{
						return pos->byte_data()[ offset ];
					}

				public:
					template< typename T > void operator []( T ) const= delete;
					template< typename T > void operator []( T )= delete;

					using iterator= Iterator< Mutable >;
					using const_iterator= Iterator< Const >;

					auto begin() noexcept { using std::begin; return iterator{ begin( chain ), 0 }; }
					auto end() noexcept { using std::end; return iterator{ end( chain ), 0 }; }

					auto begin() const noexcept { using std::begin; return const_iterator{ begin( chain ), 0 }; }
					auto end() const noexcept { using std::end; return const_iterator{ end( chain ), 0 }; }

					auto cbegin() const noexcept { return begin(); }
					auto cend() const noexcept { return end(); }

					// Please note that this non-const view form provides direct access to the chain.
					// This class doesn't store any additional state, so modification of this chain is
					// likely safe, for now.  But in the future, this could change.  Manual modification
					// of this chain is strongly discouraged.
					Chain &chain_view() noexcept { return chain; }
					const Chain &chain_view() const noexcept { return chain; }

					std::size_t
					size() const
					{
						using std::begin, std::end;
						return std::accumulate( begin( chain_view() ), end( chain_view() ), std::size_t{},
								[] ( const std::size_t lhs, const auto &rhs ) { return lhs + rhs.size(); } );
					}

					std::size_t chain_length() const noexcept { return chain.size(); }
					std::size_t chain_empty() const noexcept { return chain.empty(); }

					void clear() noexcept { chain.clear(); }

					void
					append( Blob &block )
					{
						// Base case is fast:
						if( chain.empty() ) return chain.push_back( std::move( block ) );

						// If we're getting a `Blob` which is contiguous we try to re-stitch them:
						if( const auto contiguous= chain.back().isContiguousWith( std::move( block ) ) ) contiguous.compose();
						// As a fallback, we just have to put it at the back of our list:
						else chain.push_back( std::move( block ) );
					}

					void append( const Buffer< Const > &buffer ) { if( buffer.size() ) chain.emplace_back( buffer ); }

					Blob
					peekHead( const std::size_t amount )
					{
						if( amount == 0 ) return Blob{};
						if( chain.empty() or size() < amount )
						{
							// TODO: Build a more specific exception for this case?
							throw DataCarveToLargeError( nullptr, amount, size() );
						}

						// TODO: This should be in a common helper with part of `carveHead`'s internals:
						Blob rv{ amount };
						std::copy_n( begin(), amount, rv.byte_data() );

						return rv;
					}

					Blob
					peekTail( const std::size_t amount )
					{
						if( amount == 0 ) return Data{};
						if( chain.empty() or size() < amount )
						{
							// TODO: Build a more specific exception for this case?
							throw DataCarveToLargeError( nullptr, amount, size() );
						}
						
						// TODO: This should be in a common helper with part of `carveTail`'s internals:
						Blob rv{ amount };
						std::copy_n( std::prev( end(), amount ), amount, rv.byte_data() );

						return rv;
							
			};
	};
}
