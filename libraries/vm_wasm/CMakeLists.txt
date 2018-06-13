
add_library(vm_wasm SHARED vm_wasm.cpp 
           binaryen.cpp
           wasm_interface.cpp
           wasm_eosio_injection.cpp) 

target_link_libraries(vm_wasm PRIVATE eosiolib_native binaryen softfloat wavm-shared fc)

target_include_directories(vm_wasm PRIVATE ${Boost_INCLUDE_DIR}
#   PRIVATE ${CMAKE_SOURCE_DIR}/contracts/libc++/upstream/include
#   PRIVATE ${CMAKE_SOURCE_DIR}/contracts/libc++/include/libc++/upstream/include
#   PRIVATE ${CMAKE_SOURCE_DIR}/contracts/libc++/musl/include/musl/upstream/include
#    PRIVATE ${CMAKE_SOURCE_DIR}/contracts/musl/upstream/include
    PRIVATE ${CMAKE_SOURCE_DIR}/externals/magic_get/include
    PRIVATE ${CMAKE_SOURCE_DIR}/libraries/wasm-jit/include
    PRIVATE ${CMAKE_SOURCE_DIR}/contracts
    PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/include
    PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/../../externals/binaryen/src"
)
