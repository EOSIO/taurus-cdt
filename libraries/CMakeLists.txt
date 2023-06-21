cmake_minimum_required(VERSION 3.16)

project(eosio_libraries)

find_program(CCACHE_FOUND ccache)
if (CCACHE_FOUND)
  message(STATUS "Using ccache")
  set_property(GLOBAL PROPERTY RULE_LAUNCH_COMPILE ccache)
endif()

set(CMAKE_INSTALL_LIBDIR lib/${CMAKE_SYSTEM_PROCESSOR})
include(GNUInstallDirs)

if (APPLE AND NOT CMAKE_SYSTEM_PROCESSOR MATCHES "^wasm")
  set(CMAKE_CXX_VISIBILITY_PRESET hidden)
  set(CMAKE_VISIBILITY_INLINES_HIDDEN ON)        
endif()

include(${CMAKE_CURRENT_SOURCE_DIR}/../modules/fetch_content.cmake)
set(CMAKE_INSTALL_MESSAGE NEVER)
set(CMAKE_DEBUG_POSTFIX "-debug")

if (NOT CMAKE_FIND_ROOT_PATH)
  set(CMAKE_FIND_ROOT_PATH ${CMAKE_INSTALL_PREFIX})
endif()

if (CMAKE_SYSTEM_PROCESSOR MATCHES "^wasm")
  set(IS_WASM_TARGET ON)
else()
  set(CMAKE_POSITION_INDEPENDENT_CODE ON)
endif()

if (IS_WASM_TARGET)
  add_subdirectory(softfloat)
  add_subdirectory(rt)
  set(EXTRA_TARGETS rt softfloat)
endif()

add_subdirectory(eosiolib)

install(TARGETS eosio embed tester ${EXTRA_TARGETS}
        EXPORT eosio
        COMPONENT libs)

install(TARGETS crt0
        EXPORT eosio
        OBJECTS DESTINATION ${CMAKE_INSTALL_LIBDIR}
        COMPONENT libs)

install(EXPORT eosio
        DESTINATION lib/cmake/eosio.cdt/${CMAKE_SYSTEM_PROCESSOR}
        FILE EosioTargets.cmake
        NAMESPACE eosio::
        COMPONENT libs)

install(DIRECTORY ${boost_SOURCE_DIR}/boost 
        TYPE INCLUDE 
        COMPONENT headers)
install(DIRECTORY ${abieos_SOURCE_DIR}/external/rapidjson/include/rapidjson 
        TYPE INCLUDE 
        COMPONENT headers 
        FILES_MATCHING PATTERN "*.h" PATTERN "*.hpp")
install(DIRECTORY ${fmt_SOURCE_DIR}/include/fmt 
        TYPE INCLUDE 
        COMPONENT headers 
        FILES_MATCHING PATTERN "*.h")
install(DIRECTORY meta_refl/include/bluegrass 
        TYPE INCLUDE 
        COMPONENT headers)

file(READ ${catch2_SOURCE_DIR}/single_include/catch2/catch.hpp FILE_CONTENTS)
string(REPLACE "__FILE__" "__FILE_NAME__" FILE_CONTENTS "${FILE_CONTENTS}")
file(MAKE_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/catch2)
file(WRITE ${CMAKE_CURRENT_BINARY_DIR}/catch2/catch.hpp "${FILE_CONTENTS}")
install(DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/catch2 
        TYPE INCLUDE 
        COMPONENT headers)

install(DIRECTORY ${abieos_SOURCE_DIR}/include/eosio
        TYPE INCLUDE 
        COMPONENT headers
        FILES_MATCHING FILES_MATCHING
        PATTERN "*.h"
        PATTERN "*.hpp"
        PATTERN "eosio/asset.hpp" EXCLUDE
        PATTERN "eosio/crypto.hpp" EXCLUDE
        PATTERN "eosio/fixed_bytes.hpp" EXCLUDE
        PATTERN "eosio/name.hpp" EXCLUDE
        PATTERN "eosio/symbol.hpp" EXCLUDE)

install(DIRECTORY eosiolib/capi/eosio
                  eosiolib/contracts/eosio
                  eosiolib/core/eosio
                  eosiolib/embed/eosio
                  eosiolib/malloc/eosio
                  eosiolib/tester/eosio
        TYPE INCLUDE 
        COMPONENT headers
        FILES_MATCHING 
        PATTERN "*.h"
        PATTERN "*.hpp")

install(FILES ${zpp_bits_SOURCE_DIR}/zpp_bits.h
              ${magic_enum_SOURCE_DIR}/include/magic_enum.hpp
        TYPE INCLUDE 
        COMPONENT headers)
        

