#include "Attestation.h"
#include <iostream>
#include <iomanip>
#include <iterator>
#include <vector>
#include <algorithm>
#include <mutex>
#include <exception>

// Ignore the headers, for the moment, just assume that I have included what I need.


namespace
{
	// An initial way you might do things...
	class Thing0
	{
		private:
			std::mutex mtx;

			void helper_needs_lock() {}


		public:
			void
			function()
			{
				std::lock_guard< std::mutex > lock( mtx );
				helper_needs_lock();
			}
	};


	// A better way you might do it.
	class Thing1
	{
		private:
			std::mutex mtx;

			void helper( const std::lock_guard< std::mutex > & ) {}


		public:
			void
			function()
			{
				std::lock_guard< std::mutex > lock( mtx );
				helper( lock );
			}
	};



	// A more adaptable way you might do it.
	class WithLock
	{
		public:
			WithLock( const std::lock_guard< std::mutex > & ) {}
			WithLock( const std::unique_lock< std::mutex > &lock ) { assert( lock.owns_lock() ); }
	};

	class Thing2
	{
		private:
			std::mutex mtx;

			void helper( WithLock ) {}


		public:
			void
			function()
			{
				std::lock_guard< std::mutex > lock( mtx );
				helper( lock );
			}
	};








	// How might we generalize this?





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

		static Sorted::Witness< const std::vector< int > & >
		check( std::vector< int > &v )
		{
			if( !std::is_sorted( begin( v ), end( v ) ) ) throw std::runtime_error( "" );
			return attest( Sorted::permission ).aver< const std::vector< int > & >( v );
		}
	};
}

namespace // Other guy
{
	bool
	binarySearch( Sorted::Witness< const std::vector< int > & > v, const int data )
	{
		return std::binary_search( begin( testify( v ) ), end( testify( v ) ), data );
	}
}


int
main()
{
	auto data= std::vector< int >( std::istream_iterator< int >( std::cin ),
			std::istream_iterator< int >() );
	auto witness= Sorter::check( data );

	std::cout << std::boolalpha << binarySearch( witness, 42 ) << std::endl;
}

