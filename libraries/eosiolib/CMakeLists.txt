set(CMAKE_CXX_OUTPUT_EXTENSION_REPLACE 1)

set(eosio_SOURCES eosiolib.cpp crypto.cpp tester/fpconv.c
                  ${abieos_SOURCE_DIR}/src/crypto.cpp)

if (IS_WASM_TARGET) 
  list(APPEND eosio_SOURCES simple_malloc.cpp)
endif()

add_library(eosio ${eosio_SOURCES})
target_compile_definitions(eosio PUBLIC BOOST_DISABLE_ASSERTS __eosio_cdt__)
target_include_directories(
  eosio
  PUBLIC $<BUILD_INTERFACE:${boost_SOURCE_DIR}>
         $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/../meta_refl/include>
         $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/core>
         $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/contracts>
         $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/malloc>
         $<BUILD_INTERFACE:${abieos_SOURCE_DIR}/include>
         $<BUILD_INTERFACE:${abieos_SOURCE_DIR}/external/rapidjson/include>
         $<BUILD_INTERFACE:${zpp_bits_SOURCE_DIR}>
         $<BUILD_INTERFACE:${magic_enum_SOURCE_DIR}/include>
         $<INSTALL_INTERFACE:include>)

add_library(crt0 OBJECT tester/crt0.cpp)
target_link_libraries(crt0 PRIVATE eosio)
target_include_directories(crt0 PRIVATE ${fmt_SOURCE_DIR}/include
                                        ${CMAKE_CURRENT_SOURCE_DIR}/tester)


add_library(tester tester/tester.cpp tester/tester_intrinsics.cpp 
                         ${abieos_SOURCE_DIR}/src/crypto.cpp)
add_library(eosio::tester ALIAS tester)

target_compile_definitions(tester PUBLIC CATCH_CONFIG_NO_POSIX_SIGNALS EOSIO_BUILD_TESTER_CONTRACT)
target_include_directories(
  tester
  PUBLIC $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/tester>
         $<BUILD_INTERFACE:${fmt_SOURCE_DIR}/include>
         $<BUILD_INTERFACE:${catch2_SOURCE_DIR}/single_include>)
target_link_libraries(
  tester
  PUBLIC eosio
  INTERFACE ${EXTRA_TARGETS} $<TARGET_OBJECTS:eosio::tester> )

target_sources(tester INTERFACE $<TARGET_OBJECTS:eosio::crt0>)
target_link_options(tester INTERFACE $<$<PLATFORM_ID:Generic>:-Wl,--entry,start -Wl,--export-table>)

set(embed_SOURCES
    embed/eosio/eosio_assert.cpp embed/eosio/printi.cpp embed/eosio/printn.cpp
    embed/eosio/prints.cpp embed/eosio/printui.cpp)

add_library(embed tester/fpconv.c ${abieos_SOURCE_DIR}/src/crypto.cpp
                        ${embed_SOURCES})
target_include_directories(
  embed PUBLIC $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/embed>)
target_link_libraries(
  embed
  PUBLIC eosio
  INTERFACE ${EXTRA_TARGETS})

set_target_properties(tester embed PROPERTIES PREFIX libeosio_)

