# Defines builtins library

set(builtins_sources
    compiler_builtins.cpp
    fixtfti.c
    fixunstfti.c
    fixsfti.c
    fixdfti.c
    fixunssfti.c
    fixunsdfti.c
    floattidf.c
    floatuntidf.c
    multi3.c
    ashlti3.c
    ashrti3.c
    lshrti3.c
    divti3.c
    udivti3.c
    modti3.c
    umodti3.c)

add_library(rt STATIC ${builtins_sources})
set_target_properties(rt PROPERTIES PREFIX libeosio_)
target_include_directories(
  rt PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}
                   ${softfloat_SOURCE_DIR}/source/include)
target_compile_options(rt PRIVATE -Wall)

