add_library(vm_native SHARED 
            native_interface.cpp
            vm_native.cpp)

target_link_libraries(vm_native PRIVATE eosiolib_native db_api)

target_include_directories(vm_native PRIVATE ${Boost_INCLUDE_DIR}
#   PRIVATE ${CMAKE_SOURCE_DIR}/contracts/libc++/upstream/include
#   PRIVATE ${CMAKE_SOURCE_DIR}/contracts/libc++/include/libc++/upstream/include
#   PRIVATE ${CMAKE_SOURCE_DIR}/contracts/libc++/musl/include/musl/upstream/include
#    PRIVATE ${CMAKE_SOURCE_DIR}/contracts/musl/upstream/include
    PRIVATE ${CMAKE_SOURCE_DIR}/externals/magic_get/include
    PRIVATE ${CMAKE_BINARY_DIR}/libraries/appbase/include
    PUBLIC ${CMAKE_SOURCE_DIR}/libraries/chainbase/include
    PUBLIC ${CMAKE_SOURCE_DIR}/libraries/softfloat/source/include

    PRIVATE ${CMAKE_BINARY_DIR}/libraries/fc/include
#    PRIVATE ${CMAKE_BINARY_DIR}/libraries/chain/include
    PRIVATE ${CMAKE_SOURCE_DIR}/contracts
)

