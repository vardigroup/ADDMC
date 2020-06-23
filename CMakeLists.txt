PROJECT(cmake_addmc) # overwrites 'Makefile' (lower precedence than manually written 'GNUmakefile' wrt 'make')

CMAKE_MINIMUM_REQUIRED(VERSION 2.8.9)

SET(CMAKE_CXX_STANDARD 14)
SET(CMAKE_CXX_STANDARD_REQUIRED ON)

SET(CMAKE_CXX_FLAGS "-g -Ofast -static")

SET(lib_dir ${CMAKE_CURRENT_SOURCE_DIR}/lib)
SET(lib_tar ${lib_dir}.tar)
SET(cudd_dir ${lib_dir}/cudd-3.0.0)

ADD_CUSTOM_COMMAND(
  OUTPUT ${lib_dir}
  DEPENDS ${lib_tar}
  COMMAND tar -xf ${lib_tar} && touch ${lib_dir} && cd ${cudd_dir} && ./configure --enable-silent-rules --enable-obj && make
  WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
)

LINK_DIRECTORIES(${cudd_dir}/cudd/.libs)

INCLUDE_DIRECTORIES(${cudd_dir})
INCLUDE_DIRECTORIES(${cudd_dir}/cudd)
INCLUDE_DIRECTORIES(${cudd_dir}/epd)
INCLUDE_DIRECTORIES(${cudd_dir}/mtr)
INCLUDE_DIRECTORIES(${cudd_dir}/st)

INCLUDE_DIRECTORIES(${lib_dir})

INCLUDE_DIRECTORIES(src/interface)

FILE(GLOB cpp_files "src/implementation/*.cpp")

ADD_EXECUTABLE(addmc ${cpp_files} ${lib_dir})

TARGET_LINK_LIBRARIES(addmc cudd)
