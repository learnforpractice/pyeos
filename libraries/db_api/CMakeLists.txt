file(GLOB HEADERS "include/eosio/chain/*.hpp")

add_library( db_api
              SHARED
              name.cpp
              db_api.cpp
             )

target_link_libraries( db_api PUBLIC fc chainbase softfloat)

target_include_directories( db_api
                            PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/include"
                            PUBLIC "${CMAKE_BINARY_DIR}/libraries/include"
                            PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/../wasm-jit/Include"
                            PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/../../externals/binaryen/src"
                            PUBLIC ${CMAKE_SOURCE_DIR}/libraries/chain
                            PUBLIC ${CMAKE_SOURCE_DIR}/libraries/chain/include
                            )

set_target_properties( db_api PROPERTIES PUBLIC_HEADER "${HEADERS}" )
install( TARGETS db_api 
   RUNTIME DESTINATION ${CMAKE_INSTALL_FULL_BINDIR}
   LIBRARY DESTINATION ${CMAKE_INSTALL_FULL_LIBDIR}
   ARCHIVE DESTINATION ${CMAKE_INSTALL_FULL_LIBDIR}
   PUBLIC_HEADER DESTINATION ${CMAKE_INSTALL_FULL_INCLUDEDIR}/eosio/chain
)
