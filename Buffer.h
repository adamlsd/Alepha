static_assert( __cplusplus > 2020'00 );

#pragma once

#include <vector>
#include <string>
#include <array>
#include <typeinfo>
#include <typeindex>
#include <exception>
#include <stdexcept>

#include <boost/lexical_cast.hpp>

#include "Concepts.h"
#include "assertion.h"
#include "Capabilities.h"
#include "lifetime.h"


namespace Alepha::inline Cavorite  ::detail::  buffer
{
	inline namespace exports {}

	using namespace std::literals::string_literals;

	namespace exports
	{
		class OutOfRangeError
			: public virtual std::out_of_range
		{
			private:
				const void *const baseAddress;
				const std::size_t requestedSize;
				const std::size_t availableSize;

			protected:
				~OutOfRangeError()= 0; // Make class abstract

				explicit
				OutOfRangeError( const void *const address, const std::size_t requestedSize, const std::size_t availableSize )
					: baseAddress( address ), requestedSize( requestedSize ), availableSize( availableSize )
				{}

			public
				const void *getAddress() const noexcept { return baseAddress; }
				const std::size_t getRequestedSize() const noexcept { return requestedSize; }
				const std::size_t getAvailableSize() const noexcept { return availableSize; }
		};

		inline OutOfRangeError::~OutOfRangeError()= default;

		class InsufficientSizeError
			: virtual public OutOfRangeError
		{
			private:
				const std::type_index typeID;

			public:
				explicit
				InsufficientSizeError( const void *const location, const std::size_t requestedSize, const std::size_t availableSize, const std::type_index &type )
					: std::out_of_range( "Tried to access an object of type "s + type.name() + " which is " + stringify( requestedSize ) + " bytes in size.  "
							+ "The request was at location " + stringify( location ) + " which only has " + stringify( availableSize )
							+ " bytes allocated" ),
					OutOfRangeError( location, requestedSIze, availableSize ),
					typeID( type )
				{}
		};

		class OutOfRangeSizeError
			: virtual public OutOfRangeException
		{
			public:
				explicit
				OutOfRangeSizeException( const void *const location, const std::ptrdiff_t requestedOffset, const std::size_t availableSpace )
					: std::out_of_range( "Tried to view a byte offset of " + stringify( requestedOffset ) + " into location " + stringify( location )
							+ " which is " + stringify( availableSpace ) + " bytes in size." ),
					OutOfRangeException( location, requestedOffset, availableSpace )
				{}
		};

		template< Constness constness > class Buffer;
		template< typename Derived > class BufferModel;

		constexpr Buffer< Mutable > copyData( Buffer< Mutable > destination, Buffer< Const > source );

		constexpr void zeroData( Buffer< Mutable > buffer ) noexcept;
	}

	template< Constness constness >
	class exports::Buffer
	{
		public:
			using pointer_type= maybe_const_ptr_t< void, constness >;
			using const_pointer_type= const void *;

			using byte_pointer_type= maybe_const_ptr_t< std::byte, constness >;
			using const_byte_pointer_type= const std::byte *;

		private:
			byte_pointer_type ptr= nullptr;
			std::size_t bytes= 0;

		public:
			constexpr Buffer() noexcept= default;

			constexpr
			Buffer( const pointer_type ptr, const std::size_t bytes ) noexcept
				: ptr( static_cast< byte_pointer_type >( ptr ), bytes( bytes )
			{}

			constexpr
			Buffer( const Buffer< Mutable > &copy ) noexcept
				: ptr( copy.byte_data() ), bytes( copy.size() ) {}

			constexpr
			Buffer( const Buffer< Const > &copy ) noexcept requires( Constness == Mutable )= delete;


			constexpr byte_pointer_type byte_data() const noexcept { return ptr; }
			constexpr pointer_type data() const noexcept { return ptr; }

			constexpr const_byte_pointer_type byte_data() const noexcept { return ptr; }
			constexpr const_pointer_type data() const noexcept { return ptr; }

			constexpr std::size_t size() const noexcept { return bytes; }
			constexpr bool empty() const noexcept { return size() == 0; }

			constexpr const_byte_pointer_type cbegin() const noexcept { return begin(); }
			constexpr const_byte_pointer_type cend() const noexcept { return end(); }

			template< typename T > void operator[]( T ) const= delete;
			template< typename T > void operator[]( T )= delete;

			template< typename T >
			constexpr std::add_lvalue_reference_t< maybe_const_t< T, constness > >
			as( std::nothrow_t ) const noexcept
			{
				assertion( sizeof( T ) <= bytes );
				return *start_lifetime_as< std::add_lvalue_reference_t< maybe_const_t< T, constness > > >( ptr );
			}

			template< typename T >
			constexpr maybe_const_t< T &, constness >
			as() const
			{
				if( sizeof( T ) > bytes ) throw InsufficientSizeError{ ptr, sizeof( T ), bytes, typeid( T ) };
				return this->as< std::add_lvalue_reference_t< maybe_const_t< T, constness > > >( std::nothrow );
			}

			template< typename T >
			constexpr std::add_lvalue_reference_t< std::add_const_t< T & > >
			const_as( std::nothrow_t ) const noexcept
			{
				assertion( sizeof( T ) <= bytes );
				return *start_lifetime_as< std::add_const_t< T > >( ptr );
			}

			template< typename T >
			constexpr std::add_lvalue_reference_t< std::add_const_t< T > >
			const_as() const
			{
				if( sizeof( T ) > bytes ) throw InsufficientSizeError{ ptr, sizeof( T ), bytes, typeid( const T ) };
				return this->const_as< T >( std::nothrow );
			}

			constexpr operator pointer_type () const noexcept { return ptr; }

			/*!
			 * Advance the view of this `Buffer` object.
			 *
			 * Because `Buffer` objects model a pointer to a block of data which is aware of
			 * the size of that block, advancing that pointer should permit a view of the remainder
			 * of that block.
			 *
			 * A common technique in working with such blocks is to have to advance a pointer and
			 * decrease a size count.  This operator does both actions in one semantic step.
			 *
			 * @note Behaves the same as `window= window + amount`.
			 *
			 * Example use case:
			 *
			 * ```
			 * AutoRAII targetFile{[]{ return ::fopen( "output.dat", "wb" ); }, fclose };
			 * Buffer< Const > myBuf= getSomeBufferFromSomewhere();
			 * while( not myBuf.empty() )
			 * {
			 *  const auto amtWritten= fwrite( myBuf, myBuf.size(), 1, targetFile );
			 *  myBuf+= amtWritten;
			 * }
			 * ```
			 *
			 * In this example, the code walks through the buffer pointed to by `myBuf`.  It uses `myBuf`
			 * as a "smart pointer" which knows the end of its range.
			 */
			Buffer &
			operator+= ( const std::size_t amount )
			{
				if( amount > bytes ) throw OutOfRangeSizeError( ptr, amount, bytes );

				ptr+= amount;
				bytes-= amount;

				return *this;
			}
	};

	template< Constness constness >
	constexpr auto
	cbegin( const Buffer< constness > &buffer ) noexcept
	{
		return buffer.cbegin();
	}

	template< Constness constness >
	constexpr auto
	cend( const Buffer< constness > &buffer ) noexcept
	{
		return buffer.cend();
	}

	struct BufferModel_capability {};

	template< typename T >
	concept UndecayedBufferModelable= HasCapability< T, BufferModel_capability >;

	template< typename T >
	concept BufferModelable= UndecayedBufferModelable< std::decay_t< T > >;

	template< typename Derived >
	class exports::BufferModel : public BufferModel_capability
	{
		private:
			constexpr auto &crtp() noexcept { return static_cast< Derived & >( *this ); }
			constexpr const auto &crtp() const noexcept { return static_cast< const Derived & >( *this ); }

			constexpr auto &buffer() { return static_cast< Buffer< Mutable > >( crtp() ); }
			constexpr const auto &buffer() const { return static_cast< Buffer< Const > >( crtp() ); }

		public:
			constexpr auto byte_data() { return buffer().byte_data(); }
			constexpr const auto byte_data() const { return buffer().byte_data(); }

			constexpr auto data() { return buffer().data(); }
			constexpr const auto data() const { return buffer().data(); }

			constexpr decltype( auto ) cbegin() const { return buffer().cbegin(); }
			constexpr decltype( auto ) cend() const { return buffer().cend(); }

			constexpr decltype( auto ) begin() const { return buffer().begin(); }
			constexpr decltype( auto ) end() const { return buffer().end(); }

			constexpr decltype( auto ) begin() { return buffer().begin(); }
			constexpr decltype( auto ) end() { return buffer().end(); }

			constexpr std::size_t size() const { return buffer().size(); }

			constexpr operator void *() { return buffer(); }
			constexpr operator const void *() { return buffer(); }

			constexpr operator const void *() const { return buffer(); }

			template< typename T > void operator[]( T ) const= delete;
			template< typename T > void operator[]( T )= delete;

			template< typename T > constexpr decltype( auto ) as() const { return buffer().template as< T >(); }
			template< typename T > constexpr decltype( auto ) as() { return buffer().template as< T >(); }

			template< typename T > constexpr decltype( auto ) const_as() const { return buffer().template const_as< T >(); }
			template< typename T > constexpr decltype( auto ) const_as() { return buffer().template const_as< T >(); }
	};

	template< typename T >
	extern Constness constness_of_v;

	template< UndecayedBufferModelble T >
	constexpr Constness constness_of_v= std::is_const_v< T >;

	template< Constness constness >
	constexpr Constness constness_of_v< Buffer< constness > >{ constness };

	template< Constness constness >
	constexpr auto
	operator + ( const Buffer< constness > buffer, const std::size_t offset )
	{
		if( offset > buffer.size() ) throw OutOfRangeError{ buffer.data(), offset, buffer.size() };
		return Buffer< constness >{ buffer.byte_data() + offset, buffer.size() - offset };
	}

	template< Constness constness >
	constexpr auto
	operator + ( const std::size_t offset, const Buffer< constness > buffer )
	{
		return buffer + offset;
	}


	// Compute arbitrary offsets with `BufferModel` derivatives.
	template< BufferModelable Type >
	constexpr auto
	operator + ( Type &&item, const std::size_t offset )
	{
		return static_cast< Buffer< constness_of_v< std::remove_reference_t< Type > > > >( item ) + offset;
	}

	constexpr auto
	operator + ( const std::size_t offset, BufferModelable auto &&item )
	{
		return item + offset;
	}

	constexpr Buffer< Mutable >
	make_buffer( Concepts::StandardLayoutAggregate auto &aggregate ) noexcept
	{
		return { &aggregate, sizeof( aggregate ) };
	}

	constexpr Buffer< Const >
	make_buffer( const Concepts::StandardLayoutAggregate auto &aggregate ) noexcept
	{
		return { &aggregate, sizeof( aggregate ) };
	}

	template< Concepts::StandardLayout T >
	constexpr Buffer< Mutable >
	make_buffer( std::vector< T > &vector ) noexcept
	{
		// TODO: Do we need to consider overflow here?
		return { vector.data(), vector.size() * sizeof( T ) };
	}

	template< Concepts::StandardLayout T >
	constexpr Buffer< Const >
	make_buffer( const std::vector< T > &vector ) noexcept
	{
		// TODO: Do we need to consider overflow here?
		return { vector.data(), vector.size() * sizeof( T ) };
	}


	template< Concepts::StandardLayout T, std::size_t size >
	constexpr Buffer< Mutable >
	make_buffer( std::array< T, size > &array ) noexcept
	{
		// TODO: Do we need to consider overflow here?
		return { array.data(), sizeof( array ) };
	}

	template< Concepts::StandardLayout T, std::size_t size >
	constexpr Buffer< Const >
	make_buffer( const std::array< T, size > &array ) noexcept
	{
		// TODO: Do we need to consider overflow here?
		return { array.data(), sizeof( array ) };
	}


	template< Concepts::StandardLayout T, std::size_t size >
	constexpr Buffer< Mutable >
	make_buffer( T array[ size ] ) noexcept
	{
		// TODO: Do we need to consider overflow here?
		return { array, sizeof( array ) };
	}

	template< Concepts::StandardLayout T, std::size_t size >
	constexpr Buffer< Const >
	make_buffer( const T array[ size ] ) noexcept
	{
		// TODO: Do we need to consider overflow here?
		return { array, sizeof( array ) };
	}


	template< Concepts::StandardLayout T, std::size_t size >
	constexpr Buffer< Mutable >
	make_buffer( T array[ size ] ) noexcept
	{
		// TODO: Do we need to consider overflow here?
		return { array, sizeof( array ) };
	}

	template< Concepts::StandardLayout T, std::size_t size >
	constexpr Buffer< Const >
	make_buffer( const T array[ size ] ) noexcept
	{
		// TODO: Do we need to consider overflow here?
		return { array, sizeof( array ) };
	}


	inline Buffer< Mutable >
	make_buffer( std::string &string ) noexcept
	{
		return { string.data(), string.size() };
	}

	inline Buffer< Const >
	make_buffer( const std::string &string ) noexcept
	{
		return { string.data(), string.size() };
	}


	constexpr Buffer< Mutable >
	exports::copyData( const Buffer< Mutable > destination, const Buffer< Const > source )
	{
		if( source.size() > destination.size() ) throw InsufficientSizeError{ destination.data(), source.size(), destination.size(), typeid( std::byte ) };

		::memcpy( destination, source, source.size() );
		return { destination, source.size() };
	}

	constexpr void
	exports::zeroData( const Buffer< Mutable > buffer ) noexcept
	{
		::memset( buffer, 0, buffer.size() );
	}

	namespace exports
	{
		using detail::buffer::make_buffer;
	}
}

namespace Alepha::Cavorite::inline exports::inline buffer
{
	using namespace detail::buffer::exports;
}

/*
 * It is not possible to explicitly specialize `std::cbegin` and `std::cend` with differing results than what they
 * normally return (`decltype( std::as_const( range ).begin() )`), therefore the best we can do is just delete
 * them, in the interest of preserving as much correctness as we can.
 *
 * This really isn't a problem, anyway, as `cbegin` and `cend` are meant to be ADL-found aspect-functions, not
 * explictly called from `std::`, just like `swap`.
 *
 * Correct:
 * ```
 * using std::cbegin, std::cend;
 * std::sort( cbegin( list ), cend( list ) );
 * ```
 *
 * Incorrect:
 * ```
 * std::sort( std::cbegin( list ), std::cend( list ) );
 * ```
 *
 * Because of the below deletion and the above correct/incorrect examples, it really is not a problem that they're
 * deleted.  In fact, it's a good thing.  It will help catch incorrect usage in your code.
 */

template<>
constexpr auto
std::cbegin( const ::Alepha::Cavorite::Buffer< Alepha::Cavorite::Mutable > &range ) -> decltype( range.begin() )= delete;

template<>
constexpr auto
std::cend( const ::Alepha::Cavorite::Buffer< Alepha::Cavorite::Mutable > &range ) -> decltype( range.end() )= delete;
