add_subdirectory(contracts)
add_subdirectory(tools)
#add_subdirectory(tests)

if( UNIX AND NOT APPLE )
  set(rt_library rt )
endif()

find_package( Gperftools QUIET )
if( GPERFTOOLS_FOUND )
    message( STATUS "Found gperftools; compiling steemd with TCMalloc")
    list( APPEND PLATFORM_SPECIFIC_LIBS tcmalloc )
endif()


#set_property(TARGET python3.6m PROPERTY IMPORTED_LOCATION $ENV{PYTHON_LIB})

set(PYTHON3 "${CMAKE_SOURCE_DIR}/libraries/python/dist/bin/python3")

add_custom_command(
    OUTPUT ${CMAKE_CURRENT_SOURCE_DIR}/wrap/eosapi.wrap.cpp
    COMMAND echo ${PYTHON3} -m cython --cplus ${CMAKE_CURRENT_SOURCE_DIR}/cython/eosapi.pyx -o ${CMAKE_CURRENT_SOURCE_DIR}/wrap/eosapi.wrap.cpp
    COMMAND ${PYTHON3} -m cython --cplus ${CMAKE_CURRENT_SOURCE_DIR}/cython/eosapi.pyx -o ${CMAKE_CURRENT_SOURCE_DIR}/wrap/eosapi.wrap.cpp
    DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/cython/eosapi.pyx ${CMAKE_CURRENT_SOURCE_DIR}/cython/eostypes_.pxd
)

add_custom_command(
    OUTPUT ${CMAKE_CURRENT_SOURCE_DIR}/wrap/wallet.wrap.cpp
    COMMAND ${PYTHON3} -m cython --cplus ${CMAKE_CURRENT_SOURCE_DIR}/cython/wallet.pyx -o ${CMAKE_CURRENT_SOURCE_DIR}/wrap/wallet.wrap.cpp
    DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/cython/wallet.pyx ${CMAKE_CURRENT_SOURCE_DIR}/cython/eostypes_.pxd
)

add_custom_command(
    OUTPUT ${CMAKE_CURRENT_SOURCE_DIR}/wrap/pyobject.wrap.cpp
    COMMAND ${PYTHON3} -m cython --cplus ${CMAKE_CURRENT_SOURCE_DIR}/cython/pyobject.pyx -o ${CMAKE_CURRENT_SOURCE_DIR}/wrap/pyobject.wrap.cpp
    DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/cython/pyobject.pyx
)

add_custom_command(
    OUTPUT ${CMAKE_CURRENT_SOURCE_DIR}/wrap/database.wrap.cpp
    COMMAND ${PYTHON3} -m cython --cplus ${CMAKE_CURRENT_SOURCE_DIR}/cython/database.pyx -o ${CMAKE_CURRENT_SOURCE_DIR}/wrap/database.wrap.cpp
    DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/cython/database.pyx ${CMAKE_CURRENT_SOURCE_DIR}/cython/eostypes_.pxd ${CMAKE_CURRENT_SOURCE_DIR}/cython/database_object.pxd
)

add_custom_command(
    OUTPUT ${CMAKE_CURRENT_SOURCE_DIR}/wrap/eoslib.wrap.cpp
    COMMAND ${PYTHON3} -m cython --cplus ${CMAKE_CURRENT_SOURCE_DIR}/cython/eoslib.pyx -o ${CMAKE_CURRENT_SOURCE_DIR}/wrap/eoslib.wrap.cpp
    DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/cython/eoslib.pyx
)

add_custom_command(
    OUTPUT ${CMAKE_CURRENT_SOURCE_DIR}/wrap/ipc.wrap.cpp
    COMMAND ${PYTHON3} -m cython --cplus ${CMAKE_CURRENT_SOURCE_DIR}/cython/ipc.pyx -o ${CMAKE_CURRENT_SOURCE_DIR}/wrap/ipc.wrap.cpp
    DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/cython/ipc.pyx
)

add_custom_command(
    OUTPUT ${CMAKE_CURRENT_SOURCE_DIR}/wrap/blockchain.wrap.cpp
    COMMAND ${PYTHON3} -m cython --cplus ${CMAKE_CURRENT_SOURCE_DIR}/cython/blockchain.pyx -o ${CMAKE_CURRENT_SOURCE_DIR}/wrap/blockchain.wrap.cpp
    DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/cython/blockchain.pyx ${CMAKE_CURRENT_SOURCE_DIR}/cython/eostypes_.pxd
)

add_custom_command(
    OUTPUT ${CMAKE_CURRENT_SOURCE_DIR}/wrap/eostypes.wrap.cpp
    COMMAND ${PYTHON3} -m cython --cplus ${CMAKE_CURRENT_SOURCE_DIR}/cython/eostypes.pyx -o ${CMAKE_CURRENT_SOURCE_DIR}/wrap/eostypes.wrap.cpp
    DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/cython/eostypes.pyx ${CMAKE_CURRENT_SOURCE_DIR}/cython/eostypes_.pxd
)


add_custom_command(
    OUTPUT ${CMAKE_CURRENT_SOURCE_DIR}/wrap/util.wrap.cpp
    COMMAND ${PYTHON3} -m cython --cplus ${CMAKE_CURRENT_SOURCE_DIR}/cython/util.pyx -o ${CMAKE_CURRENT_SOURCE_DIR}/wrap/util.wrap.cpp
    DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/cython/util.pyx
)

add_custom_command(
    OUTPUT ${CMAKE_CURRENT_SOURCE_DIR}/wrap/debug.wrap.cpp
    COMMAND ${PYTHON3} -m cython --cplus ${CMAKE_CURRENT_SOURCE_DIR}/cython/debug.pyx -o ${CMAKE_CURRENT_SOURCE_DIR}/wrap/debug.wrap.cpp
    DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/cython/debug.pyx
)

add_library(pyeos-shared eos_main.cpp py_plugin.cpp interface/json.cpp interface/eosapi_.cpp 
    interface/wallet_.cpp interface/pyobject.cpp interface/database_.cpp
    interface/math_.cpp  
    interface/common.cpp
    interface/debug_.cpp
    interface/debug_.cpp
    interface/eoslib_.cpp
    interface/wasm_.cpp
    interface/ipc_.cpp
#    interface/httpc.cpp
#    interface/codelimit.c 
    wrap/ipc.wrap.cpp 
    wrap/eosapi.wrap.cpp 
    wrap/database.wrap.cpp 
    wrap/eoslib.wrap.cpp 
    wrap/wallet.wrap.cpp
    wrap/pyobject.wrap.cpp
    wrap/debug.wrap.cpp)


if( APPLE )
target_link_libraries( pyeos-shared appbase chain_api_plugin 
    producer_plugin chain_plugin net_plugin http_plugin fc ${CMAKE_DL_LIBS} ${PLATFORM_SPECIFIC_LIBS} 
    wallet_plugin wallet_api_plugin
    chainbase eos_utilities fc
    python3 eosio_chain intl )
else( APPLE )
target_link_libraries( pyeos-shared appbase chain_api_plugin 
    producer_plugin chain_plugin net_plugin http_plugin fc ${CMAKE_DL_LIBS} ${PLATFORM_SPECIFIC_LIBS} 
    wallet_plugin wallet_api_plugin
    chainbase eos_utilities fc
    python3 eosio_chain util -Xlinker -export-dynamic)
endif( APPLE )

target_include_directories( pyeos-shared PUBLIC ${CMAKE_SOURCE_DIR}/contracts 
                                  PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/include"
                                  PUBLIC ${CMAKE_SOURCE_DIR}/libraries/python/Include
                                  PUBLIC ${CMAKE_SOURCE_DIR}/libraries/python
                                  PUBLIC ${CMAKE_SOURCE_DIR}/libraries/chain
                                  PUBLIC  ${CMAKE_SOURCE_DIR}/plugins/wallet_plugin/include
                                  PUBLIC  ${CMAKE_SOURCE_DIR}/plugins/wallet_plugin/include
                                  PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/interface"
                                  PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/util"
                                  PRIVATE "${CMAKE_SOURCE_DIR}/plugins/account_history_api_plugin/include"
                                  PRIVATE "${CPP_ETHEREUM_DIR}"
                                  )


add_executable(pyeos main.cpp)

target_link_libraries( pyeos pyeos-shared )


install( TARGETS
   pyeos

   RUNTIME DESTINATION bin
   LIBRARY DESTINATION lib
   ARCHIVE DESTINATION lib
)
