## SORT .cpp by most likely to change / break compile
add_library( vm_api_ipc SHARED
#    rpc_interface.cpp
#    rpc_interface_.wrap.cpp

    vm_api.cpp

    cpp/RpcService.cpp
    cpp/RpcService_types.cpp
#    cpp/rpceoslib.cpp
)

message("++++++++++++++++++" ${CMAKE_CURRENT_LIST_LINE}":"${LevelDB_INCLUDE})
message("++++++++++++++++++" ${CMAKE_CURRENT_LIST_FILE})

if ("${CMAKE_BUILD_TYPE}" STREQUAL "Debug")
   target_compile_options(vm_api_ipc PRIVATE -DDEBUG)
endif()

target_link_libraries( vm_api_ipc PUBLIC db_api fc thrift_static ${OPENSSL_LIBRARIES})

target_include_directories( vm_api_ipc
                            PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/include"
                            PUBLIC "${CMAKE_CURRENT_BINARY_DIR}/include"
                            PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/../wasm-jit/Include"
                            PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/../../externals/binaryen/src"
                            PUBLIC ${CMAKE_SOURCE_DIR}/libraries/chain/include
                            PUBLIC ${CMAKE_SOURCE_DIR}/contracts
                            PUBLIC ${CMAKE_SOURCE_DIR}/libraries/chainbase/include
                            PUBLIC ${CMAKE_SOURCE_DIR}/externals/thrift/src
                            )

install( TARGETS vm_api_ipc 
   RUNTIME DESTINATION ${CMAKE_INSTALL_FULL_BINDIR}
   LIBRARY DESTINATION ${CMAKE_INSTALL_FULL_LIBDIR}
   ARCHIVE DESTINATION ${CMAKE_INSTALL_FULL_LIBDIR}
#   PUBLIC_HEADER DESTINATION ${CMAKE_INSTALL_FULL_INCLUDEDIR}/eosio/chain
)
#install_directory_permissions( DIRECTORY ${CMAKE_INSTALL_FULL_INCLUDEDIR}/eosio/chain )
