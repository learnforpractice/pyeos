
add_library(vm_wasm_binaryen SHARED vm_wasm.cpp 
           binaryen.cpp
           wavm.cpp
           wast_to_wasm.cpp
           wasm_interface.cpp
           wasm_eosio_injection.cpp
           vm_api.cpp) 

message("++++++++++++++++++" ${CMAKE_CURRENT_LIST_LINE}":"${LevelDB_INCLUDE})
message("++++++++++++++++++" ${CMAKE_CURRENT_LIST_FILE})

#set (ETH_LIBRARY devcore ethereum evm ethashseal ethcore leveldb ${SECP256K1_LIBRARY})
set (ETH_LIBRARY devcore ethereum evm ethashseal ethcore)

target_compile_options(vm_wasm_binaryen     PRIVATE   -D_BINARYEN)
target_link_libraries(vm_wasm_binaryen       PRIVATE eosiolib_native binaryen wavm-shared softfloat builtins fc) #

target_include_directories(vm_wasm_binaryen PRIVATE ${Boost_INCLUDE_DIR}
#   PRIVATE ${CMAKE_SOURCE_DIR}/contracts/libc++/upstream/include
#   PRIVATE ${CMAKE_SOURCE_DIR}/contracts/libc++/include/libc++/upstream/include
#   PRIVATE ${CMAKE_SOURCE_DIR}/contracts/libc++/musl/include/musl/upstream/include
#    PRIVATE ${CMAKE_SOURCE_DIR}/contracts/musl/upstream/include
    PRIVATE ${CMAKE_SOURCE_DIR}/externals/magic_get/include
    PRIVATE ${CMAKE_SOURCE_DIR}/libraries/wasm-jit/Include
    PRIVATE ${CMAKE_SOURCE_DIR}/contracts
    PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/include
    PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/../../externals/binaryen/src"
)


add_library(vm_wasm_wavm SHARED vm_wasm.cpp 
           binaryen.cpp
           wavm.cpp
           wast_to_wasm.cpp
           wasm_interface.cpp
           wasm_eosio_injection.cpp
           vm_api.cpp) 

message("++++++++++++++++++" ${CMAKE_CURRENT_LIST_LINE}":"${LevelDB_INCLUDE})
message("++++++++++++++++++" ${CMAKE_CURRENT_LIST_FILE})

#set (ETH_LIBRARY devcore ethereum evm ethashseal ethcore leveldb ${SECP256K1_LIBRARY})
set (ETH_LIBRARY devcore ethereum evm ethashseal ethcore)

target_compile_options(vm_wasm_wavm     PRIVATE   -D_WAVM)
target_link_libraries(vm_wasm_wavm       PRIVATE eosiolib_native binaryen wavm-shared softfloat builtins fc) #

target_include_directories(vm_wasm_wavm PRIVATE ${Boost_INCLUDE_DIR}
#   PRIVATE ${CMAKE_SOURCE_DIR}/contracts/libc++/upstream/include
#   PRIVATE ${CMAKE_SOURCE_DIR}/contracts/libc++/include/libc++/upstream/include
#   PRIVATE ${CMAKE_SOURCE_DIR}/contracts/libc++/musl/include/musl/upstream/include
#    PRIVATE ${CMAKE_SOURCE_DIR}/contracts/musl/upstream/include
    PRIVATE ${CMAKE_SOURCE_DIR}/externals/magic_get/include
    PRIVATE ${CMAKE_SOURCE_DIR}/libraries/wasm-jit/Include
    PRIVATE ${CMAKE_SOURCE_DIR}/contracts
    PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/include
    PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/../../externals/binaryen/src"
)