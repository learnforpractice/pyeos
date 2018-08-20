add_subdirectory( vm_example )

if(BUILD_VM_PY)
   add_subdirectory( vm_py )
endif()

add_subdirectory( vm_wasm )

if(BUILD_ETHEREUM)
   add_subdirectory( libevm4eos )
   add_subdirectory( vm_eth )
endif()

add_subdirectory( vm_api_python )

add_subdirectory( vm_native )
add_subdirectory( vm_cpython )
add_subdirectory( vm_cpython_ss )

if (BUILD_VM_JULIA)
   add_subdirectory( vm_julia )
endif()
