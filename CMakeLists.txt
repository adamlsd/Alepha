cmake_minimum_required( VERSION 3.19 )
project( Alepha )

include( cmake/rules.cmake )



# The core alepha library:

add_library( alepha SHARED
	Console.cc
	ProgramOptions.cc
	string_algorithms.cc
	word_wrap.cc
)
# Everything else depends upon it
link_libraries( alepha )

# The subdirs to build
add_subdirectory( Meta )
add_subdirectory( Proof )
add_subdirectory( IOStreams )
add_subdirectory( Reflection )
add_subdirectory( Testing )
add_subdirectory( Utility )

# The local subdir tests to build
add_subdirectory( AutoRAII.test )
add_subdirectory( comparisons.test )
add_subdirectory( Exception.test )
add_subdirectory( word_wrap.test )
add_subdirectory( string_algorithms.test )
add_subdirectory( tuplize_args.test )

# Sample applications
add_executable( example example.cc )
