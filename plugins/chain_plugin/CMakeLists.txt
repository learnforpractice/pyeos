file(GLOB HEADERS "include/eosio/chain_plugin/*.hpp")
add_library( chain_plugin SHARED
             chain_plugin.cpp
             ${HEADERS} )

target_link_libraries( chain_plugin eosio_chain appbase )
target_include_directories( chain_plugin PUBLIC 
"${CMAKE_CURRENT_SOURCE_DIR}/include" 
"${CMAKE_SOURCE_DIR}/plugins/chain_interface/include" 
"${CMAKE_SOURCE_DIR}/libraries/appbase/include")
