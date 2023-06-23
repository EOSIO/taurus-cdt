cmake_minimum_required(VERSION 3.5)

project(tests)

set(EOSIO_WASM_OLD_BEHAVIOR "Off")
find_package(eosio.cdt)

add_module(tester tester.cpp)
set_contract_stack_size(tester 65536)
target_compile_options(tester PUBLIC -Os)
target_link_libraries(tester PRIVATE eosio::tester)
set_target_properties(tester PROPERTIES RUNTIME_OUTPUT_DIRECTORY
                       ${CMAKE_CURRENT_BINARY_DIR}/..
                       LIBRARY_OUTPUT_DIRECTORY
                       ${CMAKE_CURRENT_BINARY_DIR}/..)
