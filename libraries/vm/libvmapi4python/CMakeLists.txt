set(PYTHON3 "LD_LIBRARY_PATH=${CMAKE_SOURCE_DIR}/externals/python;${CMAKE_SOURCE_DIR}/externals/python/dist/bin/python3")

add_custom_command(
    OUTPUT ${CMAKE_CURRENT_SOURCE_DIR}/eoslib.wrap.cpp
    COMMAND ${PYTHON3} -m cython --cplus ${CMAKE_CURRENT_SOURCE_DIR}/eoslib.pyx -o ${CMAKE_CURRENT_SOURCE_DIR}/eoslib.wrap.cpp
    DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/eoslib.pyx
)

add_custom_command(
    OUTPUT ${CMAKE_CURRENT_SOURCE_DIR}/db.wrap.cpp
    COMMAND ${PYTHON3} -m cython --cplus ${CMAKE_CURRENT_SOURCE_DIR}/db.pyx -o ${CMAKE_CURRENT_SOURCE_DIR}/db.wrap.cpp
    DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/db.pyx
)

add_library(vmapi4python STATIC
            db.wrap.cpp
            eoslib_.cpp
            eoslib.wrap.cpp
            )

set_target_properties(vmapi4python  PROPERTIES LINK_FLAGS "${LINK_FLAGS}")

target_link_libraries(vmapi4python PRIVATE eosiolib_native fc)

target_include_directories(vmapi4python PRIVATE ${Boost_INCLUDE_DIR}
#   PRIVATE ${CMAKE_SOURCE_DIR}/contracts/libc++/upstream/include
#   PRIVATE ${CMAKE_SOURCE_DIR}/contracts/libc++/include/libc++/upstream/include
#   PRIVATE ${CMAKE_SOURCE_DIR}/contracts/libc++/musl/include/musl/upstream/include
#    PRIVATE ${CMAKE_SOURCE_DIR}/contracts/musl/upstream/include
    PRIVATE ${CMAKE_SOURCE_DIR}/externals/magic_get/include
    PRIVATE ${CMAKE_BINARY_DIR}/libraries/appbase/include
    PUBLIC ${CMAKE_SOURCE_DIR}/libraries/chainbase/include
    PUBLIC ${CMAKE_SOURCE_DIR}/libraries/softfloat/source/include

    PUBLIC ${CMAKE_SOURCE_DIR}/externals/python/Include
    PUBLIC ${CMAKE_SOURCE_DIR}/externals/python

    PRIVATE ${CMAKE_BINARY_DIR}/libraries/fc/include
#    PRIVATE ${CMAKE_BINARY_DIR}/libraries/chain/include
    PRIVATE ${CMAKE_SOURCE_DIR}/contracts
)

