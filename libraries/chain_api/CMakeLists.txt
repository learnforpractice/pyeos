file(GLOB HEADERS "include/eosio/chain/*.hpp")

add_library( chain_api
              SHARED
              chain_api.cpp
             )

target_link_libraries( chain_api PUBLIC fc chainbase eosiolib_native)

target_include_directories( chain_api
                            PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/include"
                            PUBLIC "${CMAKE_BINARY_DIR}/libraries/include"
                            PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/../wasm-jit/Include"
                            PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/../../externals/binaryen/src"
                            PUBLIC ${CMAKE_SOURCE_DIR}/libraries/chain
                            PUBLIC ${CMAKE_SOURCE_DIR}/libraries/chain/include
                            PUBLIC ${CMAKE_SOURCE_DIR}/contracts
                            PRIVATE  ${Boost_INCLUDE_DIR}
                            PUBLIC ${CMAKE_SOURCE_DIR}/externals/magic_get/include
                            )

set_target_properties( chain_api PROPERTIES PUBLIC_HEADER "${HEADERS}" )
install( TARGETS chain_api 
   RUNTIME DESTINATION ${CMAKE_INSTALL_FULL_BINDIR}
   LIBRARY DESTINATION ${CMAKE_INSTALL_FULL_LIBDIR}
   ARCHIVE DESTINATION ${CMAKE_INSTALL_FULL_LIBDIR}
   PUBLIC_HEADER DESTINATION ${CMAKE_INSTALL_FULL_INCLUDEDIR}/eosio/chain
)
