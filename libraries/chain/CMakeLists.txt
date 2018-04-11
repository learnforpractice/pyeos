file(GLOB HEADERS "include/eosio/chain/*.hpp" "include/eosio/chain/contracts/*.hpp")

find_package(LevelDB)

set(PYTHON3 "${CMAKE_SOURCE_DIR}/libraries/python/dist/bin/python3")
 
add_custom_command(
    OUTPUT ${CMAKE_CURRENT_SOURCE_DIR}/eoslib/eoslib_.wrap.cpp
    COMMAND ${PYTHON3} -m cython --cplus ${CMAKE_CURRENT_SOURCE_DIR}/eoslib/eoslib_.pyx -o ${CMAKE_CURRENT_SOURCE_DIR}/eoslib/eoslib_.wrap.cpp
    DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/eoslib/eoslib_.pyx
)

add_custom_command(
    OUTPUT ${CMAKE_CURRENT_SOURCE_DIR}/rpc_interface/rpc_interface_.wrap.cpp
    COMMAND ${PYTHON3} -m cython --cplus ${CMAKE_CURRENT_SOURCE_DIR}/rpc_interface/rpc_interface_.pyx -o ${CMAKE_CURRENT_SOURCE_DIR}/rpc_interface/rpc_interface_.wrap.cpp
    DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/rpc_interface/rpc_interface_.pyx
)


## SORT .cpp by most likely to change / break compile
add_library( eosio_chain
             chain_config.cpp
             merkle.cpp
             name.cpp
             transaction.cpp
             block.cpp
             block_trace.cpp
             wast_to_wasm.cpp
             wasm_interface.cpp

             evm_interface.cpp
             micropython_interface.cpp
             
             rpc_interface/rpc_interface.cpp
             rpc_interface/rpc_interface_.wrap.cpp

#             wasm_eosio_constraints.cpp

             wasm_eosio_validation.cpp
             wasm_eosio_injection.cpp

             apply_context.cpp
             resource_limits.cpp

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

             micropython/mpeoslib.cpp

             evm/EosState.cpp
             evm/EosExecutive.cpp
             evm/EosDB.cpp
             evm/EosExtVM.cpp

             webassembly/wavm.cpp
             webassembly/binaryen.cpp

             eoslib/eoslib_.wrap.cpp
             eoslib/eoslib_.cpp

             ${HEADERS}
             transaction_metadata.cpp)

set(CPP_ETHEREUM_DIR "${CMAKE_SOURCE_DIR}/libraries/ethereum")

include_directories (${CPP_ETHEREUM_DIR})
include_directories (${CPP_ETHEREUM_DIR}/evmjit/include)
include_directories (${CPP_ETHEREUM_DIR}/utils)
include_directories (jsoncpp/include)

#add_library(secp256k STATIC IMPORTED)
#set_property(TARGET secp256k PROPERTY IMPORTED_LOCATION "${CMAKE_SOURCE_DIR}/build/deps/lib/libsecp256k1.a")

#set (ETH_LIBRARY devcore ethereum evm ethashseal ethcore leveldb ${SECP256K1_LIBRARY})
set (ETH_LIBRARY devcore ethereum evm ethashseal ethcore)

message("++++++++++++++++++"${CMAKE_CURRENT_LIST_LINE}":"${LevelDB_INCLUDE})
message("++++++++++++++++++"${CMAKE_CURRENT_LIST_FILE})

target_link_libraries( eosio_chain PUBLIC eos_utilities fc chainbase appbase Logging IR WAST WASM Runtime 
        wasm asmjs passes cfg ast emscripten-optimizer support softfloat ${ETH_LIBRARY} python3)

target_include_directories( eosio_chain
                            PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/include" "${CMAKE_CURRENT_BINARY_DIR}/include"
                                   "${CMAKE_CURRENT_SOURCE_DIR}/../wasm-jit/Include"
                                   "${CMAKE_CURRENT_SOURCE_DIR}/../../externals/binaryen/src"
                            PUBLIC ${CMAKE_SOURCE_DIR}/libraries/python/Include
                            PUBLIC ${CMAKE_SOURCE_DIR}/libraries/python
                            PUBLIC ${CMAKE_SOURCE_DIR}/libraries/micropython
                            PUBLIC ${CMAKE_SOURCE_DIR}/libraries/micropython/py
                            PUBLIC ${CMAKE_SOURCE_DIR}/libraries/micropython/extmod
                            PUBLIC ${CMAKE_SOURCE_DIR}/libraries/micropython/ports/unix
                            PUBLIC ${CMAKE_SOURCE_DIR}/libraries/micropython/ports/unix/build
                            PRIVATE ${LevelDB_INCLUDE}
                            PRIVATE ${CPP_ETHEREUM_DIR}
                            )

if(MSVC)
  set_source_files_properties( db_init.cpp db_block.cpp database.cpp block_log.cpp PROPERTIES COMPILE_FLAGS "/bigobj" )
endif(MSVC)

