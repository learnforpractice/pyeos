
set(sources
    EVMC.cpp EVMC.h
    ExtVMFace.cpp ExtVMFace.h
    Instruction.cpp Instruction.h
    interpreter.h
    LegacyVM.cpp LegacyVM.h
    LegacyVMCalls.cpp
    LegacyVMOpt.cpp
    VMFace.h
    VMConfig.h
    VM.cpp VM.h
    VMCalls.cpp
    VMOpt.cpp
    VMSIMD.cpp
    VMValidate.cpp
    VMFactory.cpp VMFactory.h
    VM_API.cpp
)

add_library(evm4eos ${sources})

target_link_libraries(evm4eos PUBLIC eosiolib_native)

if(EVM_OPTIMIZE)
    target_compile_definitions(evm4eos PRIVATE EVM_OPTIMIZE)
endif()


target_include_directories(evm4eos PRIVATE ${Boost_INCLUDE_DIR}
    PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include
    PRIVATE ${CMAKE_SOURCE_DIR}/contracts
)

