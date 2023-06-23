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

# cmake-format: off
macro(add_unit_test TEST_NAME)
   add_test( ${TEST_NAME} ${CMAKE_BINARY_DIR}/tests/unit/native/${TEST_NAME} )
   set_property(TEST ${TEST_NAME} PROPERTY LABELS unit_tests)
endmacro()

add_unit_test( asset_tests )
add_unit_test( binary_extension_tests )
add_unit_test( crypto_tests )
add_unit_test( datastream_tests )
add_unit_test( fixed_bytes_tests )
add_unit_test( name_tests )
add_unit_test( rope_tests )
add_unit_test( print_tests )
add_unit_test( serialize_tests )
add_unit_test( symbol_tests )
add_unit_test( system_tests )
add_unit_test( time_tests )
add_unit_test( varint_tests )
add_unit_test( pb_serialize_tests )
add_unit_test( memory_alloc_tests )

# cmake-format: on

add_subdirectory(toolchain-tester)
add_test(
  NAME toolchain_tests
  COMMAND ${CMAKE_CURRENT_BINARY_DIR}/toolchain-tester/toolchain-tester
          ${CMAKE_CURRENT_SOURCE_DIR}/toolchain --cdt ${CMAKE_BINARY_DIR}/bin
  WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/toolchain-tester)
set_property(TEST toolchain_tests PROPERTY LABELS toolchain_tests)

# cmake-format: off

ExternalProject_Add(
  TesterWasmTests
  SOURCE_DIR "${CMAKE_SOURCE_DIR}/tests/tester"
  BINARY_DIR "${CMAKE_BINARY_DIR}/tests/tester/wasm"
  CMAKE_ARGS
    -DCMAKE_TOOLCHAIN_FILE=${CMAKE_BINARY_DIR}/lib/cmake/${CMAKE_PROJECT_NAME}/EosioWasmToolchain.cmake
    -DCMAKE_BUILD_TYPE=Release
  UPDATE_COMMAND ""
  PATCH_COMMAND ""
  TEST_COMMAND ""
  INSTALL_COMMAND ""
  BUILD_ALWAYS 1
  DEPENDS EosioWasmLibraries-Release EosioPlugins)

  ExternalProject_Add(
    TesterNativeTests
    SOURCE_DIR "${CMAKE_SOURCE_DIR}/tests/tester"
    BINARY_DIR "${CMAKE_BINARY_DIR}/tests/tester/native"
    CMAKE_ARGS
      -DCMAKE_TOOLCHAIN_FILE=${CMAKE_BINARY_DIR}/lib/cmake/${CMAKE_PROJECT_NAME}/EosioNativeToolchain.cmake
      -DCMAKE_BUILD_TYPE=Debug
    UPDATE_COMMAND ""
    PATCH_COMMAND ""
    TEST_COMMAND ""
    INSTALL_COMMAND ""
    BUILD_ALWAYS 1
    DEPENDS EosioWasmLibraries-Release EosioNativeLibraries-Debug EosioPlugins)


ExternalProject_Add(
  EosioWasmTests
  SOURCE_DIR "${CMAKE_SOURCE_DIR}/tests/unit/test_contracts"
  BINARY_DIR "${CMAKE_BINARY_DIR}/tests/unit/test_contracts/wasm"
  CMAKE_ARGS
    -DCMAKE_TOOLCHAIN_FILE=${CMAKE_BINARY_DIR}/lib/cmake/${CMAKE_PROJECT_NAME}/EosioWasmToolchain.cmake
    -DCMAKE_BUILD_TYPE=Release
  UPDATE_COMMAND ""
  PATCH_COMMAND ""
  TEST_COMMAND ""
  INSTALL_COMMAND ""
  BUILD_ALWAYS 1
  DEPENDS EosioWasmLibraries-Release EosioPlugins)

ExternalProject_Add(
  EosioNativeTests
  SOURCE_DIR "${CMAKE_SOURCE_DIR}/tests/unit/test_contracts"
  BINARY_DIR "${CMAKE_BINARY_DIR}/tests/unit/test_contracts/native"
  CMAKE_ARGS
    -DCMAKE_TOOLCHAIN_FILE=${CMAKE_BINARY_DIR}/lib/cmake/${CMAKE_PROJECT_NAME}/EosioNativeToolchain.cmake
    -DCMAKE_BUILD_TYPE=Debug
  UPDATE_COMMAND ""
  PATCH_COMMAND ""
  TEST_COMMAND ""
  INSTALL_COMMAND ""
  BUILD_ALWAYS 1
  DEPENDS EosioWasmLibraries-Release EosioNativeLibraries-Debug  EosioPlugins)

ExternalProject_Add(
  EosioLibNativeTests
  SOURCE_DIR "${CMAKE_SOURCE_DIR}/tests/unit"
  BINARY_DIR "${CMAKE_BINARY_DIR}/tests/unit/native"
  CMAKE_ARGS
    -DCMAKE_TOOLCHAIN_FILE=${CMAKE_BINARY_DIR}/lib/cmake/${CMAKE_PROJECT_NAME}/EosioNativeToolchain.cmake
    -DCMAKE_BUILD_TYPE=Debug
  UPDATE_COMMAND ""
  PATCH_COMMAND ""
  TEST_COMMAND ""
  INSTALL_COMMAND ""
  BUILD_ALWAYS 1
  DEPENDS EosioWasmLibraries-Release EosioNativeLibraries-Debug)


function(add_integeration_test Mode build_type tester suffix)
  set(TESTER_PATH ${TAURUS_NODE_ROOT}/bin/${tester})
  string(TOLOWER ${Mode} mode)

  if (ENV{BUILDKITE} OR EXISTS "${TESTER_PATH}")
    add_test(${mode}_integration_tests ${TESTER_PATH}
            ${CMAKE_BINARY_DIR}/tests/integration/integration_tests${suffix})
    set_property(TEST ${mode}_integration_tests PROPERTY LABELS integration_tests)

    add_test(
      NAME ${mode}_tester_tests
      COMMAND ${TESTER_PATH}
              ${CMAKE_BINARY_DIR}/tests/tester/tester${suffix}
      WORKING_DIRECTORY ${CMAKE_BINARY_DIR}/tests/tester)
    set_property(TEST ${mode}_tester_tests PROPERTY LABELS integration_tests)

    ExternalProject_Add(
      Eosio${Mode}IntegrationTests
      SOURCE_DIR "${CMAKE_SOURCE_DIR}/tests/integration"
      BINARY_DIR "${CMAKE_BINARY_DIR}/tests/integration/${Mode}"
      CMAKE_ARGS
        -DCMAKE_TOOLCHAIN_FILE=${CMAKE_BINARY_DIR}/lib/cmake/${CMAKE_PROJECT_NAME}/Eosio${Mode}Toolchain.cmake
        -DCMAKE_BUILD_TYPE=${build_type}
      UPDATE_COMMAND ""
      PATCH_COMMAND ""
      TEST_COMMAND ""
      INSTALL_COMMAND ""
      BUILD_ALWAYS 1
      DEPENDS Eosio${Mode}Tests)
  else()
    message(WARNING "Could not find ${TESTER_PATH}, skipping ${mode} integration tests.")
  endif()
endfunction()

if (TAURUS_NODE_ROOT)
  add_integeration_test(Wasm Release eosio-tester .wasm)
  add_integeration_test(Native Debug native-tester .so)
endif()

add_subdirectory(gen-zpp-tests)
