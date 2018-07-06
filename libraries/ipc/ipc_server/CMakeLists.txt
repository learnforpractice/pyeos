## SORT .cpp by most likely to change / break compile
add_library( ipc_server SHARED
    ipc_server.cpp
    ipc_interface.cpp
    ${CMAKE_SOURCE_DIR}/libraries/ipc/thrift/RpcService.cpp
    ${CMAKE_SOURCE_DIR}/libraries/ipc/thrift/RpcService_types.cpp
)


if ("${CMAKE_BUILD_TYPE}" STREQUAL "Debug")
   target_compile_options(ipc_server PRIVATE -DDEBUG)
endif()

target_link_libraries( ipc_server PUBLIC db_api fc thrift_static eosiolib_native ${OPENSSL_LIBRARIES})

target_include_directories( ipc_server
                            PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}"
                            PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/include"
                            PUBLIC "${CMAKE_CURRENT_BINARY_DIR}/include"
                            PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/../wasm-jit/Include"
                            PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/../../externals/binaryen/src"
                            PUBLIC ${CMAKE_SOURCE_DIR}/libraries/chain/include
                            PUBLIC ${CMAKE_SOURCE_DIR}/contracts
                            PUBLIC ${CMAKE_SOURCE_DIR}/contracts/eosiolib_native
                            PUBLIC ${CMAKE_SOURCE_DIR}/libraries/chainbase/include
                            PUBLIC ${CMAKE_SOURCE_DIR}/externals/thrift/src
                            PUBLIC ${CMAKE_SOURCE_DIR}/libraries/ipc
                            PUBLIC ${CMAKE_SOURCE_DIR}/libraries/vm_py
                            PUBLIC ${CMAKE_SOURCE_DIR}/libraries/vm_py/py
                            PUBLIC ${CMAKE_SOURCE_DIR}/libraries/vm_wasm
                            )

install( TARGETS ipc_server 
   RUNTIME DESTINATION ${CMAKE_INSTALL_FULL_BINDIR}
   LIBRARY DESTINATION ${CMAKE_INSTALL_FULL_LIBDIR}
   ARCHIVE DESTINATION ${CMAKE_INSTALL_FULL_LIBDIR}
#   PUBLIC_HEADER DESTINATION ${CMAKE_INSTALL_FULL_INCLUDEDIR}/eosio/chain
)
#install_directory_permissions( DIRECTORY ${CMAKE_INSTALL_FULL_INCLUDEDIR}/eosio/chain )
