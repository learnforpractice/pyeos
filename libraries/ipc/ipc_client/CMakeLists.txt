## SORT .cpp by most likely to change / break compile
add_library( ipc_client SHARED
    Client.cpp
    vm_api.cpp
    ipc_manager.cpp
    thrift/RpcService.cpp
    thrift/RpcService_types.cpp
)


if ("${CMAKE_BUILD_TYPE}" STREQUAL "Debug")
   target_compile_options(ipc_client PRIVATE -DDEBUG)
endif()

target_link_libraries( ipc_client PUBLIC db_api fc thrift_static ${OPENSSL_LIBRARIES})

target_include_directories( ipc_client
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
                            )

install( TARGETS ipc_client 
   RUNTIME DESTINATION ${CMAKE_INSTALL_FULL_BINDIR}
   LIBRARY DESTINATION ${CMAKE_INSTALL_FULL_LIBDIR}
   ARCHIVE DESTINATION ${CMAKE_INSTALL_FULL_LIBDIR}
#   PUBLIC_HEADER DESTINATION ${CMAKE_INSTALL_FULL_INCLUDEDIR}/eosio/chain
)
#install_directory_permissions( DIRECTORY ${CMAKE_INSTALL_FULL_INCLUDEDIR}/eosio/chain )
