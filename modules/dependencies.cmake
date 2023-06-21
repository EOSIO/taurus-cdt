include(fetch_content)

if(DEFINED ENV{llvm_SOURCE_DIR})
  set(llvm_SOURCE_DIR "$ENV{llvm_SOURCE_DIR}")
endif()

if (CMAKE_VERSION VERSION_GREATER_EQUAL "3.24.0")
	cmake_policy(SET CMP0135 NEW)
endif()

fetch_content(
  llvm
  URL            https://github.com/llvm/llvm-project/archive/refs/tags/llvmorg-13.0.1.tar.gz
  URL_HASH       SHA256=09c50d558bd975c41157364421820228df66632802a4a6a7c9c17f86a7340802
)

fetch_content(
  wabt
  GIT_REPOSITORY https://github.com/WebAssembly/wabt
  GIT_TAG        1.0.29
)


fetch_content(
  jsoncons
  GIT_REPOSITORY https://github.com/danielaparker/jsoncons
  GIT_TAG        v0.168.3
)

fetch_content(
  ut
  GIT_REPOSITORY https://github.com/boost-ext/ut
  GIT_TAG v1.1.8
)

fetch_content(
  musl
  GIT_REPOSITORY https://github.com/EOSIO/musl
  GIT_TAG        5bbae2ff4850d71c8e7446fd0825318bbf2fdef6
)

fetch_content(
  abieos
  GIT_REPOSITORY https://github.com/EOSIO/taurus-abieos
  GIT_TAG        08da184d3ac0ab32bfbb6931e43f4ce3dac38b79
)


fetch_content(
  Catch2
  GIT_REPOSITORY https://github.com/catchorg/Catch2.git
  GIT_TAG        v2.13.8
)

fetch_content(
  fmt
  GIT_REPOSITORY https://github.com/fmtlib/fmt.git
  GIT_TAG        6.0.0
)


fetch_content(
  softfloat
  GIT_REPOSITORY https://github.com/EOSIO/berkeley-softfloat-3.git
  GIT_TAG        203b6df7dedc5bae1b2a7b1b23562335a6344578
)

fetch_content(
  boost
  URL            https://boostorg.jfrog.io/artifactory/main/release/1.78.0/source/boost_1_78_0.tar.gz
  URL_HASH       SHA256=94ced8b72956591c4775ae2207a9763d3600b30d9d7446562c552f0a14a63be7
)

fetch_content(
  cxxopts
  GIT_REPOSITORY https://github.com/jarro2783/cxxopts
  GIT_TAG        v3.0.0
)

fetch_content(
  zpp_bits
  GIT_REPOSITORY https://github.com/EOSIO/taurus-zpp-bits
  GIT_TAG        691ca79fa270e945d74f69bc8e14a4002c9d88fe
)

fetch_content(
  magic_enum
  GIT_REPOSITORY https://github.com/Neargye/magic_enum
  GIT_TAG        v0.8.1
)

find_file(DESCRIPTOR_H google/protobuf/descriptor.h)
find_program(PROTOC protoc)
if (DESCRIPTOR_H AND PROTOC)
  file(READ ${DESCRIPTOR_H} DESCRIPTOR_H_CONTENT)
  string(FIND "${DESCRIPTOR_H_CONTENT}"  "map_key()" has_map_key)
  if (NOT has_map_key EQUAL -1)
    find_package(Protobuf REQUIRED)
  endif()
endif()
if(NOT Protobuf_FOUND)
  set(protobuf_BUILD_TESTS OFF CACHE INTERNAL "")  # Forces the value
  set(protobuf_BUILD_LIBPROTOC ON CACHE INTERNAL "")  # Forces the value
  FetchContent_Declare(
    protobuf
    GIT_REPOSITORY https://github.com/protocolbuffers/protobuf
    GIT_TAG        v21.2
    GIT_SHALLOW TRUE
    FIND_PACKAGE_ARGS
  )
  FetchContent_MakeAvailable(protobuf)
  add_executable(protobuf::protoc ALIAS protoc)
  add_library(protobuf::libprotoc ALIAS libprotoc)
  add_library(protobuf::libprotobuf ALIAS libprotobuf)
  set(Protobuf_INCLUDE_DIR ${protobuf_SOURCE_DIR}/src CACHE INTERNAL "")
  set(Protobuf_LIBRARIES protobuf::libprotobuf CACHE INTERNAL "")

  set_target_properties(protoc PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin"
  )

  file(COPY ${protobuf_SOURCE_DIR}/src/google/protobuf/descriptor.proto DESTINATION ${CMAKE_BINARY_DIR}/include/google/protobuf)
else()
  find_file(DESCRIPTOR_PROTO google/protobuf/descriptor.proto)
  file(COPY ${DESCRIPTOR_PROTO} DESTINATION ${CMAKE_BINARY_DIR}/include/google/protobuf)
endif()
