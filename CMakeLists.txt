cmake_minimum_required( VERSION 3.25 )
project( Alepha )

include( cmake/rules.cmake )



# The core alepha library:

add_library( alepha SHARED
	Console.cpp
	ProgramOptions.cpp
	string_algorithms.cpp
	word_wrap.cpp
)
# Everything else depends upon it
link_libraries( alepha )

# The subdirs to build
add_subdirectory( Meta )
add_subdirectory( Proof )
add_subdirectory( Reflection )
add_subdirectory( Testing )

# The local subdir tests to build
add_subdirectory( AutoRAII.test )
add_subdirectory( comparisons.test )
add_subdirectory( Exception.test )
add_subdirectory( word_wrap.test )

# Sample applications
add_executable( example example.cc )
