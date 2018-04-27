include(wasm)
set(DEFAULT_SYSTEM_INCLUDE_FOLDERS ${CMAKE_SOURCE_DIR}/contracts ${CMAKE_SOURCE_DIR}/contracts/libc++/upstream/include ${CMAKE_SOURCE_DIR}/contracts/musl/upstream/include ${Boost_INCLUDE_DIRS})
set(STANDARD_INCLUDE_FOLDERS ${CMAKE_SOURCE_DIR}/contracts ${CMAKE_SOURCE_DIR}/externals/magic_get/include)

add_custom_command(
    OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/dummy.cpp
    COMMAND touch ${CMAKE_CURRENT_BINARY_DIR}/dummy.cpp
    DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/tools_config.py.in
)

add_wast_executable(TARGET dummy
  INCLUDE_FOLDERS "${STANDARD_INCLUDE_FOLDERS}"
  SOURCE_FILES ${CMAKE_CURRENT_BINARY_DIR}/dummy.cpp
  LIBRARIES libc++ libc eosiolib
  DESTINATION_FOLDER ${CMAKE_CURRENT_BINARY_DIR}
)

string(REPLACE ";" " " WASM_COMMAND "${WASM_COMMAND}")

configure_file( ${CMAKE_CURRENT_SOURCE_DIR}/tools_config.py.in ${CMAKE_CURRENT_SOURCE_DIR}/tools_config.py)

add_custom_target( dummy2 ALL
  echo 'hello,world'
  COMMENT "Generating tools_config.py"
  DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/tools_config.py
)
add_dependencies(dummy2 dummy)
