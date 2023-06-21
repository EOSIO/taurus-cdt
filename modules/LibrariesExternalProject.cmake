include(ExternalProject)

ExternalProject_Add(
  stdlibs
  SOURCE_DIR "${CMAKE_SOURCE_DIR}/stdlibs"
  BINARY_DIR "${CMAKE_BINARY_DIR}/stdlibs"
  INSTALL_DIR "${CMAKE_BINARY_DIR}"
  CMAKE_ARGS -DCMAKE_TOOLCHAIN_FILE=${CMAKE_BINARY_DIR}/lib/cmake/${CMAKE_PROJECT_NAME}/EosioWasmToolchain.cmake 
             -Dllvm_SOURCE_DIR=${llvm_SOURCE_DIR}
             -Dmusl_SOURCE_DIR=${musl_SOURCE_DIR}
             -DCMAKE_BUILD_TYPE=Release
             -DCMAKE_INSTALL_PREFIX=<INSTALL_DIR>
  UPDATE_COMMAND ""
  PATCH_COMMAND  ""
  TEST_COMMAND   ""
  BUILD_ALWAYS 1
  DEPENDS EosioTools EosioPlugins
)

configure_file(${boost_SOURCE_DIR}/LICENSE_1_0.txt ${CMAKE_BINARY_DIR}/licenses/boost.license COPYONLY)
configure_file(${abieos_SOURCE_DIR}/LICENSE ${CMAKE_BINARY_DIR}/licenses/abieos.license COPYONLY)
configure_file(${catch2_SOURCE_DIR}/LICENSE.txt ${CMAKE_BINARY_DIR}/licenses/Catch2.license COPYONLY)
configure_file(${fmt_SOURCE_DIR}/LICENSE.rst ${CMAKE_BINARY_DIR}/licenses/fmt.license COPYONLY)
configure_file(${softfloat_SOURCE_DIR}/COPYING.txt ${CMAKE_BINARY_DIR}/licenses/softfloat.license COPYONLY)
configure_file(${CMAKE_SOURCE_DIR}/libraries/eosiolib/tester/fpconv.license ${CMAKE_BINARY_DIR}/licenses/fpconv.license COPYONLY)

ExternalProject_Add(
  EosioWasmLibraries-Release
  SOURCE_DIR "${CMAKE_SOURCE_DIR}/libraries"
  BINARY_DIR "${CMAKE_BINARY_DIR}/libraries/wasm/Release"
  INSTALL_DIR "${CMAKE_BINARY_DIR}"
  CMAKE_ARGS -DCMAKE_TOOLCHAIN_FILE=${CMAKE_BINARY_DIR}/lib/cmake/${CMAKE_PROJECT_NAME}/EosioWasmToolchain.cmake 
             -DCMAKE_BUILD_TYPE=Release
             -DCMAKE_INSTALL_PREFIX=<INSTALL_DIR>
             -Dabieos_SOURCE_DIR=${abieos_SOURCE_DIR}
             -Dcatch2_SOURCE_DIR=${catch2_SOURCE_DIR}
             -Dfmt_SOURCE_DIR=${fmt_SOURCE_DIR}
             -Dsoftfloat_SOURCE_DIR=${softfloat_SOURCE_DIR}
             -Dboost_SOURCE_DIR=${boost_SOURCE_DIR}
             -Dzpp_bits_SOURCE_DIR=${zpp_bits_SOURCE_DIR}
             -Dmagic_enum_SOURCE_DIR=${magic_enum_SOURCE_DIR}
  UPDATE_COMMAND ""
  PATCH_COMMAND  ""
  TEST_COMMAND   ""
  BUILD_ALWAYS 1
  DEPENDS EosioTools EosioPlugins stdlibs
)

ExternalProject_Add(
  EosioNativeLibraries-Debug
  SOURCE_DIR "${CMAKE_SOURCE_DIR}/libraries"
  BINARY_DIR "${CMAKE_BINARY_DIR}/libraries/${CMAKE_SYSTEM_PROCESSOR}/Debug"
  INSTALL_DIR "${CMAKE_BINARY_DIR}"
  CMAKE_ARGS -DCMAKE_TOOLCHAIN_FILE=${CMAKE_BINARY_DIR}/lib/cmake/${CMAKE_PROJECT_NAME}/EosioNativeToolchain.cmake 
             -DCMAKE_BUILD_TYPE=Debug
             -DCMAKE_INSTALL_PREFIX=<INSTALL_DIR>
             -Dabieos_SOURCE_DIR=${abieos_SOURCE_DIR}
             -Dcatch2_SOURCE_DIR=${catch2_SOURCE_DIR}
             -Dfmt_SOURCE_DIR=${fmt_SOURCE_DIR}
             -Dboost_SOURCE_DIR=${boost_SOURCE_DIR}
             -Dzpp_bits_SOURCE_DIR=${zpp_bits_SOURCE_DIR}
             -Dmagic_enum_SOURCE_DIR=${magic_enum_SOURCE_DIR}
  INSTALL_COMMAND ${CMAKE_COMMAND} --install <BINARY_DIR> --component libs
  UPDATE_COMMAND ""
  PATCH_COMMAND  ""
  TEST_COMMAND   ""
  BUILD_ALWAYS 1
  DEPENDS EosioTools EosioPlugins 
)

ExternalProject_Add(
  EosioNativeLibraries-Release
  SOURCE_DIR "${CMAKE_SOURCE_DIR}/libraries"
  BINARY_DIR "${CMAKE_BINARY_DIR}/libraries/${CMAKE_SYSTEM_PROCESSOR}/Release"
  INSTALL_DIR "${CMAKE_BINARY_DIR}"
  CMAKE_ARGS -DCMAKE_TOOLCHAIN_FILE=${CMAKE_BINARY_DIR}/lib/cmake/${CMAKE_PROJECT_NAME}/EosioNativeToolchain.cmake 
             -DCMAKE_BUILD_TYPE=Release
             -DCMAKE_INSTALL_PREFIX=<INSTALL_DIR>
             -Dabieos_SOURCE_DIR=${abieos_SOURCE_DIR}
             -Dcatch2_SOURCE_DIR=${catch2_SOURCE_DIR}
             -Dfmt_SOURCE_DIR=${fmt_SOURCE_DIR}
             -Dboost_SOURCE_DIR=${boost_SOURCE_DIR}
             -Dzpp_bits_SOURCE_DIR=${zpp_bits_SOURCE_DIR}
             -Dmagic_enum_SOURCE_DIR=${magic_enum_SOURCE_DIR}
  INSTALL_COMMAND ${CMAKE_COMMAND} --install <BINARY_DIR> --component libs
  UPDATE_COMMAND ""
  PATCH_COMMAND  ""
  TEST_COMMAND   ""
  BUILD_ALWAYS 1
  DEPENDS EosioTools EosioPlugins 
)
