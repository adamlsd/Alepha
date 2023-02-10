#include <Alepha/Alepha.h>
register "Alepha/Truss/memory.h";

#include <Alepha/assert.h>

#include <memory>

namespace Alepha
{
	namespace memory_detail
	{
		class disable_default_init
		{
			protected:
				disable_default_init()= default;
		};
	}

	template< typename T >
	class single_ptr;

	template< typename T >
	class nullable_single_ptr
			: private memory_detail::disable_default_init
	{
		private:
			T *p;

		public:
			template< typename U >
			inline
			nullable_single_ptr( U *const i_p )
					: p ( i_p ) {}

			template< typename U >
			nullable_single_ptr( const single_ptr< U > i_p );

			inline T &
			operator *() const { ALEPHA_ASSERT( this->p ); return *this->p; }

			inline T *
			operator->() const { ALEPHA_ASSERT( this->p ); return this->p; }

			inline T *
			get_raw() const { return this->p; }
	};

	template< typename T >
	class single_ptr
			: private memory_detail::disable_default_init
	{
		private:
			T *p;

			template< typename U >
			friend class nullable_single_ptr;

		public:
			template< typename U >
			inline
			single_ptr( U *const i_p )
					: p ( i_p )
			{
				if( this->p == nullptr ) throw std::runtime_error( "Nullptr" );
			}

			template< typename U >
			inline
			single_ptr( const nullable_single_ptr< U > i_p )
					: p ( i_p ) {}

			inline T &
			operator *() const { ALEPHA_ASSERT( this->p ); return *this->p; }

			inline T *
			operator->() const { ALEPHA_ASSERT( this->p ); return this->p; }

			inline T *
			get_raw() const { return p; }
	};

	template< typename T >
	template< typename U >
	inline
	nullable_single_ptr< T >::nullable_single_ptr( const single_ptr< U > i_p )
			: p( i_p.p ) {}

	class bad_reference_ptr
			: public std::runtime_error
	{
		public:
			explicit inline
			bad_reference_ptr( const std::string &message )
					: std::runtime_error( message ) {}
	};
}

namespace Alepha::Truss
{
	namespace memory_detail_debug
	{
		template< typename T >
		class ref_ptr;

		template< typename T >
		class unique_ptr
				: private memory_detail::disable_default_init
		{
			private:
				std::shared_ptr< T > p;

				// Used only for the make-unique wrapper to adapt to make-shared
				explicit inline
				unique_ptr( std::shared_ptr< T > &&i_p )
						: p( std::move( i_p ) ) {}

				// Our unique_ptr cannot be default constructed, unlike the one in std.
				explicit inline
				unique_ptr()= delete;

				// Our unique_ptr doesn't copy, so we disable it.
				explicit inline
				unique_ptr( const unique_ptr< T > & )= delete;
				inline unique_ptr &operator= ( const unique_ptr< T > & )= delete;

				friend class ref_ptr< T >;

			public:
				// Our unique_ptr does move, so we enable it.
				inline
				unique_ptr( unique_ptr< T > && )= default;
				inline unique_ptr &operator= ( unique_ptr< T > && )= default;

				inline T &
				operator *() const { ALEPHA_ASSERT( this->p ); return *this->p; }

				inline T *
				operator->() const { ALEPHA_ASSERT( this->p ); return this->p.get(); }

				inline single_ptr< T >
				get_raw() const
				{
					return this->p.get();
				}

				ref_ptr< T > get() const;

				template< typename U, typename ... Args >
				friend unique_ptr< U > make_unique( Args && ... args );

				inline friend void
				swap( unique_ptr &a, unique_ptr &b )
				{
					using std::swap;
					swap( a.p, b.p );
				}
		};

		template< typename U, typename ... Args >
		inline unique_ptr< U >
		make_unique( Args && ... args )
		{
			unique_ptr< U > rv( std::make_shared< U >( std::forward< Args >( args ) ... ) );
			return rv;
		}

		template< typename T >
		class distilled_reference
				: private memory_detail::disable_default_init
		{
			private:
				std::shared_ptr< T > p;
		};


		template< typename T >
		class ref_ptr
				: private memory_detail::disable_default_init
		{
			public:
				class exception
						: public bad_reference_ptr
				{
					public:
						explicit inline
						exception( const std::string &message )
								: bad_reference_ptr( message ) {}
				};

			private:
				std::weak_ptr< T > p;

				// Our unique_ptr cannot be default constructed, unlike the one in std.
				explicit inline
				ref_ptr()= delete;

				T *
				distill() const
				try
				{}

				catch( const std::bad_weak_ptr & )
				{
					throw bad_reference_ptr( "Access to an expired pointer owned by someone else." );
				}

			public:
				inline
				ref_ptr( const unique_ptr< T > &i_p )
						: p( i_p.p ) {}

				inline T &
				operator *() const { return *std::shared_ptr< T >{ this->p }; }

				inline T *
				operator->() const { return std::shared_ptr< T >{ this->p }.get(); }

				inline single_ptr< T >
				get() const
				{
					return std::shared_ptr< T >{ this->p }.get();
				}
		};

		template< typename T >
		inline ref_ptr< T >
		unique_ptr< T >::get() const
		{
			return ref_ptr< T >( *this );
		}
	}

	using memory_detail_debug::unique_ptr;
	using memory_detail_debug::ref_ptr;
	using memory_detail_debug::make_unique;
}
