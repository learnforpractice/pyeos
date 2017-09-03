#add_executable( pyeos main.cpp )
file(GLOB COMMON_SOURCES "common/*.cpp")

if( UNIX AND NOT APPLE )
  set(rt_library rt )
endif()

find_package( Gperftools QUIET )
if( GPERFTOOLS_FOUND )
    message( STATUS "Found gperftools; compiling steemd with TCMalloc")
    list( APPEND PLATFORM_SPECIFIC_LIBS tcmalloc )
endif()


add_library(python3.6m SHARED IMPORTED)
set_property(TARGET python3.6m PROPERTY IMPORTED_LOCATION "/Users/newworld/anaconda/lib/libpython3.6m.so")

file(GLOB UNIT_TESTS "tests/*.cpp")
add_executable( pyeos main.cpp api.cpp py_plugin.cpp eosapi_.cpp eostypes_.cpp ${UNIT_TESTS} ${COMMON_SOURCES} )

target_link_libraries( pyeos appbase account_history_api_plugin account_history_plugin chain_api_plugin producer_plugin chain_plugin net_plugin http_plugin eos_chain fc ${CMAKE_DL_LIBS} ${PLATFORM_SPECIFIC_LIBS}
eos_native_contract eos_chain chainbase eos_utilities eos_egenesis_none eos_wallet fc ${PLATFORM_SPECIFIC_LIBS} python3.6m boost_python3-mt)

target_include_directories( pyeos PUBLIC ${CMAKE_SOURCE_DIR}/contracts )
target_include_directories( pyeos PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/include" "/Users/newworld/anaconda/include/python3.6m")


install( TARGETS
   pyeos

   RUNTIME DESTINATION bin
   LIBRARY DESTINATION lib
   ARCHIVE DESTINATION lib
)
