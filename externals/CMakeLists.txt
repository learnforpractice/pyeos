add_subdirectory( binaryen )
add_subdirectory( thrift )
add_subdirectory( python )

if (BUILD_ETHEREUM)
   add_subdirectory( ethereum )
endif()

add_subdirectory( lua_sandbox )

#add_subdirectory( mpir-cmake )

