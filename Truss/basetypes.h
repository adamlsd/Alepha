static_assert( __cplusplus > 2020'00 );

#pragma once

#include <Alepha/Alepha.h>


namespace Alepha
{
	namespace Truss
	{
		namespace types
		{
			using nullptr_t= decltype( nullptr );
			using size_t= decltype( sizeof( 0 ) );
		}

		namespace detail
		{
			template< unsigned char v > struct count_one_bits_unsigned_char;

			template<>
			struct count_one_bits_unsigned_char< 0 >
			{
				static const types::size_t value= 0;
			};

			template< unsigned char v >
			struct count_one_bits_unsigned_char
			{
				static const types::size_t value= ( ( v & 0x1 ) ? 1 : 0 )
						+ count_one_bits_unsigned_char< ( v >> 1 ) >::value;
			};

			const unsigned char zero= 0;
			const unsigned char max= zero - 1;

			const types::size_t platform_char_bits= count_one_bits_unsigned_char< max >::value;

			class uint24_t
			{
				private:
					std::uint32_t value:24;
			};
		}
	}
}
