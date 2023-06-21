
if (CMAKE_SYSTEM_PROCESSOR MATCHES "^wasm")
  set(IS_WASM_TARGET ON)
else()
  set(CMAKE_CXX_VISIBILITY_PRESET hidden)
endif()
cmake_policy(SET CMP0116 NEW)

separate_arguments(EOSIO_CXX_OPTION_LIST UNIX_COMMAND ${EOSIO_CXX_OPTIONS})
define_property(TARGET PROPERTY WASM_STACK_SIZE BRIEF_DOCS "wasm stack size" FULL_DOCS "wasm stack size")
define_property(TARGET PROPERTY PROTOBUF_FILES BRIEF_DOCS "protobuf files" FULL_DOCS "protobuf files used by the contract")
define_property(TARGET PROPERTY PROTOBUF_DIR BRIEF_DOCS "protobuf root directory" FULL_DOCS "protobuf root directory")


function(set_contract_stack_size target size)
  if(IS_WASM_TARGET)
    set_target_properties(${target} PROPERTIES WASM_STACK_SIZE ${size})
  endif()
endfunction()

function(contract_codegen TARGET CONTRACT_NAME)
  set(OUTPUT_DIR ${CMAKE_CURRENT_BINARY_DIR}/${CONTRACT_NAME}.dir)
  file(MAKE_DIRECTORY ${OUTPUT_DIR})

  foreach(src ${ARGN})
    cmake_path(GET src FILENAME src_filename)
    cmake_path(GET src PARENT_PATH src_parent)
    list(APPEND actions ${OUTPUT_DIR}/${src_filename}.actions.cpp)
    list(APPEND parents ${src_parent})
    cmake_path(ABSOLUTE_PATH src NORMALIZE OUTPUT_VARIABLE abs_path)
    list(APPEND inputs ${abs_path})
  endforeach()

  list(REMOVE_DUPLICATES parents)
  if (CMAKE_HOST_SYSTEM_NAME STREQUAL "Darwin")
    set(HOST_SHARED_LIBRARY_SUFFIX ".dylib")
  else()
    set(HOST_SHARED_LIBRARY_SUFFIX ".so")
  endif()

  set(DEPFILE ${CONTRACT_NAME}.dir/${CONTRACT_NAME}.abi.d)
  set(CXX_OPTIONS ${EOSIO_CXX_OPTION_LIST} -MD -MT ${CONTRACT_NAME}.dir/${CONTRACT_NAME}.abi -MF ${CMAKE_CURRENT_BINARY_DIR}/${DEPFILE})

  add_custom_command(
    OUTPUT ${OUTPUT_DIR}/${CONTRACT_NAME}.abi ${OUTPUT_DIR}/${CONTRACT_NAME}.dispatch.cpp ${actions}
    COMMAND ${CMAKE_FIND_ROOT_PATH}/bin/eosio-codegen ${inputs} --contract ${CONTRACT_NAME} 
            -D '$<TARGET_PROPERTY:${TARGET},COMPILE_DEFINITIONS>' 
            -I '$<TARGET_PROPERTY:${TARGET},INCLUDE_DIRECTORIES>'
            --cxx '${CXX_OPTIONS}'
            --output-dir "${OUTPUT_DIR}"
            --protobuf-dir '$<TARGET_PROPERTY:${TARGET},PROTOBUF_DIR>'
            --protobuf-files '$<TARGET_PROPERTY:${TARGET},PROTOBUF_FILES>'
            $<TARGET_PROPERTY:${TARGET},EOSIO_CODEGEN_OPTIONS>
    DEPFILE ${DEPFILE}
    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
    DEPENDS ${ARGN} ${CMAKE_FIND_ROOT_PATH}/bin/eosio-codegen 
            ${CMAKE_FIND_ROOT_PATH}/bin/eosio_codegen${HOST_SHARED_LIBRARY_SUFFIX} 
            ${CMAKE_FIND_ROOT_PATH}/bin/eosio_attrs${HOST_SHARED_LIBRARY_SUFFIX} 
            ${CMAKE_TOOLCHAIN_FILE}
            $<TARGET_PROPERTY:${TARGET},CONTRACT_PROTOBUF>
    COMMENT Generating ${OUTPUT_DIR}/${CONTRACT_NAME}.abi ${OUTPUT_DIR}/${CONTRACT_NAME}.dispatch.cpp ${actions}
  )

  target_sources(${TARGET} PRIVATE ${OUTPUT_DIR}/${CONTRACT_NAME}.dispatch.cpp ${actions})
  if (parents)
    target_include_directories(${TARGET} PRIVATE ${parents}) ## Required for CLion intellisense
  endif()
  set_source_files_properties(${ARGN} PROPERTIES HEADER_FILE_ONLY TRUE)

  add_custom_command(TARGET ${TARGET} POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy ${OUTPUT_DIR}/${CONTRACT_NAME}.abi $<TARGET_FILE_DIR:${TARGET}>/${TARGET}.abi
  )
endfunction()

function(add_module TARGET)
  if(IS_WASM_TARGET)
    add_executable(${TARGET} ${ARGN})
    add_custom_command(TARGET ${TARGET} POST_BUILD
      COMMAND ${CMAKE_FIND_ROOT_PATH}/bin/eosio-pp --profile=eosio $<TARGET_FILE:${TARGET}>
    )
    set_target_properties(${TARGET} PROPERTIES WASM_STACK_SIZE 8192)
    target_link_options(${TARGET} PRIVATE -Wl,-zstack-size=$<TARGET_PROPERTY:WASM_STACK_SIZE>)
    target_compile_options(${TARGET} PUBLIC -nobuiltininc)
  else()
    add_library(${TARGET} MODULE ${ARGN})
    target_link_options(${TARGET} PRIVATE $<$<PLATFORM_ID:Linux>:-Wl,--allow-shlib-undefined> $<$<PLATFORM_ID:Darwin>:-Wl,-undefined,dynamic_lookup>)
  endif()
endfunction()

macro(add_contract CONTRACT_NAME TARGET)
  add_module(${TARGET} ${ARGN})
  target_link_libraries(${TARGET} PRIVATE eosio::eosio)
  if (IS_WASM_TARGET)
    target_link_options(${TARGET}  PRIVATE -Wl,--entry,apply)
  endif()
  if (${ARGC} GREATER 2)
    contract_codegen(${TARGET} ${CONTRACT_NAME} ${ARGN})
  endif()
endmacro()

macro(target_ricardian_directory TARGET DIR)
  file(GLOB contracts ${DIR}/*.contracts.md ${DIR}/*.clauses.md)
  add_custom_target(${TARGET}.ricardian DEPENDS ${contracts})
endmacro()

# Generate C++ headers for protobuf files and tie to a given target
#
#    target_add_protobuf(target 
#                        INPUT_DIRECTORY input_dir
#                        OUTPUT_DIRECTORY output_dir
#                        FILES file [file1 ...])
#
# Given a list of .proto file, this function genernates the corresponding C++ headers,
# and set the generated files as sources the the specified target. 
#
# If the input_dir is not specified, the default is ${CMAKE_CURRENT_SOURCE_DIR} and 
# the files specified must be relative paths to the input_dir. 
#
function(target_add_protobuf TARGET)
  cmake_parse_arguments(ADD_PROTOBUF "" "INPUT_DIRECTORY;OUTPUT_DIRECTORY" "FILES" ${ARGN})

  if (IS_ABSOLUTE ${ADD_PROTOBUF_OUTPUT_DIRECTORY})
    message(FATAL_ERROR "The OUTPUT_DIRECTORY for function contract_add_protobuf must be a relative directory")
  endif()

  if (ADD_PROTOBUF_OUTPUT_DIRECTORY)
    set(OUTPUT_DIR  ${CMAKE_CURRENT_BINARY_DIR}/${ADD_PROTOBUF_OUTPUT_DIRECTORY})
  else()
    set(OUTPUT_DIR  ${CMAKE_CURRENT_BINARY_DIR})
  endif()

  foreach (protofile ${ADD_PROTOBUF_FILES})
    if(IS_ABSOLUTE ${protofile})
      message(FATAL_ERROR "The FILES parameters for function contract_add_protobuf must be relative paths, illegal path `${protofile}`")
    endif()
    cmake_path(GET protofile EXTENSION LAST_ONLY proto_ext)
    if (NOT proto_ext STREQUAL ".proto")
      message(FATAL_ERROR "The illegal parameter `${protofile}` for function contract_add_protobuf")
    endif()
    cmake_path(REPLACE_EXTENSION protofile LAST_ONLY ".pb.hpp" OUTPUT_VARIABLE hdr)
    list(APPEND OUTPUT_HDRS ${OUTPUT_DIR}/${hdr})
    if (ADD_PROTOBUF_INPUT_DIRECTORY)
      list(APPEND INPUT_FILES ${ADD_PROTOBUF_INPUT_DIRECTORY}/${protofile})
    else()
      list(APPEND INPUT_FILES ${CMAKE_CURRENT_SOURCE_DIR}/${protofile})
    endif()
  endforeach()

  if (NOT ADD_PROTOBUF_INPUT_DIRECTORY)
    set(ADD_PROTOBUF_INPUT_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR})
  endif()

  find_program(PROTOC protoc
    HINTS ${CMAKE_FIND_ROOT_PATH}/bin /usr/local/bin
    REQUIRED)

  if (CMAKE_GENERATOR STREQUAL "Unix Makefiles" AND NOT EXISTS ${OUTPUT_DIR})
    file(MAKE_DIRECTORY ${OUTPUT_DIR})
  endif()

  add_custom_command(
    COMMENT Generating ${OUTPUT_HDRS} from ${INPUT_FILES}
    OUTPUT ${OUTPUT_HDRS} 
    COMMAND ${PROTOC} -I ${ADD_PROTOBUF_INPUT_DIRECTORY} -I ${CMAKE_FIND_ROOT_PATH}/include --plugin=${CMAKE_FIND_ROOT_PATH}/bin/protoc-gen-zpp --zpp_out ${OUTPUT_DIR} ${ADD_PROTOBUF_FILES}
    DEPENDS ${CMAKE_FIND_ROOT_PATH}/bin/protoc-gen-zpp ${INPUT_FILES}
    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
  )

  add_custom_target(${TARGET}.protos ALL
    DEPENDS ${OUTPUT_HDRS} 
  )

  get_target_property(type ${TARGET} TYPE)
  if ("${type}" STREQUAL "INTERFACE_LIBRARY")
    target_sources(${TARGET} INTERFACE ${OUTPUT_HDRS})
    target_include_directories(${TARGET} INTERFACE ${CMAKE_CURRENT_BINARY_DIR})
  else()
    target_sources(${TARGET} PRIVATE ${OUTPUT_HDRS})
    target_include_directories(${TARGET} PUBLIC ${CMAKE_CURRENT_BINARY_DIR})
    add_dependencies(${TARGET} ${TARGET}.protos)
  endif()

  set_target_properties(${TARGET} PROPERTIES 
    PROTOBUF_DIR "${ADD_PROTOBUF_INPUT_DIRECTORY}"
    PROTOBUF_FILES "${ADD_PROTOBUF_FILES}")
endfunction()

function(contract_use_protobuf CONTRACT_TARGET PROTOBUF_TARGET)
  get_target_property(PROTO_DIR ${PROTOBUF_TARGET} PROTOBUF_DIR)
  get_target_property(PROTO_FILES ${PROTOBUF_TARGET} PROTOBUF_FILES)
  set_target_properties(${CONTRACT_TARGET} PROPERTIES 
    PROTOBUF_DIR ${PROTO_DIR}
    PROTOBUF_FILES "${PROTO_FILES}"
  )
endfunction()
