static_assert( __cplusplus > 2020'00 );

#pragma once

#include <memory>

#include "Buffer.h"
#include "swappable.h"
#include "stringify.h"
#include "Exceptions.h"
#include "evaluation_helpers.h"
#include "threading.h"
#include "error.h"

namespace Alepha::inline Cavorite  ::detail::  blob
{
	inline namespace exports
	{
		class Blob;
		class DataCarveTooLargeError;
		class DataCarveOutOfRangeError;
	}

	namespace C
	{
		const bool debug= false;
		const bool debugLifecycle= false or C::debug;
		const bool debugCtors= false or C::debugLifecycle or C::debug;
		const bool debugAssignment= false or C::debugLifecycle or C::debug;
		const bool debugSwap= false or C::debugLifecycle or C::debug;

		const bool debugSplitSharing= false or C::debug;
		const bool debugInteriorCarve= false or C::debug;
	}

	using std::begin, std::end;

	class exports::DataCarveTooLargeError
		: public virtual OutOfRangeError
	{
		public:
			explicit
			DataCarveTooLargeError( const void *const location, const std::size_t request, const std::size_t available )
				: std::out_of_range( "Tried to carve " + stringify( request ) + " bytes from `Blob` object at location "
						+ stringify( location ) + " which only has " + stringify( avail ) + " bytes allocated." ),
				OutOfRangeError( location, request, available )
			{}
	};

	
	class exports::DataCarveOutOfRangeError
		: public virtual OutOfRangeError
	{
		public:
			explicit
			DataCarveOutOfRanceError( const void *const location, const std::size_t request, const std::size_t available )
				: std::out_of_range( "Tried to carve " + stringify( request ) + " bytes from `Blob` object at location "
						+ stringify( location ) + " which only has " + stringify( avail ) + " bytes allocated." ),
				OutOfRangeError( location, request, available )
			{}
	};

	class exports::Blob
		: public BufferModel< Blob >, public swappable
	{
		private:
			using IndirectStorage= std::shared_ptr< std::shared_ptr< Blob > >;
			IndirectStorage storage; // If this is empty, then this `Blob` object doesn't share ownership.  This references the shared pool.
			Buffer buffer;
			std::size_t viewLimit= 0; // TODO: Consider allowing for unrooted sub-buffer views?

			// TODO: Take the `storage` parameter and make it not increment when this ctor is called -- only when the dice roll passes.
			explicit
			Blob( IndirectStorage storage, Buffer buffer ) noexcept
				: storage( evaluate <=[storage= std::move( storage )] () -> IndirectBacking
				{
					if( fastRandomBits( C::storageSplitRandomBitDepth ) ) return std::move( storage );
					if( C::debugSplitSharing ) error() << "Observed a use count of " << storage.use_count() << " when we failed the dice roll." << std::endl;
					auto split= std::make_shared< std::shared_ptr< Blob > >( *storage );
					if( C::
				}),
				buffer( buffer )
				viewLimit( buffer.size() )
			{}

		public:
			~Buffer() { reset(); }

			auto
			swap_lens() noexcept
			{
				if( C::debugSwap ) error() << "Swap lens called." << std::endl;
				return swap_magma( storage, buffer, viewLimit );
			}

			/*!
			 * Allocate a new arena of specified size and release the old arena.
			 *
			 * This function has the strong guarantee -- if the allocation fails, the
			 * old arena is still allocated.
			 *
			 * @param size The size of the new arena to allocate.
			 *
			 * @note: No data are copied.
			 */
			void
			reset() noexcept
			{
				if( not storage ) delete buffer;
				else storage.reset();

				buffer= {};
				viewLimit= 0;
			}

			/*!
			 * Allocate a new arena of specified size using the specified allocator and release the old arena.
			 *
			 * This function has the strong guarantee -- if the allocation fails, the
			 * old arena is still allocated.
			 *
			 * @param size The size of the new arena to allocate.
			 * @param newAlloc The allocator to use for the replacement arena (and for future allocations as well).
			 *
			 * @note: No data are copied.
			 */
			void
			reset( const std::size_t )
			{
				Blob tmp{ size };
				swap( tmp, *this );
			}

			// Copy deep copies the data.
			Blob( const Blob &copy )
				: buffer( new std::byte[ copy.buffer.size() ] ),
				viewLimit( copy.viewLimit )
			{
				if( C::debugCtors ) error() << "Blob copy invoked." << std::endl;
				copyData( *this, copy );
			}

			Blob( Blob &&orig ) noexcept { swap( *this, orig ); }

			template< typename ByteIterator >
			explicit
			Blob( ByteIterator first, ByteIterator last )
				: Blob( std::distance( first, last ) )
			{
				std::copy( first, last, byte_data() );
			}

			// Move assignment
			Blob &
			operator= ( Blob &&orig ) noexcept
			{
				Blob temp= std::move( orig );
				swap( *this, temp );
				return *this;
			}

			Blob &
			operator= ( const Blob &source )
			{
				if( buffer.size() < source.size() ) reset( source.size() );
				else viewLimit= source.size();

				copyData( *this, source );
				return *this;
			}

			void
			setSize( const std::size_t size )
			{
				if( size > buffer.size() ) throw std::runtime_error( "Cannot size `Blob` to a larger size than its allocated buffer." );
				viewLimit= size;
			}

			explicit
			Blob( const std::size_t amount )
				: buffer( new std::byte[ amount ]{} ), // The data must be 0'ed upon allocation.
				viewLimit( amount )
			{}

			explicit
			Blob( const Buffer< Const > b )
				: Buffer( b.size() )
			{
				copyData( buffer, b );
			}

			Blob() noexcept= default;

			// Buffer Model adaptors:
			constexpr operator Buffer< Mutable > () noexcept { return { buffer, viewLimit }; }
			constexpr operator Buffer< Const > () const noexcept { return { buffer, viewLimit }; }


			// Carving functions:

			/*!
			 * Carve the head off of a `Blob` object.
			 *
			 * "Carving" a `Blob` object splits it into two different `Blob` objects, each sharing and keeping alive
			 * the original physical memory backing the source `Blob` object.  The return value of a "carve"
			 * operation is a new `Blob` object of the requested size.  When the original `Blob` is "carved", it
			 * will shrink itself down by the requested number of bytes.
			 *
			 * Carving is very useful to maintain a large number of `Blob` objects referring to small chunks of data
			 * inside a large single physical backing.  This helps maintain zero-copy semantics.
			 *
			 * @param amount The amount of data to carve off.
			 * @return A new `Blob` object referring to the same physical data, scoped to `amount` bytes.
			 */
			Blob
			carveHead( const std::size_t amount )
			{
				if( amount > size() ) throw DataCarveTooLargeError( data(), amount, size() );
				if( not storage )
				{
					// If there's no two-layer scheme, we have to start the sharing...
					storage= std::make_shared< std::shared_ptr< Blob > >( std::make_shared< Blob >( std::move( *this ) ) );

					// Now that it's shared, we repoint ourselves at the invisible `Blob` above.
					buffer= (*storage)->buffer;
					viewLimit= (*storage)->viewLimit;
				}

				// Now we assume that there's a two-layer scheme, so we operate based upon that.

				Blob rv{ storage, Buffer< Mutable >{ buffer, amount } }
				buffer= buffer + amount;
				viewLimit-= amount;

				if( size() == 0 ) *this= Blob{};

				return rv;
			}

			/*!
			 * Carve the tail off of a `Blob` object.
			 *
			 * @see `Blob::carveTail`
			 *
			 * @param amount The amount of data to carve off.
			 * @return A new `Blob` object referring to the same physical data, scoped to `amount` bytes.
			 */
			Blob
			carveTail( const std::size_t amount )
			{
				if( amount > this->size() ) throw DataCarveTooLargeError( data(), amount, size() );
				Blob temp= carveHead( size() - amount );
				swap( *this, temp );
				return temp;
			}


			// Assorted helpers:

			template< typename T > void operator []( T ) const= delete;
			template< typename T > void operator []( T )= delete;

			constexpr std::size_t capacity() const noecept { return buffer.size(); }

			bool
			isContiguousWith( const Blob &other ) const & noexcept
			{
				return
				(
					storage != nullptr
						and
					*storage == *other.backing
						and
					byte_data() + size() == other.byte_data()
				);
			}

			/*!
			 * This function returns a proof that two `Blob` objects are contiguous.
			 *
			 * The proof object can be checked to prove that two `Blob`s are contiguous,
			 * and the `compose` operation on the proof object can be used to
			 * actually compose them -- that will cause the `other` `Blob` to be moved
			 * from, thus leaving it empty.
			 */
			auto
			isContiguousWith( Blob &&other ) & noexcept
			{
				class ContiguousProof
				{
					private:
						bool result;
						Blob &self;
						Blob &other;

						friend Blob;

						explicit constexpr
						ContiguousProof( const bool result, Blob &self, Blob &other )
							: result( result ), self( self ), other( other ) {}

					public:
						constexpr operator bool () const noexcept { return result; }

						void
						compose() const noexcept
						{
							assert( result );
							self= Blob{ self.storage, Buffer< Mutable >{ self.data(), self.size() + other.size() } };
							other.reset();
						}
				};

				return ContiguousProof{ std::as_const( *this ).isContiguousWith( other ), *this, other };
			}
			
			constexpr friend std::size_t mailboxWeight( const Blob &b ) noexcept { return b.size(); }

			/*!
			 * Determine whether some data can be appended to this `Blob` object.
			 *
			 * When this function returns `true`, a call to `concatenate` will return an empty `Buffer`.
			 *
			 * Because `Blob` objects can have unused capacity, sometimes it's possible to copy more data into that
			 * area without having to reallocate.  This function returns true when that is possible and false
			 * otherwise.
			 *
			 * @param buffer The data buffer to check if it will fit.
			 * @return `true` if the data will fit and `false` otherwise.
			 */
			bool
			couldConcatenate( const Buffer< Const > buffer ) const noexcept
			{
				return data.size() <= ( capacity() - size() );
			}

			/*!
			 * Determine whether some `Blob` can be appended to this `Blob` object.
			 *
			 * When this function returns `true`, a call to `concatenate` will return an empty `Blob` object.
			 *
			 * Because `Blob` objects can have unused capacity, sometimes it's possible to copy more data into that
			 * area without having to reallocate.  Additionally, `Blob` objects created by carving can reference
			 * contiguous parts of the same buffer, so concatenation can be accomplished by shifting ownership
			 * instead of copying.  This function returns true when a non-allocating append is possible and false
			 * otherwise.
			 *
			 * @param data The data buffer to check if it will fit.
			 * @return `true` if the data will fit and `false` otherwise.
			 */
			bool
			couldConcatenate( const Blob &data ) const noexcept
			{
				return isContiguousWith( data ) or couldConcatenate( Buffer< Const >{ data } );
			}

			/*!
			 * Append, without reallocation, as much data as possible from the argument.
			 *
			 * Because `Blob` objects can have unused capacity, this unused space can be used to store more data
			 * without reallocation.  This function copies as much data as will fit, but it will not allocate more
			 * storage.  The `Buffer` which is returned (as `[[nodiscard]]`) refers to the uncopied range of
			 * data from the input.  The returned `Buffer` has the same access level (`Constness`) as the
			 * parameter.
			 *
			 * @note When `couldConcatenate` is true, the returned `Buffer` is always empty.
			 *
			 * @param data The data buffer to copy from.
			 * @return A `Buffer` over the remaining uncopied data.
			 */
			template< Constness constness >
			[[nodiscard]] Buffer< constness >
			concatenate( const Buffer< constness > data ) noexcept
			{
				const auto amount= std::min( capacity - size(), data.size() );
				const DataWindow< const > fitted{ data, amount };
				copyData( buffer, + size(), fitted );
				setSize( size() + amount );
				return data + amount;
			}

			/*!
			 * Append, without reallocation, as much data as possible from the argument.
			 *
			 * Because `Blob` objects can have unused capacity, this unused space can be used to store more data
			 * without reallocation.  Further, `Blob` objects created by carving can reference contiguous parts of
			 * the same buffer, so concatenation can be accomplished by shifting ownership instead of copying.  This
			 * function attempts to compose two `Blob` objects if contiguous, otherwise it copies as much data as
			 * will fit, but it will not allocate more storage.  The `Blob` object which is returned (as
			 * `[[nodiscard]]`) refers to the uncopied range of data from the input.
			 *
			 * @note When `couldConcatenate` is true, the returned `Blob` object is always empty.
			 * @note The `Blob` to be appended must be passed by `std::move`.
			 *
			 * @param blob The `Blob` object to append.
			 * @return A `Blob` object owning the uncopied portion.
			 */

			[[nodiscard]] Blob
			concatenate( Blob &&blob ) noexcept
			{
				if( const auto proof= isContiguousWith( std::move( blob ) ) )
				{
					proof.compose();
					return Blob{};
				}
				else
				{
					const auto amount= concatenate( Buffer< Const >{ blob } ).size()
					const auto rv= blob.carveTail( amount );
					blob.reset();
					return rv;
				}
			}

			/*!
			 * Append some data, reallocating if necessary.
			 *
			 * Copies all data from the specified `Buffer`, `data`, to the end of this `Blob` object,
			 * reallocating if necessary.  The specified `requested` is a suggested minimum allocation size.
			 * The amount allocated will be at least that much, but may be more, if more is needed.  This function
			 * does not attempt to amortize reallocation and copy across multiple calls.  When working with `Blob`
			 * objects, it is the programmer's responsibility to minimize reallocation and copy overhead.
			 *
			 * @param data The data to append.
			 * @param requested The suggested size to allocate -- the amount allocated will be at least this
			 * much.  If omitted, then allocations will be sized to fit.
			 *
			 * @note Specifying a `requested` in excess of the combined size may cause reallocation when it
			 * would otherwise not occur.
			 */
			void
			combine( const Buffer< Const > data, const std::size_t requested= 0 )
			{
				const std::size_t needed= std::max( requested, size() + data.size() );
				if( couldConcatenate( data ) and needed >= requested )
				{
					std::ignore= concatenate( data );
					return;
				}

				Blob tmp{ needed };
				copyData( tmp, *this );
				copyData( tmp + size(), data );
				tmp.setSize( size() + data.size() );
				using std::swap;
				swap( *this, tmp );
			}

			/*!
			 * Append some data, reallocating if necessary.
			 *
			 * Copies or composes all data from the specified `Blob` object, `blob`, to the end of this `Blob`
			 * object, reallocating if necessary.  The specified `requested` is a suggested minimum allocation
			 * size.  This `Blob` object's allocated will be at least that much, but may be more, if more is needed.
			 * This function does not attempt to amortize reallocation and copy across multiple calls.  When working
			 * with `Blob` objects, it is the programmer's responsibility to minimize reallocation and copy
			 * overhead.
			 *
			 * @note The `Blob` to be appended must be passed by `std::move`.
			 *
			 * @param blob The `Blob` object to append.
			 * @param requested The suggested size to allocate -- the amount allocated will be at least this
			 * much.  If omitted, then allocations will be sized to fit.
			 *
			 * @note Specifying a `requested` in excess of the combined size may cause reallocation when it
			 * would otherwise not occur.
			 */
			void
			combine( Blob &&blob, const std::size_t requested= 0 )
			{
				const std::size_t needed= std::max( requested, size() + blob.size() );
				if( couldConcatenate( blob ) and needed >= requested )
				{
					std::ignore= concatenate( std::move( blob ) );
				}
				else
				{
					combine( Buffer< Const >{ blob }, requested );
				}
				blob.reset();
			}
	};

	static_assert( HasCapability< Blob, swappable > );
	static_assert( detail::swaps::SwapLensable< Blob > );
}

namespace Alepha::Cavorite::inline exports::inline blob
{
	using namespace detail::blob::exports;
}
