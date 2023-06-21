

configure_file(gen_zpp_test.sh.in gen_zpp_test.sh @ONLY)

function(zpp_gen_test)
    cmake_parse_arguments(ZPP_GEN "" "NAME" "INPUT_FILES" ${ARGN}) 
    add_test(NAME ${ZPP_GEN_NAME}
            COMMAND bash gen_zpp_test.sh ${ZPP_GEN_INPUT_FILES}
            WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR})
    set_tests_properties(${ZPP_GEN_NAME} PROPERTIES LABELS zpp_gen_tests)
endfunction()


zpp_gen_test(NAME monster INPUT_FILES monster.proto)
zpp_gen_test(NAME unittest_proto3_lite INPUT_FILES unittest_proto3_lite.proto google/protobuf/unittest_import.proto)
zpp_gen_test(NAME unittest_zpp_custom_options INPUT_FILES unittest_zpp_custom_options.proto)