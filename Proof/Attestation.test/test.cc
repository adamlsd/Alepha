#include "../Attestation.h"

#include <cstddef>
#include <cstdlib>
#include <algorithm>
#include <vector>
#include <mutex>
#include <memory>

#include <random>

namespace
{
	struct Tester;
	struct foobar_tag { using averant= Tester; };

	using FoobarFact= Alepha::Proof::Attestation< foobar_tag >;

	struct Tester
	{
		static void
		test()
		{
			FoobarFact token= Alepha::Proof::impute< FoobarFact >();
			FoobarFact token2= attest( FoobarFact::permission );
		}
	};

	struct Sorter;
	struct sorted_tag { using averant= Sorter; };

	using Sorted= Alepha::Proof::Attestation< sorted_tag >;

	struct Sorter
	{
		static Sorted::Witness< const std::vector< int > & >
		sort( std::vector< int > &v )
		{
			std::sort( begin( v ), end( v ) );
			return attest( Sorted::permission ).aver< const std::vector< int > & >( v );
		}
	};

	struct Locker;
	struct with_lock_tag { using averant= Locker; };

	using Locked= Alepha::Proof::Attestation< with_lock_tag >;

	struct Locker
	{
		struct WithLock : Locked
		{
			public:
				WithLock( const std::lock_guard< std::mutex > & )
						: Locked( attest( Locked::permission ) ) {}

				WithLock( std::lock_guard< std::mutex > && )= delete;
		};
	};

	using WithLock= Locker::WithLock;

	struct NullChecker;
	struct non_null_tag { using averant= NullChecker; };

	using NonNull= Alepha::Proof::Attestation< non_null_tag >;

	struct NullChecker
	{
		template< typename Ptype >
		static NonNull::Witness< Ptype >
		isNotNull( Ptype p )
		{
			if( !p ) throw std::logic_error( "" );
			return attest( NonNull::permission ).averCopy( std::move( p ) );
		}
	};

	template< typename P >
	auto
	makeUnique()
	{
		return NullChecker::isNotNull( std::make_unique< P >() );
	}

	// Page aligned example

	struct page_aligned_tag { using averant= struct AlignmentChecker; };
	using PageAligned= Alepha::Proof::Attestation< page_aligned_tag >;

	struct AlignmentChecker
	{
		template< typename Ptype >
		static auto
		check( Ptype *pointer )
		{
			if( pointer % 4096 ) throw std::runtime_error( "" );
			return attest( PageAligned::permission ).template averCopy< Ptype *const >( pointer );
		}

		static PageAligned::Witness< void *const >
		allocate( const std::size_t amt )
		{
			void *rv;
			const int ec= posix_memalign( &rv, 4096, amt );
			if( ec ) std::get_new_handler()();
			return attest( PageAligned::permission ).averCopy< void *const >( rv );
		}
	};


	template< typename T >
	PageAligned::Witness< std::unique_ptr< T > >
	make_unique_on_page()
	{
		// Dtor ordering and transition of responsibility struct
		struct Safety
		{
			std::unique_ptr< void > memory;

			explicit Safety( void *p ) : memory( p ) { new (p) T(); }

			~Safety()= delete;

			std::unique_ptr< T >
			finish () & noexcept
			{
				return std::unique_ptr< T >( reinterpret_cast< T * >( memory.release() ) );
			}
		};

		auto safe_adapt= []( auto p )
		{
			std::array< std::uint8_t, sizeof( Safety ) > locals;
			auto safe= (new (locals.data()) Safety( witness( p ) ))->finish();
			return std::make_pair( std::move( safe ), fact( p ) );
		};

		auto [ ptr, alignment ]= safe_adapt( AlignmentChecker::allocate( sizeof( T ) ) );

		return alignment.template averCopy< std::unique_ptr< T > >( std::move( ptr ) );
	}
}


int
main()
{
	Tester::test();
	//attest( FoobarFact::permission );
	FoobarFact token= Alepha::Proof::impute< FoobarFact >();

	std::vector< int > v{ 5, 11, 3, 2, 1 };
	Sorted::Witness< const std::vector< int > & > sortedVector= Sorter::sort( v );
	assert( std::is_sorted( begin( testify( sortedVector ) ), end( testify( sortedVector ) ) ) );

	std::shuffle( begin( v ), end( v ), std::random_device() );

	assert( !std::is_sorted( begin( testify( sortedVector ) ), end( testify( sortedVector ) ) ) );


	std::mutex mtx;

	auto checker= []( WithLock ) {};

	std::lock_guard< std::mutex > lk( mtx );
	checker( lk );

	auto myInt= makeUnique< int >();
	return EXIT_SUCCESS;
}



namespace
{
	namespace mongo
	{
		struct OperationContext { int member; };
		void
		operation( NonNull::Witness< OperationContext * > opCtx )
		{
			// ...
			(void) testify( opCtx )->member;
		}
	}
}
