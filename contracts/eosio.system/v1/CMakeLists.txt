
add_library(eosio_native_v1 SHARED eosio.system.cpp) #exchange_state.cpp

target_link_libraries(eosio_native_v1 PRIVATE eosiolib_native softfloat)

target_include_directories(eosio_native_v1 PRIVATE ${Boost_INCLUDE_DIR}
#   PRIVATE ${CMAKE_SOURCE_DIR}/contracts/libc++/upstream/include
#   PRIVATE ${CMAKE_SOURCE_DIR}/contracts/libc++/include/libc++/upstream/include
#   PRIVATE ${CMAKE_SOURCE_DIR}/contracts/libc++/musl/include/musl/upstream/include
#    PRIVATE ${CMAKE_SOURCE_DIR}/contracts/musl/upstream/include
    PRIVATE ${CMAKE_SOURCE_DIR}/externals/magic_get/include
     PRIVATE ${CMAKE_SOURCE_DIR}/contracts
)

