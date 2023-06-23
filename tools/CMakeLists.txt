set(CMAKE_CXX_STANDARD 20)

file(MAKE_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
function(copy_to_bin target)
  add_custom_command(OUTPUT
          ${CMAKE_BINARY_DIR}/bin/${target}
      COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_FILE:${target}>
          ${CMAKE_BINARY_DIR}/bin
      DEPENDS ${target}
  )
  # Create target which consume the command via DEPENDS.
  add_custom_target(copy_${target} ALL
      DEPENDS ${CMAKE_BINARY_DIR}/bin/${target}
  )
  install(TARGETS ${target})
endfunction()


add_executable(eosio-codegen src/eosio-codegen.cpp ${abieos_SOURCE_DIR}/src/pb_support.cpp)
target_compile_definitions(eosio-codegen PRIVATE
  SHARED_LIB_SUFFIX=\"${CMAKE_SHARED_LIBRARY_SUFFIX}\"
  ABIEOS_HAS_PROTOBUF
)
target_include_directories(eosio-codegen
  PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/include
    ${jsoncons_SOURCE_DIR}/include
    ${cxxopts_SOURCE_DIR}/include
    ${LLVM_INCLUDE_DIRS}
)
target_link_libraries(eosio-codegen PRIVATE LLVMSupport abieos)

set_target_properties(eosio-codegen PROPERTIES RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin")

install(TARGETS eosio-codegen)

target_compile_definitions(eosio-codegen PRIVATE EOSIO_CDT_VERSION="${VERSION_FULL}")

add_executable(eosio-pp src/postpass.cc)
target_link_libraries(eosio-pp wabt)
target_include_directories(eosio-pp PRIVATE ${wabt_SOURCE_DIR} ${wabt_BINARY_DIR})
set_target_properties(eosio-pp PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin"
)
install(TARGETS eosio-pp)


function(target_add_protobuf TARGET)
  foreach(protofile ${ARGN})
    cmake_path(REPLACE_EXTENSION protofile LAST_ONLY "pb.h" OUTPUT_VARIABLE hdr)
    cmake_path(REPLACE_EXTENSION protofile LAST_ONLY "pb.cc" OUTPUT_VARIABLE src)
    list(APPEND hdrs ${hdr})
    list(APPEND srcs ${src})
  endforeach()

  add_custom_command(
    COMMENT Generating ${hdrs} ${srcs}
    OUTPUT ${hdrs} ${srcs}
    COMMAND protobuf::protoc -I ${Protobuf_INCLUDE_DIR} -I ${CMAKE_CURRENT_SOURCE_DIR} --cpp_out ${CMAKE_CURRENT_BINARY_DIR} ${ARGN}
    DEPENDS protobuf::protoc
    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
  )
  target_sources(${TARGET} PRIVATE ${srcs} ${hdrs}) 
endfunction()

add_executable(protoc-gen-zpp src/zpp-protoc-plugin.cpp)
target_add_protobuf(protoc-gen-zpp src/zpp_options.proto)
target_link_libraries(protoc-gen-zpp PRIVATE protobuf::libprotoc protobuf::libprotobuf)
target_include_directories(protoc-gen-zpp PRIVATE ${CMAKE_CURRENT_BINARY_DIR})
install(TARGETS protoc-gen-zpp)
set_target_properties(protoc-gen-zpp PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin"
)

copy_to_bin(wat2wasm)
copy_to_bin(wasm2wat)
copy_to_bin(wasm-objdump)

foreach(target clang clang++ wasm-ld llvm-ar llvm-ranlib )
  file(CREATE_LINK  ${LLVM_TOOLS_BINARY_DIR}/${target} ${CMAKE_BINARY_DIR}/bin/${target} SYMBOLIC)
  install(FILES ${CMAKE_BINARY_DIR}/bin/${target} TYPE BIN)
endforeach()

if(NOT TARGET EosioTools)
  add_custom_target(EosioTools)
  add_dependencies(EosioTools eosio-codegen eosio-pp)
endif()

configure_file(src/zpp_options.proto ${CMAKE_CURRENT_BINARY_DIR}/../include/zpp/zpp_options.proto COPYONLY)

