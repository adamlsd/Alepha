#cmake_policy( SET CMP0002 OLD )

add_compile_options( -std=c++20 )
add_compile_options( -fdiagnostics-column-unit=byte )
include_directories( ${CMAKE_BINARY_DIR} . )


list( APPEND CMAKE_CTEST_ARGUMENTS "-VV" )


set( VERBOSE 1 )
set( CMAKE_VERBOSE_MAKEFILE true )

file( CREATE_LINK ${CMAKE_SOURCE_DIR} Alepha SYMBOLIC )

if( DEFINED ALEPHA_BOOST_PATH )
file( CREATE_LINK ${CMAKE_BINARY_DIR}/${ALEPHA_BOOST_PATH} boost SYMBOLIC )
include_directories( ${CMAKE_BINARY_DIR}/${ALEPHA_BOOST_PATH}/.. )
endif()


include(CTest)


function( unit_test TEST_NAME )

get_filename_component( TEST_DOMAIN ${CMAKE_CURRENT_SOURCE_DIR} NAME )
set( FULL_TEST_NAME ${TEST_DOMAIN}.${TEST_NAME} )

add_executable( ${FULL_TEST_NAME} ${TEST_NAME}.cc )
add_test( ${FULL_TEST_NAME} ${FULL_TEST_NAME} )

endfunction( unit_test )

