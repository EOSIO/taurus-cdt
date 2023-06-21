cmake_minimum_required(VERSION 3.5)
project(cdt_unit_tests)


set(EOSIO_WASM_OLD_BEHAVIOR Off)
find_package(eosio.cdt)

add_library(legacy_tester legacy_tester.cpp)
target_link_libraries(legacy_tester PUBLIC eosio::eosio)


macro(add_cdt_unit_test TEST_NAME)
   add_executable(${TEST_NAME} ${TEST_NAME}.cpp)
   target_link_libraries(${TEST_NAME} PUBLIC legacy_tester)
endmacro()

add_cdt_unit_test(asset_tests)
add_cdt_unit_test(binary_extension_tests)
add_cdt_unit_test(crypto_tests)
add_cdt_unit_test(datastream_tests)
add_cdt_unit_test(fixed_bytes_tests)
add_cdt_unit_test(name_tests)
add_cdt_unit_test(rope_tests)
add_cdt_unit_test(serialize_tests)
# add_cdt_unit_test(string_tests1)
# add_cdt_unit_test(string_tests2)
add_cdt_unit_test(symbol_tests)
add_cdt_unit_test(system_tests)
add_cdt_unit_test(print_tests)
add_cdt_unit_test(time_tests)
add_cdt_unit_test(varint_tests)
add_cdt_unit_test(pb_serialize_tests)
target_compile_options(pb_serialize_tests PRIVATE -ftemplate-backtrace-limit=0)
add_cdt_unit_test(memory_alloc_tests)

add_cdt_unit_test(zpp_json_tests)
target_add_protobuf(zpp_json_tests FILES zpp_json_test.proto)
# target_compile_options( rope_tests PUBLIC -g )
