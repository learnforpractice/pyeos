add_subdirectory( vm )

add_subdirectory( ipc )

add_subdirectory( vm_manager )

add_subdirectory( builtins )
add_subdirectory( softfloat )
add_subdirectory( chainbase )
add_subdirectory( db_api )
add_subdirectory( wasm-jit )
add_subdirectory( utilities )
add_subdirectory( appbase )
add_subdirectory( chain )
add_subdirectory( testing )
add_subdirectory( abi_generator )
add_subdirectory( fc ) 


#turn these off for now
set(BUILD_TESTS OFF CACHE BOOL "Build GTest-based tests")
set(BUILD_TOOLS OFF CACHE BOOL "Build wabt tools")
set(RUN_RE2C OFF CACHE BOOL "Run re2c")
add_subdirectory( wabt )
