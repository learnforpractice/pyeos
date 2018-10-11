file(GLOB HEADERS "include/eosio/chain/*.hpp")

add_library( eos_api
              SHARED
              eos_api.cpp
             )

target_link_libraries( eos_api PUBLIC fc chainbase softfloat eosiolib_native chain_api)

target_include_directories( eos_api
                            PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/include"
                            PUBLIC "${CMAKE_BINARY_DIR}/libraries/include"
                            PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/../wasm-jit/Include"
                            PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/../../externals/binaryen/src"
                            PUBLIC ${CMAKE_SOURCE_DIR}/libraries/chain/include
                            PUBLIC ${CMAKE_SOURCE_DIR}/libraries/eos_api/include
                            )

set_target_properties( eos_api PROPERTIES PUBLIC_HEADER "${HEADERS}" )
install( TARGETS eos_api 
   RUNTIME DESTINATION ${CMAKE_INSTALL_FULL_BINDIR}
   LIBRARY DESTINATION ${CMAKE_INSTALL_FULL_LIBDIR}
   ARCHIVE DESTINATION ${CMAKE_INSTALL_FULL_LIBDIR}
   PUBLIC_HEADER DESTINATION ${CMAKE_INSTALL_FULL_INCLUDEDIR}/eosio/chain
)
