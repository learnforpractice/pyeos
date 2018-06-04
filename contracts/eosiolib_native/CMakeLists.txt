add_library(eosiolib_native STATIC eosiolib.cpp
action.cpp
chain.cpp
crypto.cpp
db.cpp
memory.cpp
permission.cpp
print.cpp
privileged.cpp
system.cpp
transaction.cpp
) #exchange_state.cpp

target_link_libraries( eosiolib_native PRIVATE )

target_include_directories(eosiolib_native PRIVATE ${Boost_INCLUDE_DIR}
#   PRIVATE ${CMAKE_SOURCE_DIR}/contracts/libc++/upstream/include
#   PRIVATE ${CMAKE_SOURCE_DIR}/contracts/libc++/include/libc++/upstream/include
#   PRIVATE ${CMAKE_SOURCE_DIR}/contracts/libc++/musl/include/musl/upstream/include
#    PRIVATE ${CMAKE_SOURCE_DIR}/contracts/musl/upstream/include
    PRIVATE ${CMAKE_SOURCE_DIR}/externals/magic_get/include
    PRIVATE ${CMAKE_SOURCE_DIR}/contracts
    PRIVATE ${CMAKE_SOURCE_DIR}/contracts/eosiolib
)

