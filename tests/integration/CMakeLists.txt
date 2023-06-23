cmake_minimum_required( VERSION 3.22 )
project(cdt_integration_test)

set(EOSIO_WASM_OLD_BEHAVIOR "off")
find_package(eosio.cdt)

configure_file(${CMAKE_SOURCE_DIR}/contracts.hpp.in ${CMAKE_BINARY_DIR}/contracts.hpp)
include_directories(${CMAKE_BINARY_DIR})

add_module(integration_tests action_results_test.cpp capi_tests.cpp codegen_tests.cpp kv_tests.cpp pb_test.cpp main.cpp push_event_test.cpp malloc_free_test.cpp memory_tests.cpp rsa_verify_test.cpp ecdsa_verify_test.cpp)
set_contract_stack_size(integration_tests 65536)
target_link_libraries(integration_tests PUBLIC eosio::tester)
set_target_properties(integration_tests
  PROPERTIES
  RUNTIME_OUTPUT_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/..
  LIBRARY_OUTPUT_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/..)

target_add_protobuf(integration_tests 
  INPUT_DIRECTORY ../unit/test_contracts
  FILES test.proto)
