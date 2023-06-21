cmake_minimum_required(VERSION 3.16)

# Sanity check our source directory to make sure that we are not trying to
# generate an in-source build, and to make sure that we don't have any stray
# generated files lying around in the tree
if(CMAKE_CURRENT_SOURCE_DIR STREQUAL CMAKE_CURRENT_BINARY_DIR)
  message(
    FATAL_ERROR
      "In-source builds are not allowed.
Please create a directory and run cmake from there, passing the path
to this source directory as the last argument.
This process created the file `CMakeCache.txt' and the directory `CMakeFiles'.
Please delete them.")
endif()

project(eosio.cdt)
include(FetchContent)

find_program(SCCACHE_FOUND sccache)
if(SCCACHE_FOUND)
  message(STATUS "Using sccache")
  set_property(GLOBAL PROPERTY RULE_LAUNCH_COMPILE sccache)
else()
  find_program(CCACHE_FOUND ccache)
  if(CCACHE_FOUND)
    message(STATUS "Using ccache")
    set_property(GLOBAL PROPERTY RULE_LAUNCH_COMPILE ccache)
  endif()
endif()

set(VERSION_MAJOR 3)
set(VERSION_MINOR 0)
set(VERSION_PATCH 0)

find_package(Git)
execute_process(
  COMMAND ${GIT_EXECUTABLE} rev-parse HEAD
  WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
  OUTPUT_VARIABLE VERSION_SUFFIX
  ERROR_QUIET
  OUTPUT_STRIP_TRAILING_WHITESPACE
)

execute_process(
  COMMAND ${GIT_EXECUTABLE} diff --quiet
  WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
  RESULT_VARIABLE GIT_DIRTY
  ERROR_QUIET
  OUTPUT_STRIP_TRAILING_WHITESPACE 
)

if (GIT_DIRTY)
  set(VERSION_SUFFIX "${VERSION_SUFFIX}-dirty")
endif()

if(VERSION_SUFFIX)
  set(VERSION_FULL
      "${VERSION_MAJOR}.${VERSION_MINOR}.${VERSION_PATCH}-${VERSION_SUFFIX}")
else()
  set(VERSION_FULL "${VERSION_MAJOR}.${VERSION_MINOR}.${VERSION_PATCH}")
endif()

# compatible eosio.cdt version

set(CDT_INSTALL_DIR opt/eosio.cdt/${VERSION_FULL})
set(CMAKE_INSTALL_LIBDIR ${CDT_INSTALL_DIR}/lib)
set(CMAKE_INSTALL_BINDIR ${CDT_INSTALL_DIR}/bin)
set(CMAKE_INSTALL_INCLUDEDIR ${CDT_INSTALL_DIR}/include)

install(CODE "set(CDT_INSTALL_DIR ${CDT_INSTALL_DIR})")

include(GNUInstallDirs)

set(CMAKE_CXX_STANDARD 20)

find_package(LLVM 13 REQUIRED CONFIG)
find_package(Clang REQUIRED)

list(APPEND CMAKE_MODULE_PATH ${LLVM_DIR} ${CMAKE_SOURCE_DIR}/modules)

include(AddLLVM)
include(dependencies)

add_subdirectory(${wabt_SOURCE_DIR} ${wabt_BINARY_DIR} EXCLUDE_FROM_ALL)
set(ABIEOS_DISABLE_TESTS ON)
add_subdirectory(${abieos_SOURCE_DIR} ${abieos_BINARY_DIR} EXCLUDE_FROM_ALL)

set_property(DIRECTORY ${wabt_SOURCE_DIR} PROPERTY COMPILE_OPTIONS -Wno-format)

if(UNIX AND NOT APPLE)
  if("${CMAKE_CXX_COMPILER_ID}" STREQUAL "Clang")
    add_compile_options(-Wno-deprecated-enum-enum-conversion)
  endif()
endif()

add_subdirectory(modules)
add_subdirectory(plugins)
add_subdirectory(tools)

include(LibrariesExternalProject)

configure_file(${CMAKE_SOURCE_DIR}/imports/eosio.imports.in
               ${CMAKE_BINARY_DIR}/eosio.imports COPYONLY)
install(FILES ${CMAKE_BINARY_DIR}/eosio.imports DESTINATION ${CDT_INSTALL_DIR})

configure_file(${CMAKE_SOURCE_DIR}/scripts/ricardeos/ricardeos.py
               ${CMAKE_BINARY_DIR}/scripts/ricardeos.py COPYONLY)
install(
  FILES ${CMAKE_BINARY_DIR}/scripts/ricardeos.py
  DESTINATION ${CDT_INSTALL_DIR}/scripts
  PERMISSIONS
    OWNER_EXECUTE
    OWNER_WRITE
    OWNER_READ
    GROUP_EXECUTE
    GROUP_READ
    WORLD_EXECUTE
    WORLD_READ)

# section for package construction
set(VENDOR "EOISO-Taurus")
set(PROJECT_NAME "taurus-cdt")
set(DESC "Toolchain and supporting tools for the EOSIO-Taurus platform")
set(URL "https://github.com/eosio/taurus-cdt")
set(EMAIL "")
configure_file(${CMAKE_SOURCE_DIR}/scripts/generate_package.sh.in
               ${CMAKE_BINARY_DIR}/packages/generate_package.sh @ONLY)

# add licenses
configure_file(${llvm_SOURCE_DIR}/llvm/LICENSE.TXT
               ${CMAKE_BINARY_DIR}/licenses/llvm.license COPYONLY)
configure_file(${CMAKE_SOURCE_DIR}/libraries/meta_refl/LICENSE
               ${CMAKE_BINARY_DIR}/licenses/meta_refl.license COPYONLY)
configure_file(${wabt_SOURCE_DIR}/LICENSE
               ${CMAKE_BINARY_DIR}/licenses/wabt.license COPYONLY)
configure_file(
  ${CMAKE_SOURCE_DIR}/LICENSE
  ${CMAKE_BINARY_DIR}/licenses/${CMAKE_PROJECT_NAME}.license COPYONLY)
configure_file(${jsoncons_SOURCE_DIR}/LICENSE
               ${CMAKE_BINARY_DIR}/licenses/jsoncons.license COPYONLY)

configure_file(${zpp_bits_SOURCE_DIR}/LICENSE
               ${CMAKE_BINARY_DIR}/licenses/zpp_bits.license COPYONLY)

configure_file(${magic_enum_SOURCE_DIR}/LICENSE
               ${CMAKE_BINARY_DIR}/licenses/magic_enum.license COPYONLY)

install(
  CODE [[
  file(MAKE_DIRECTORY $ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib/cmake/eosio.cdt)
  file(MAKE_DIRECTORY $ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/bin)
  file(WRITE $ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib/cmake/eosio.cdt/eosio.cdt-config.cmake
         "include(\${CMAKE_CURRENT_LIST_DIR}/../../../${CDT_INSTALL_DIR}/lib/cmake/eosio.cdt/eosio.cdt-config.cmake)")
]])

install(
  CODE "file(CREATE_LINK  \"../${CDT_INSTALL_DIR}/bin/eosio-codegen\" \$ENV{DESTDIR}\${CMAKE_INSTALL_PREFIX}/bin/eosio-codegen SYMBOLIC)"
  CODE "file(CREATE_LINK  \"../../../${CDT_INSTALL_DIR}/lib/cmake/eosio.cdt/EosioNativeToolchain.cmake\" \$ENV{DESTDIR}\${CMAKE_INSTALL_PREFIX}/lib/cmake/eosio.cdt/EosioNativeToolchain.cmake SYMBOLIC)"
  CODE "file(CREATE_LINK  \"../../../${CDT_INSTALL_DIR}/lib/cmake/eosio.cdt/EosioWasmToolchain.cmake\" \$ENV{DESTDIR}\${CMAKE_INSTALL_PREFIX}/lib/cmake/eosio.cdt/EosioWasmToolchain.cmake SYMBOLIC)"
)
install(DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/include/ TYPE INCLUDE)
install(DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/lib/ TYPE LIB)
install(DIRECTORY ${CMAKE_BINARY_DIR}/licenses DESTINATION ${CDT_INSTALL_DIR})

include(CTest)
enable_testing()
add_subdirectory(tests)

if(CMAKE_SYSTEM_NAME STREQUAL "Linux")
  configure_file(scripts/DEBIAN/postinst.in ${CMAKE_BINARY_DIR}/DEBIAN/postinst
                 @ONLY)
  install(
    FILES scripts/debian_install_llvm.sh
    DESTINATION ${CDT_INSTALL_DIR}/scripts
    PERMISSIONS
      OWNER_EXECUTE
      OWNER_WRITE
      OWNER_READ
      GROUP_EXECUTE
      GROUP_READ
      WORLD_EXECUTE
      WORLD_READ)
endif()

set(CPACK_PACKAGE_VERSION_MAJOR ${VERSION_MAJOR})
set(CPACK_PACKAGE_VERSION_MINOR ${VERSION_MINOR})
set(CPACK_PACKAGE_VERSION_PATCH ${VERSION_PATCH})
set(CPACK_PACKAGE_VERSION ${VERSION_FULL})

set(CPACK_PACKAGE_NAME ${PROJECT_NAME})
set(CPACK_PACKAGE_VENDOR "${VENDOR}")
set(CMAKE_PROJECT_HOMEPAGE_URL "${URL}")
set(CPACK_PACKAGE_CONTACT "${VENDOR} <${EMAIL}>")
set(CPACK_PACKAGE_DESCRIPTION "${DESC}")
set(CPACK_PACKAGE_DESCRIPTION_SUMMARY "${DESC}")

set(CPACK_RPM_PACKAGE_LICENSE "MIT")
if(VERSION_SUFFIX)
  set(CPACK_RPM_PACKAGE_RELEASE "${VERSION_SUFFIX}")
else()
  set(CPACK_RPM_PACKAGE_RELEASE 1)
endif()

set(CPACK_DEBIAN_PACKAGE_SECTION "devel")
set(CPACK_DEBIAN_PACKAGE_CONTROL_EXTRA ${CMAKE_BINARY_DIR}/DEBIAN/postinst)
set(CPACK_DEBIAN_PACKAGE_SUGGESTS "clang-13, lld-13, llvm-13")

if(CPACK_GENERATOR MATCHES "TGZ" AND NOT DEFINED
                                     CPACK_INCLUDE_TOPLEVEL_DIRECTORY)
  set(CPACK_INCLUDE_TOPLEVEL_DIRECTORY OFF)
endif()

include(CPack)
