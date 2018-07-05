set(HEADERS
    ${CMAKE_SOURCE_DIR}/externals/magic_get/include
    ${CMAKE_SOURCE_DIR}/libraries/wasm-jit/Include
    ${CMAKE_SOURCE_DIR}/contracts
    ${CMAKE_CURRENT_SOURCE_DIR}/include
    PUBLIC ${CMAKE_SOURCE_DIR}/libraries/chainbase/include
    "${CMAKE_CURRENT_SOURCE_DIR}/../../externals/binaryen/src" )


#reference from https://github.com/BYVoid/uchardet/blob/master/src/symbols.cmake
set (LINK_FLAGS "")

if (APPLE)
    set(_symbols_list "${CMAKE_CURRENT_SOURCE_DIR}/symbols.list")
    set(LINK_FLAGS "${LINK_FLAGS} -Wl,-exported_symbols_list,'${_symbols_list}'")
elseif (CMAKE_CXX_COMPILER_ID STREQUAL GNU)
    set(_version_script "${CMAKE_CURRENT_BINARY_DIR}/version.script")
    set(LINK_FLAGS "${LINK_FLAGS} -Wl,--version-script,\"${_version_script}\"")
endif (APPLE)

#set(LINK_FLAGS "${LINK_FLAGS} -export-symbols-regex '^vm_*'")


add_library(vm_wasm_binaryen SHARED 
           vm_wasm.cpp 
           binaryen.cpp
           wavm.cpp
           wast_to_wasm.cpp
           wasm_interface.cpp
           wasm_eosio_injection.cpp
           vm_api.cpp name.cpp) 

target_compile_options(vm_wasm_binaryen PRIVATE -D_BINARYEN)
target_link_libraries(vm_wasm_binaryen  PRIVATE binaryen wavm-shared softfloat builtins fc)

target_include_directories(vm_wasm_binaryen PRIVATE ${HEADERS})


add_library(vm_wasm_wavm STATIC
#           vm_wasm.cpp 
           binaryen.cpp
           wavm.cpp
           wast_to_wasm.cpp
           wasm_interface.cpp
           wasm_eosio_injection.cpp
           vm_api.cpp name.cpp) 

target_compile_options(vm_wasm_wavm     PRIVATE   -D_WAVM)
target_link_libraries(vm_wasm_wavm       PRIVATE eosiolib_native binaryen wavm-shared softfloat builtins fc)

target_include_directories(vm_wasm_wavm PRIVATE ${HEADERS})


set_target_properties(vm_wasm_wavm  PROPERTIES LINK_FLAGS "${LINK_FLAGS}")
set_target_properties(vm_wasm_binaryen  PROPERTIES LINK_FLAGS "${LINK_FLAGS}")



foreach(LIBINDEX RANGE 0 10 1)
    add_library(vm_wasm_wavm-${LIBINDEX} SHARED vm_wasm.cpp)

    target_compile_options(vm_wasm_wavm-${LIBINDEX}     PRIVATE   -D_WAVM)
    target_link_libraries(vm_wasm_wavm-${LIBINDEX}      PRIVATE vm_wasm_wavm eosiolib_native 
                            binaryen wavm-shared softfloat builtins fc)

    target_include_directories(vm_wasm_wavm-${LIBINDEX} PRIVATE ${HEADERS})
    
    set_target_properties(vm_wasm_wavm-${LIBINDEX}  PROPERTIES LINK_FLAGS "${LINK_FLAGS}")
endforeach(LIBINDEX)

