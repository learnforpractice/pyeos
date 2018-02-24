file(GLOB HEADERS "include/eosio/chain/*.hpp" "include/eosio/chain/contracts/*.hpp")

add_custom_command(
    OUTPUT ${CMAKE_CURRENT_SOURCE_DIR}/python_contract.wrap.cpp
    COMMAND python3 -m cython --cplus ${CMAKE_CURRENT_SOURCE_DIR}/python_contract.pyx -o ${CMAKE_CURRENT_SOURCE_DIR}/python_contract.wrap.cpp
    DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/python_contract.pyx
)

add_custom_command(
    OUTPUT ${CMAKE_CURRENT_SOURCE_DIR}/eoslib/eoslib.wrap.cpp
    COMMAND python3 -m cython --cplus ${CMAKE_CURRENT_SOURCE_DIR}/eoslib/eoslib.pyx -o ${CMAKE_CURRENT_SOURCE_DIR}/eoslib/eoslib.wrap.cpp
    DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/eoslib/eoslib.pyx
)

## SORT .cpp by most likely to change / break compile
add_library( eosio_chain
             chain_config.cpp
             merkle.cpp
             name.cpp
             transaction.cpp
             block.cpp
             wast_to_wasm.cpp
             wasm_interface.cpp

             python_interface.cpp
             micropython_interface.cpp
             block_schedule.cpp

             wasm_eosio_constraints.cpp
             apply_context.cpp
             rate_limiting.cpp

             fork_database.cpp
             get_config.cpp
             block_log.cpp
             asset.cpp


             global_property_object.cpp
             chain_controller.cpp

             contracts/eosio_contract.cpp
             contracts/chain_initializer.cpp
             contracts/genesis_state.cpp
             contracts/abi_serializer.cpp

<<<<<<< HEAD
             types.cpp
             chain_administration_interface.cpp
             message_handling_contexts.cpp

             python_contract.wrap.cpp
             eoslib/eoslib.wrap.cpp
             eoslib/eoslib_.cpp
 
             producer_objects.cpp
             staked_balance_objects.cpp
=======
>>>>>>> origin/master

             micropython/mpeoslib.cpp

             ${HEADERS}
             transaction_metadata.cpp)

<<<<<<< HEAD
target_link_libraries( eos_chain fc chainbase appbase eos_types Logging 
    IR WAST WASM Runtime 
    tinypy micropython
)

target_include_directories( eos_chain
=======
target_link_libraries( eosio_chain eos_utilities fc chainbase Logging IR WAST WASM Runtime )
target_include_directories( eosio_chain
>>>>>>> origin/master
                            PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/include" "${CMAKE_CURRENT_BINARY_DIR}/include"
                                   "${CMAKE_CURRENT_SOURCE_DIR}/../wasm-jit/Include"
                            PUBLIC ${CMAKE_SOURCE_DIR}/libraries/tinypy/Include
                            PUBLIC ${CMAKE_SOURCE_DIR}/libraries/tinypy
                            PUBLIC ${CMAKE_SOURCE_DIR}/libraries/micropython/py
                            PUBLIC ${CMAKE_SOURCE_DIR}/libraries/micropython/extmod
                            PUBLIC ${CMAKE_SOURCE_DIR}/libraries/micropython/extmod/eoslib
                            PUBLIC ${CMAKE_SOURCE_DIR}/libraries/micropython/ports/unix
                            PUBLIC ${CMAKE_SOURCE_DIR}/libraries/micropython/ports/unix/build

                            )

if(MSVC)
  set_source_files_properties( db_init.cpp db_block.cpp database.cpp block_log.cpp PROPERTIES COMPILE_FLAGS "/bigobj" )
endif(MSVC)

INSTALL( TARGETS
   eosio_chain

   RUNTIME DESTINATION bin
   LIBRARY DESTINATION lib
   ARCHIVE DESTINATION lib
)
INSTALL( FILES ${HEADERS} DESTINATION "include/eosio/chain" )

add_subdirectory(test)

#add_executable( test test.cpp )
#target_link_libraries( test eosio_chain ${CMAKE_DL_LIBS} ${PLATFORM_SPECIFIC_LIBS} ${Intl_LIBRARIES} )


