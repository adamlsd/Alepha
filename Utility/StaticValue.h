static_assert( __cplusplus > 201700, "C++17 Required" );

#pragma once

namespace Alepha::Hydrogen::Utility
{
	inline namespace exports { inline namespace static_value {} }

	namespace detail::static_value
	{
		inline namespace exports {}

		template< typename T >
		struct default_init
		{
			T *operator()() const { return new T{}; }
		};

		namespace exports
		{
			template< typename T, typename Init= default_init< T > >
			struct StaticValue;
		}

		template< typename T, typename Init >
		struct exports::StaticValue
		{
			private:
				T *storage= nullptr;

			public:
				decltype( auto )
				get()
				{
					if( not storage ) storage= Init{}();
					return *storage;
				}

				decltype( auto ) operator ()() { return get(); }
		};
	}

	namespace exports::static_value
	{
		using namespace detail::static_value::exports;
	}
}
