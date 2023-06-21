
set(SRC_FILENAMES algorithm.cpp any.cpp bind.cpp charconv.cpp chrono.cpp condition_variable.cpp functional.cpp
                  future.cpp ios.cpp ios.instantiations.cpp iostream.cpp locale.cpp memory.cpp mutex.cpp new.cpp optional.cpp
                  regex.cpp stdexcept.cpp string.cpp strstream.cpp system_error.cpp exception.cpp
                  typeinfo.cpp utility.cpp valarray.cpp variant.cpp vector.cpp hash.cpp)

set(LIBCXX_SRC_DIR ${llvm_SOURCE_DIR}/libcxx/src/)
set(LIBCXXABI_SRC_DIR ${llvm_SOURCE_DIR}/libcxxabi/src/)

set(_LIBCPP_HAS_NO_THREADS ON)
set(_LIBCPP_HAS_MUSL_LIBC ON)
set(_LIBCPP_DISABLE_VISIBILITY_ANNOTATIONS ON)
set(_LIBCPP_HAS_NO_VENDOR_AVAILABILITY_ANNOTATIONS ON)
set(_LIBCPP_ABI_DEFINES "#define _LIBCPP_NO_EXCEPTIONS")
configure_file(${llvm_SOURCE_DIR}/libcxx/include/__config_site.in ${CMAKE_CURRENT_BINARY_DIR}/include/__config_site)

add_library(c++ "cxa_virtual_fallback.cpp" 
           ${LIBCXXABI_SRC_DIR}/private_typeinfo.cpp
           ${LIBCXXABI_SRC_DIR}/cxa_demangle.cpp
           ${LIBCXX_SRC_DIR}$<JOIN:${SRC_FILENAMES}, ${LIBCXX_SRC_DIR}>)
target_compile_options(c++ PUBLIC -Wno-everything -frtti)
target_compile_definitions(c++ PRIVATE _LIBCPP_BUILDING_LIBRARY _LIBCXXABI_BUILDING_LIBRARY)
target_link_libraries(c++ PUBLIC cstdio c)
target_include_directories(c++ PRIVATE ${CMAKE_CURRENT_BINARY_DIR}/include
                                       ${llvm_SOURCE_DIR}/libcxx/include
                                       ${llvm_SOURCE_DIR}/libcxxabi/include
                                       ${LIBCXX_SRC_DIR})

install(FILES ${CMAKE_CURRENT_BINARY_DIR}/include/__config_site DESTINATION include/${CMAKE_SYSTEM_PROCESSOR}/c++/v1) 
install(DIRECTORY ${llvm_SOURCE_DIR}/libcxx/include/ DESTINATION include/${CMAKE_SYSTEM_PROCESSOR}/c++/v1) 
install(DIRECTORY ${llvm_SOURCE_DIR}/libcxxabi/include/ DESTINATION include/${CMAKE_SYSTEM_PROCESSOR}/c++/v1) 
install(TARGETS c++ DESTINATION lib/${CMAKE_SYSTEM_PROCESSOR})