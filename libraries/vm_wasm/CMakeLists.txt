
add_library(vm_wasm_binaryen SHARED 
           vm_wasm.cpp 
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

#-fvisibility=hidden
target_compile_options(vm_wasm_binaryen PRIVATE -D_BINARYEN)
target_link_libraries(vm_wasm_binaryen  PRIVATE eosiolib_native binaryen wavm-shared softfloat builtins fc)

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


add_library(vm_wasm_wavm SHARED
           vm_wasm.cpp 
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

#-Wl,--version-script=${CMAKE_CURRENT_SOURCE_DIR}/libvm_wasm.map
#-fvisibility=hidden
target_compile_options(vm_wasm_wavm     PRIVATE   -D_WAVM)
target_link_libraries(vm_wasm_wavm       PRIVATE eosiolib_native binaryen wavm-shared softfloat builtins fc
)
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


#reference from https://github.com/BYVoid/uchardet/blob/master/src/symbols.cmake
set (LINK_FLAGS "")

if (TRUE)
    set(_symbols_list "${CMAKE_CURRENT_SOURCE_DIR}/symbols.list")
    set(LINK_FLAGS "${LINK_FLAGS} -Wl,-exported_symbols_list,'${_symbols_list}'")
elseif (CMAKE_CXX_COMPILER_ID STREQUAL GNU)
    set(_version_script "${CMAKE_CURRENT_BINARY_DIR}/version.script")
    set(LINK_FLAGS "${LINK_FLAGS} -Wl,--version-script,\"${_version_script}\"")
endif (APPLE)

#set(LINK_FLAGS "${LINK_FLAGS} -export-symbols-regex '^vm_*'")

set_target_properties(vm_wasm_wavm  PROPERTIES LINK_FLAGS "${LINK_FLAGS}")
set_target_properties(vm_wasm_binaryen  PROPERTIES LINK_FLAGS "${LINK_FLAGS}")
