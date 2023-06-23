include(${CMAKE_BINARY_DIR}/lib/cmake/eosio.cdt/EosioNativeToolchain.cmake)
configure_file(toolchain-tester.in toolchain-tester )
configure_file(codegen.in codegen.sh )
