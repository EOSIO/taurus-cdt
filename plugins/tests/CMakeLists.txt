add_executable(blanc_utils_tests ${CMAKE_CURRENT_SOURCE_DIR}/blanc_utils.cpp)

target_include_directories(blanc_utils_tests PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/../eosio ${ut_SOURCE_DIR}/include)
