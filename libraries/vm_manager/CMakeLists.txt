file(GLOB HEADERS "include/eosio/chain/*.hpp")

add_library( vm_manager
              SHARED
              ro_db.cpp
              rw_db.cpp
              utility.cpp
              vm_manager.cpp
             )

target_link_libraries( vm_manager PUBLIC fc appbase db_api eosiolib_native)

target_include_directories( vm_manager
                            PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/include"
                            PUBLIC "${CMAKE_BINARY_DIR}/libraries/include"
                            PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/../wasm-jit/Include"
                            PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/../../externals/binaryen/src"
                            PUBLIC ${CMAKE_SOURCE_DIR}/libraries/chain
                            PUBLIC ${CMAKE_SOURCE_DIR}/libraries/chain/include
                            PUBLIC ${CMAKE_SOURCE_DIR}/libraries/vm_py
                            PUBLIC ${CMAKE_SOURCE_DIR}/libraries/vm_wasm
                            PUBLIC ${CMAKE_SOURCE_DIR}/libraries/vm_py/py
                            PUBLIC ${CMAKE_SOURCE_DIR}/contracts
                            PUBLIC ${CMAKE_SOURCE_DIR}/externals/magic_get/include
                            )

set_target_properties( vm_manager PROPERTIES PUBLIC_HEADER "${HEADERS}" )
install( TARGETS vm_manager 
   RUNTIME DESTINATION ${CMAKE_INSTALL_FULL_BINDIR}
   LIBRARY DESTINATION ${CMAKE_INSTALL_FULL_LIBDIR}
   ARCHIVE DESTINATION ${CMAKE_INSTALL_FULL_LIBDIR}
   PUBLIC_HEADER DESTINATION ${CMAKE_INSTALL_FULL_INCLUDEDIR}/eosio/chain
)
