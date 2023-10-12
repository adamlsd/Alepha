#include "../Attestation.h"

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


	class NotSorted : public std::exception
	{
		public:
			virtual const char *what() const noexcept final { return "Something wasn't sorted."; }
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

		static Sorted::Witness< const std::vector< int > & >
		check( std::vector< int > &v )
		{
			if( !std::is_sorted( begin( v ), end( v ) ) ) throw NotSorted{};
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

	inline namespace test_detail
	{
		std::vector< int > getData();
	}
}


int
main()
{
	//auto data= std::vector< int >( std::istream_iterator< int >( std::cin ),
			//std::istream_iterator< int >() );

	std::vector< int > data= getData();


	try
	{
		auto witness= Sorter::check( data );
		throw std::logic_error( "This should have thrown in `check`!" );
	}
	catch( const NotSorted & ) {}

	std::sort( begin( data ), end( data ) );

	auto witness= Sorter::check( data ); // This time it oughtn't throw.

	std::cout << std::boolalpha << binarySearch( witness, 42 ) << std::endl;
}


namespace
{
	std::vector< int >
	test_detail::getData()
	{
		return
		{
			49344,
			248,
			34529,
			12412,
			28387,
			52204,
			17330,
			64061,
			26865,
			53098,
			6254,
			24235,
			12433,
			5162,
			6967,
			22064,
			3150,
			37829,
			47719,
			15118,
			24956,
			7880,
			34712,
			57950,
			28045,
			58978,
			32835,
			41625,
			34921,
			4994,
			42600,
			21544,
			57838,
			48717,
			293,
			35508,
			13139,
			59576,
			2232,
			9298,
			65262,
			62943,
			32059,
			58746,
			8749,
			21421,
			20637,
			8188,
			15577,
			44297,
			55962,
			40634,
			58919,
			32798,
			34149,
			44650,
			53797,
			5154,
			65141,
			17675,
			7335,
			6778,
			44150,
			12318,
			59968,
			46993,
			35697,
			49319,
			56361,
			26640,
			40924,
			43750,
			43787,
			31692,
			65222,
			59041,
			20470,
			40424,
			52828,
			4730,
			3432,
			60223,
			44893,
			62286,
			34341,
			46247,
			56626,
			6411,
			38579,
			44396,
			64745,
			24651,
			21627,
			11543,
			44708,
			41595,
			40306,
			23704,
			41460,
			34339,
			51595,
			45262,
			36429,
			56661,
			25925,
			19652,
			54824,
			46212,
			12859,
			64783,
			47842,
			35101,
			52132,
			56040,
			25982,
			40335,
			48109,
			39422,
			21357,
			6038,
			27967,
			16297,
			30156,
			56378,
			44745,
			9223,
			11229,
			49482,
			17689,
			40163,
			22734,
			18030,
			50310,
			9113,
			25363,
			8367,
			16282,
			11922,
			4331,
			5890,
			19401,
			28942,
			37930,
			50699,
			57258,
			46439,
			10721,
			9926,
			54356,
			5157,
			19125,
			62868,
			7203,
			3141,
			8435,
			11667,
			27030,
			32321,
			49138,
			39657,
			33371,
			22733,
			59225,
			4257,
			35253,
			43775,
			44733,
			36009,
			58911,
			65521,
			2472,
			4557,
			23185,
			36930,
			15248,
			30805,
			23263,
			29451,
			32896,
			64592,
			26391,
			49382,
			59807,
			4328,
			60062,
			17463,
			17645,
			3516,
			25561,
			28430,
			32386,
			11974,
			13668,
			25827,
			45584,
			64900,
			20025,
			40000,
			50531,
			49065,
		};
	}
}
