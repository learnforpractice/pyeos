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

#set(PYTHON3 "${CMAKE_SOURCE_DIR}/libraries/python/dist/bin/python3")
set(PYTHON3 "LD_LIBRARY_PATH=${CMAKE_SOURCE_DIR}/libraries/python;${CMAKE_SOURCE_DIR}/libraries/python/dist/bin/python3")


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

add_custom_command(
    OUTPUT ${CMAKE_CURRENT_SOURCE_DIR}/wrap/db.wrap.cpp
    COMMAND ${PYTHON3} -m cython --cplus ${CMAKE_CURRENT_SOURCE_DIR}/cython/db.pyx -o ${CMAKE_CURRENT_SOURCE_DIR}/wrap/db.wrap.cpp
    DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/cython/db.pyx
)

add_custom_command(
    OUTPUT ${CMAKE_CURRENT_SOURCE_DIR}/wrap/net.wrap.cpp
    COMMAND ${PYTHON3} -m cython --cplus ${CMAKE_CURRENT_SOURCE_DIR}/cython/net.pyx -o ${CMAKE_CURRENT_SOURCE_DIR}/wrap/net.wrap.cpp
    DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/cython/net.pyx
)

add_custom_command(
    OUTPUT ${CMAKE_CURRENT_SOURCE_DIR}/debugging/eoslib.wrap.cpp
    COMMAND ${PYTHON3} -m cython --cplus ${CMAKE_CURRENT_SOURCE_DIR}/debugging/eoslib.pyx -o ${CMAKE_CURRENT_SOURCE_DIR}/debugging/eoslib.wrap.cpp
    DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/debugging/eoslib.pyx
)


add_library(pyeos-shared eos_main.cpp interface/json.cpp interface/eosapi_.cpp 
    interface/wallet_.cpp interface/pyobject.cpp
    interface/math_.cpp  
    interface/debug_.cpp
#    interface/wasm_.cpp
    interface/net_.cpp

    wrap/eosapi.wrap.cpp 
    wrap/wallet.wrap.cpp
    wrap/pyobject.wrap.cpp
    wrap/db.wrap.cpp
    wrap/debug.wrap.cpp
    wrap/net.wrap.cpp

    debugging/eoslib.wrap.cpp
    debugging/eoslib_.cpp

    )



    
if( APPLE )
target_link_libraries( pyeos-shared eosio_chain appbase softfloat db_api
    chainbase eos_utilities fc ${CMAKE_DL_LIBS} ${PLATFORM_SPECIFIC_LIBS} 
    python3  intl )
else( APPLE )
target_link_libraries( pyeos-shared eosio_chain appbase softfloat db_api
    chainbase eos_utilities fc ${CMAKE_DL_LIBS} ${PLATFORM_SPECIFIC_LIBS} 
    python3 util  -Xlinker -export-dynamic)
endif( APPLE )

target_include_directories( pyeos-shared 
                              PUBLIC ${CMAKE_SOURCE_DIR}/libraries/ethereum
                              PUBLIC ${CMAKE_SOURCE_DIR}/contracts 
                              PUBLIC ${CMAKE_SOURCE_DIR}/libraries/chain
                              PUBLIC ${CMAKE_SOURCE_DIR}/libraries/chain/include
                              PUBLIC ${CMAKE_BINARY_DIR}//libraries/chain/include
                              PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include
                              PUBLIC ${CMAKE_SOURCE_DIR}/libraries/python
                              PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/interface
                              PUBLIC ${CMAKE_SOURCE_DIR}/libraries/vm_py
                              PUBLIC ${CMAKE_SOURCE_DIR}/libraries/vm_wasm
                              PUBLIC ${CMAKE_SOURCE_DIR}/libraries/vm_py/ports/unix
                              PUBLIC ${CMAKE_SOURCE_DIR}/libraries/vm_py/ports/unix/build
                              PUBLIC ${CMAKE_SOURCE_DIR}/libraries/python/Include
                              PUBLIC ${CMAKE_SOURCE_DIR}/plugins/wallet_plugin/include
                              PUBLIC ${CMAKE_SOURCE_DIR}/plugins/wallet_plugin/include
                              PRIVATE ${CMAKE_SOURCE_DIR}/plugins/history_plugin/include
                              PRIVATE ${CMAKE_SOURCE_DIR}/plugins/history_api_plugin/include
                              PRIVATE ${CMAKE_SOURCE_DIR}/plugins/net_plugin/include
                              PRIVATE ${CMAKE_SOURCE_DIR}/plugins/chain_plugin/include
                              PRIVATE ${CMAKE_SOURCE_DIR}/plugins/chain_interface/include
                              PRIVATE ${CMAKE_SOURCE_DIR}/plugins/chain_api_plugin/include
                              PRIVATE ${CMAKE_SOURCE_DIR}/plugins/http_plugin/include
                              PRIVATE ${CMAKE_SOURCE_DIR}/plugins/producer_plugin/include
                              PRIVATE ${CMAKE_SOURCE_DIR}/plugins/http_client_plugin/include
                              PRIVATE ${CMAKE_BINARY_DIR}/programs/nodeos
                           )


add_executable(pyeos main.cpp)

target_link_libraries( pyeos pyeos-shared )

add_dependencies(pyeos nodeos)


install( TARGETS
   pyeos

   RUNTIME DESTINATION bin
   LIBRARY DESTINATION lib
   ARCHIVE DESTINATION lib
)
