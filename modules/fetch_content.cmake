
include(FetchContent)

Set(FETCHCONTENT_QUIET FALSE)

macro(fetch_content name)
   if (NOT ${name}_SOURCE_DIR)
      FetchContent_Declare(
         ${name}
         ${ARGN}
      )
      FetchContent_GetProperties(${name})
      if(NOT ${name}_POPULATED)
         FetchContent_Populate(${name})
      endif()
   endif()
endmacro()

macro(add_content name)
   FetchContent_Declare(
      ${name}
      ${ARGN}
   )
   FetchContent_MakeAvailable(${name})
endmacro()