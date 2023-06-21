
cmake_minimum_required(VERSION 3.5)
project(cdt_unit_tests)

set(EOSIO_WASM_OLD_BEHAVIOR Off)
find_package(eosio.cdt)

macro(add_test_contract contract)
  add_contract(${contract} ${contract} ${ARGN})
  set_target_properties(${contract} PROPERTIES RUNTIME_OUTPUT_DIRECTORY
                       ${CMAKE_BINARY_DIR}/..
                       LIBRARY_OUTPUT_DIRECTORY
                       ${CMAKE_BINARY_DIR}/..)
endmacro()

add_test_contract(action_results_test action_results_test.cpp)
add_test_contract(simple_tests simple_tests.cpp)
add_test_contract(transfer_contract transfer.cpp)
add_test_contract(minimal_tests minimal_tests.cpp)
add_test_contract(tester_tests tester_tests.cpp)
add_test_contract(enum_tests enum_tests.cpp)
add_test_contract(kv_single_index_tests kv_single_index_tests.cpp)
add_test_contract(kv_variant_tests kv_variant_tests.cpp)
add_test_contract(capi_tests)
target_sources(capi_tests PRIVATE
  capi/capi.c
  capi/action.c
  capi/chain.c
  capi/crypto.c
  capi/db.c
  capi/permission.c
  capi/print.c
  capi/privileged.c
  capi/system.c
  capi/transaction.c)
add_test_contract(kv_bios kv_bios/kv_bios.cpp)
add_test_contract(boot boot.cpp)
add_test_contract(pb_msg_test pb_msg_test.cpp)
add_test_contract(malloc_free malloc_free.cpp)
add_test_contract(malloc_tests malloc_tests.cpp)
add_test_contract(kv_migrate kv_migrate.cpp)
target_add_protobuf(pb_msg_test OUTPUT_DIRECTORY test 
                                FILES test.proto)
add_test_contract(push_event_test push_event_test.cpp)
add_test_contract(rsa_verify_test rsa_verify_test.cpp)
add_test_contract(ecdsa_verify_test ecdsa_verify_test.cpp)

configure_file(${CMAKE_CURRENT_SOURCE_DIR}/simple_wrong.abi
               ${CMAKE_CURRENT_BINARY_DIR}/simple_wrong.abi COPYONLY)
configure_file(${CMAKE_CURRENT_SOURCE_DIR}/capi/capi_tests.abi
               ${CMAKE_CURRENT_BINARY_DIR}/capi_tests.abi COPYONLY)
configure_file(${CMAKE_CURRENT_SOURCE_DIR}/kv_bios/kv_bios.abi
               ${CMAKE_CURRENT_BINARY_DIR}/kv_bios.abi COPYONLY)

