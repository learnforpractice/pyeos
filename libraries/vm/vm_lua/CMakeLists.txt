add_library(vm_lua SHARED 
            vm_lua.cpp
            vm_api.cpp
            )


target_link_libraries(vm_lua PRIVATE eosiolib_native luasandbox)

target_include_directories(vm_lua PRIVATE ${Boost_INCLUDE_DIR}
    PRIVATE ${CMAKE_SOURCE_DIR}/externals/magic_get/include
    PRIVATE ${CMAKE_SOURCE_DIR}/contracts
    PRIVATE ${CMAKE_SOURCE_DIR}/externals/lua_sandbox/include
)
