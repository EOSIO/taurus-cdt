set(LLVM_ENABLE_EH 1)

function(add_clang_plugin name src)
  add_llvm_library(${name} PARTIAL_SOURCES_INTENDED MODULE ${src} PLUGIN_TOOL clang)
  target_include_directories(${name} SYSTEM PRIVATE "${LLVM_INCLUDE_DIRS};${CLANG_INCLUDE_DIRS}")

  if (NOT LLVM_ENABLE_RTTI)
    target_compile_options(${name} PRIVATE -fno-rtti)
  endif()

  set_target_properties(${name} PROPERTIES 
    LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

  if(APPLE)
    target_link_libraries(${name} PUBLIC clang-cpp)
  endif()

  install(TARGETS ${name} LIBRARY DESTINATION ${CDT_INSTALL_DIR}/bin)
endfunction()

add_clang_plugin(eosio_attrs custom_attrs.cpp)
add_clang_plugin(eosio_codegen codegen.cpp)

target_include_directories(eosio_codegen
  PRIVATE 
    ${CMAKE_SOURCE_DIR}/tools/include
    ${jsoncons_SOURCE_DIR}/include)

if(NOT TARGET EosioPlugins)
  add_custom_target(EosioPlugins)
  add_dependencies(EosioPlugins eosio_attrs eosio_codegen)
endif()
