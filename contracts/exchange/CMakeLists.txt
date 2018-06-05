file(GLOB ABI_FILES "*.abi")
add_wast_executable(TARGET exchange
  INCLUDE_FOLDERS "${STANDARD_INCLUDE_FOLDERS}"
  LIBRARIES libc++ libc eosiolib
  DESTINATION_FOLDER ${CMAKE_CURRENT_BINARY_DIR}
)
configure_file("${ABI_FILES}" "${CMAKE_CURRENT_BINARY_DIR}" COPYONLY)

add_executable(test_exchange test_exchange.cpp )
#bfp/lib/pack.c bfp/lib/posit.cpp bfp/lib/util.c bfp/lib/op2.c)
target_link_libraries( test_exchange fc )
target_include_directories( test_exchange PUBLIC fixed_point/include )

add_library(exchange_native SHARED exchange.cpp) #exchange_state.cpp

target_link_libraries(exchange_native PRIVATE eosiolib_native)

target_include_directories(exchange_native PRIVATE ${Boost_INCLUDE_DIR}
#   PRIVATE ${CMAKE_SOURCE_DIR}/contracts/libc++/upstream/include
#   PRIVATE ${CMAKE_SOURCE_DIR}/contracts/libc++/include/libc++/upstream/include
#   PRIVATE ${CMAKE_SOURCE_DIR}/contracts/libc++/musl/include/musl/upstream/include
#    PRIVATE ${CMAKE_SOURCE_DIR}/contracts/musl/upstream/include
    PRIVATE ${CMAKE_SOURCE_DIR}/externals/magic_get/include
     PRIVATE ${CMAKE_SOURCE_DIR}/contracts
)

