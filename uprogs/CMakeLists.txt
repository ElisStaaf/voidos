cmake_minimum_required(VERSION 3.5)

project(uprogs LANGUAGES CXX ASM C)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_COMPILER clang++)
set(CMAKE_SYSTEM_PROCESSOR i386)
set(CMAKE_LINKER ld)
# Override defautl linker executable (pattern to link an executable, not
# /linker executable/ itself), by default it uses `<CMAKE_CXX_COMPILER>`
# instead of a linker
set(CMAKE_CXX_LINK_EXECUTABLE
    "<CMAKE_LINKER> <FLAGS> <CMAKE_CXX_LINK_FLAGS> <LINK_FLAGS> <OBJECTS> -o <TARGET> <LINK_LIBRARIES>"
)

set(XV6_CFLAGS @${CMAKE_SOURCE_DIR}/../cflags.txt)
set(XV6_LDFLAGS @${CMAKE_SOURCE_DIR}/../ldflags.txt)
set(XV6_INCLUDE ${CMAKE_SOURCE_DIR}/..)
set(XV6_USER_INCLUDE ${CMAKE_SOURCE_DIR}/lib)

add_library(ulib lib/umalloc.cpp lib/printf.cpp lib/ulib.cpp lib/usys.S)
target_compile_options(ulib PRIVATE ${XV6_CFLAGS})
target_include_directories(ulib PRIVATE ${XV6_INCLUDE} ${XV6_USER_INCLUDE})

file(GLOB SRC_FILES ${CMAKE_SOURCE_DIR}/*.cpp)
foreach(file ${SRC_FILES})
  get_filename_component(prog ${file} NAME_WE)
  string(PREPEND prog "_")
  message(STATUS "${prog} using ${file}")
  add_executable(${prog} ${file})

  target_link_libraries(${prog} ulib)
  target_compile_options(${prog} PRIVATE ${XV6_CFLAGS} -nostdinc)
  target_link_options(
    ${prog}
    PRIVATE
    ${XV6_LDFLAGS}
    # Make code and data sections readable and writable QUESTION why enable
    # this (it gives warnings by default)
    -N
    --no-warn-rwx-segments # Disable default warnings
    # Override default entry point to main
    -e
    main
    # Put text section in the specified location QUESTION why is this
    # needed, and why put things at zero?
    -Ttext
    0)
  target_include_directories(${prog} PRIVATE ${XV6_INCLUDE}
                                             ${XV6_USER_INCLUDE})
  set_target_properties(${prog} PROPERTIES RUNTIME_OUTPUT_DIRECTORY
                                           ${CMAKE_SOURCE_DIR}/exe)
endforeach()
