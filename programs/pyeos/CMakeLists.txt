add_subdirectory(contracts)

if( UNIX AND NOT APPLE )
  set(rt_library rt )
endif()

if( "$ENV{PYTHON_INC_DIR}" STREQUAL "" )
  message( FATAL_ERROR "PYTHON_INC_DIR env not found." )
endif()

if( "$ENV{PYTHON_LIB}" STREQUAL "")
  message( FATAL_ERROR "PYTHON_LIB env not found." )
endif()

find_package( Gperftools QUIET )
if( GPERFTOOLS_FOUND )
    message( STATUS "Found gperftools; compiling steemd with TCMalloc")
    list( APPEND PLATFORM_SPECIFIC_LIBS tcmalloc )
endif()


add_library(python3.6m SHARED IMPORTED)
set_property(TARGET python3.6m PROPERTY IMPORTED_LOCATION $ENV{PYTHON_LIB})

add_custom_command(
    OUTPUT ${CMAKE_CURRENT_SOURCE_DIR}/wrap/eosapi.wrap.cpp
    COMMAND cython --cplus ${CMAKE_CURRENT_SOURCE_DIR}/cython/eosapi.pyx -o ${CMAKE_CURRENT_SOURCE_DIR}/wrap/eosapi.wrap.cpp
    DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/cython/eosapi.pyx
)

add_custom_command(
    OUTPUT ${CMAKE_CURRENT_SOURCE_DIR}/wrap/wallet.wrap.cpp
    COMMAND cython --cplus ${CMAKE_CURRENT_SOURCE_DIR}/cython/wallet.pyx -o ${CMAKE_CURRENT_SOURCE_DIR}/wrap/wallet.wrap.cpp
    DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/cython/wallet.pyx
)

add_custom_command(
    OUTPUT ${CMAKE_CURRENT_SOURCE_DIR}/wrap/eostypes.wrap.cpp
    COMMAND cython --cplus ${CMAKE_CURRENT_SOURCE_DIR}/cython/eostypes.pyx -o ${CMAKE_CURRENT_SOURCE_DIR}/wrap/eostypes.wrap.cpp
    DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/cython/eostypes.pyx
)

add_custom_command(
    OUTPUT ${CMAKE_CURRENT_SOURCE_DIR}/wrap/eoslib.wrap.cpp
    COMMAND cython --cplus ${CMAKE_CURRENT_SOURCE_DIR}/cython/eoslib.pyx -o ${CMAKE_CURRENT_SOURCE_DIR}/wrap/eoslib.wrap.cpp
    DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/cython/eoslib.pyx
)

add_custom_command(
    OUTPUT ${CMAKE_CURRENT_SOURCE_DIR}/wrap/pyobject.wrap.cpp
    COMMAND cython --cplus ${CMAKE_CURRENT_SOURCE_DIR}/cython/pyobject.pyx -o ${CMAKE_CURRENT_SOURCE_DIR}/wrap/pyobject.wrap.cpp
    DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/cython/pyobject.pyx
)

add_custom_command(
    OUTPUT ${CMAKE_CURRENT_SOURCE_DIR}/wrap/eostest.wrap.cpp
    COMMAND cython --cplus ${CMAKE_CURRENT_SOURCE_DIR}/cython/eostest.pyx -o ${CMAKE_CURRENT_SOURCE_DIR}/wrap/eostest.wrap.cpp
    DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/cython/eostest.pyx
)

add_custom_command(
    OUTPUT ${CMAKE_CURRENT_SOURCE_DIR}/wrap/database.wrap.cpp
    COMMAND cython --cplus ${CMAKE_CURRENT_SOURCE_DIR}/cython/database.pyx -o ${CMAKE_CURRENT_SOURCE_DIR}/wrap/database.wrap.cpp
    DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/cython/database.pyx
)

add_custom_command(
    OUTPUT ${CMAKE_CURRENT_SOURCE_DIR}/wrap/blockchain.wrap.cpp
    COMMAND cython --cplus ${CMAKE_CURRENT_SOURCE_DIR}/cython/blockchain.pyx -o ${CMAKE_CURRENT_SOURCE_DIR}/wrap/blockchain.wrap.cpp
    DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/cython/blockchain.pyx
)

add_executable( pyeos main.cpp py_plugin.cpp interface/json.cpp interface/eosapi_.cpp 
    interface/wallet_.cpp interface/pyobject.cpp interface/database_.cpp interface/eoslib_.cpp 
    interface/math_.cpp interface/eostest_.cpp interface/blockchain_.cpp interface/common.cpp
    interface/codelimit.c
    wrap/eoslib.wrap.cpp wrap/eosapi.wrap.cpp wrap/eostypes.wrap.cpp wrap/wallet.wrap.cpp 
    wrap/eostest.wrap.cpp wrap/database.wrap.cpp wrap/blockchain.wrap.cpp
    wrap/pyobject.wrap.cpp)

target_link_libraries( pyeos appbase account_history_api_plugin account_history_plugin chain_api_plugin 
    producer_plugin chain_plugin net_plugin http_plugin eos_chain fc ${CMAKE_DL_LIBS} ${PLATFORM_SPECIFIC_LIBS} 
    wallet_plugin wallet_api_plugin intl
    eos_native_contract eos_chain chainbase eos_utilities eos_egenesis_none fc
    python3.6m)

target_include_directories( pyeos PUBLIC ${CMAKE_SOURCE_DIR}/contracts 
                                  PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/include" $ENV{PYTHON_INC_DIR}
                                  PUBLIC  ${CMAKE_SOURCE_DIR}/plugins/wallet_plugin/include
                                  PUBLIC  ${CMAKE_SOURCE_DIR}/plugins/wallet_plugin/include
                                  PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/interface"
                                  )



install( TARGETS
   pyeos

   RUNTIME DESTINATION bin
   LIBRARY DESTINATION lib
   ARCHIVE DESTINATION lib
)
