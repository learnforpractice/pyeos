add_library(vm_julia SHARED
            vm_api.c
            vm_julia.cpp)


target_link_libraries(vm_julia PRIVATE eosiolib_native ${CMAKE_SOURCE_DIR}/externals/julia/usr/lib/libjulia.dylib)

target_include_directories(vm_julia PRIVATE ${Boost_INCLUDE_DIR}
    PRIVATE ${CMAKE_SOURCE_DIR}/externals/magic_get/include
    PRIVATE ${CMAKE_SOURCE_DIR}/contracts
    PRIVATE ${CMAKE_SOURCE_DIR}/externals/julia/src
    PRIVATE ${CMAKE_SOURCE_DIR}/externals/julia/usr/include/julia
)

