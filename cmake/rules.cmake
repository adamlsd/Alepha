#cmake_policy( SET CMP0002 OLD )

add_compile_options( -I ${CMAKE_BINARY_DIR}  ; -std=c++20 ; -I . )
add_compile_options( -fdiagnostics-column-unit=byte )


list( APPEND CMAKE_CTEST_ARGUMENTS "-VV" )


set( VERBOSE 1 )
set( CMAKE_VERBOSE_MAKEFILE true )

file( CREATE_LINK ${CMAKE_SOURCE_DIR} Alepha SYMBOLIC )


include(CTest)


function( unit_test TEST_NAME )

get_filename_component( TEST_DOMAIN ${CMAKE_CURRENT_SOURCE_DIR} NAME )
set( FULL_TEST_NAME ${TEST_DOMAIN}.${TEST_NAME} )

add_executable( ${FULL_TEST_NAME} ${TEST_NAME}.cc )
add_test( ${FULL_TEST_NAME} ${FULL_TEST_NAME} )

endfunction( unit_test )

