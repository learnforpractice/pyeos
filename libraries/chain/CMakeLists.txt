file(GLOB HEADERS "include/eos/chain/*.hpp")

add_custom_command(
    OUTPUT ${CMAKE_CURRENT_SOURCE_DIR}/python_contract.wrap.cpp
    COMMAND cython --cplus ${CMAKE_CURRENT_SOURCE_DIR}/python_contract.pyx -o ${CMAKE_CURRENT_SOURCE_DIR}/python_contract.wrap.cpp
    DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/python_contract.pyx
)

add_custom_command(
    OUTPUT ${CMAKE_CURRENT_SOURCE_DIR}/eoslib/eoslib.wrap.cpp
    COMMAND cython --cplus ${CMAKE_CURRENT_SOURCE_DIR}/eoslib/eoslib.pyx -o ${CMAKE_CURRENT_SOURCE_DIR}/eoslib/eoslib.wrap.cpp
    DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/eoslib/eoslib.pyx
)

## SORT .cpp by most likely to change / break compile
add_library( eos_chain
             wast_to_wasm.cpp
             chain_controller.cpp
             wasm_interface.cpp
             python_interface.cpp
             block_schedule.cpp

             fork_database.cpp

             transaction.cpp
             block.cpp

             get_config.cpp

             block_log.cpp
             BlockchainConfiguration.cpp

             types.cpp
             chain_administration_interface.cpp
             message_handling_contexts.cpp
             python_contract.wrap.cpp
             eoslib/eoslib.wrap.cpp
             eoslib/eoslib_.cpp
             ${HEADERS}
           )

target_link_libraries( eos_chain fc chainbase appbase eos_types Logging 
    IR WAST WASM Runtime 
    tinypy
)

target_include_directories( eos_chain
                            PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/include" "${CMAKE_CURRENT_BINARY_DIR}/include"
                                   "${CMAKE_CURRENT_SOURCE_DIR}/../wasm-jit/Include"
                            PUBLIC ${CMAKE_SOURCE_DIR}/libraries/tinypy/Include
                            PUBLIC ${CMAKE_SOURCE_DIR}/libraries/tinypy
                            )

if(MSVC)
  set_source_files_properties( db_init.cpp db_block.cpp database.cpp block_log.cpp PROPERTIES COMPILE_FLAGS "/bigobj" )
endif(MSVC)

INSTALL( TARGETS
   eos_chain

   RUNTIME DESTINATION bin
   LIBRARY DESTINATION lib
   ARCHIVE DESTINATION lib
)
INSTALL( FILES ${HEADERS} DESTINATION "include/eos/chain" )
