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

add_custom_command(
    OUTPUT ${CMAKE_CURRENT_SOURCE_DIR}/eosapi.wrap.cpp
    COMMAND cython --cplus ${CMAKE_CURRENT_SOURCE_DIR}/eosapi.pyx -o ${CMAKE_CURRENT_SOURCE_DIR}/eosapi.wrap.cpp
    DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/eosapi.pyx
)

add_custom_command(
    OUTPUT ${CMAKE_CURRENT_SOURCE_DIR}/wallet.wrap.cpp
    COMMAND cython --cplus ${CMAKE_CURRENT_SOURCE_DIR}/wallet.pyx -o ${CMAKE_CURRENT_SOURCE_DIR}/wallet.wrap.cpp
    DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/wallet.pyx
)

add_custom_command(
    OUTPUT ${CMAKE_CURRENT_SOURCE_DIR}/eostypes.wrap.cpp
    COMMAND cython --cplus ${CMAKE_CURRENT_SOURCE_DIR}/eostypes_.pyx -o ${CMAKE_CURRENT_SOURCE_DIR}/eostypes.wrap.cpp
    DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/eostypes_.pyx
)


file(GLOB UNIT_TESTS "tests/*.cpp")
add_executable( pyeos main.cpp api.cpp py_plugin.cpp eosapi.cpp 
eosapi.wrap.cpp eostypes.wrap.cpp wallet.wrap.cpp wallet_.cpp pyobject.cpp ${UNIT_TESTS} ${COMMON_SOURCES} )

target_link_libraries( pyeos appbase account_history_api_plugin account_history_plugin chain_api_plugin 
    producer_plugin chain_plugin net_plugin http_plugin eos_chain fc ${CMAKE_DL_LIBS} ${PLATFORM_SPECIFIC_LIBS} 
    wallet_plugin wallet_api_plugin
    eos_native_contract eos_chain chainbase eos_utilities eos_egenesis_none fc
    python3.6m boost_python3-mt)

target_include_directories( pyeos PUBLIC ${CMAKE_SOURCE_DIR}/contracts 
                                  PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/include" "/Users/newworld/anaconda/include/python3.6m"
                                  PUBLIC  ${CMAKE_SOURCE_DIR}/plugins/wallet_plugin/include
                                  PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/include"
                                  )



install( TARGETS
   pyeos

   RUNTIME DESTINATION bin
   LIBRARY DESTINATION lib
   ARCHIVE DESTINATION lib
)
