cmake_minimum_required(VERSION 2.8.7)

# faiss project
project(faiss C CXX)

option(BUILD_TUTORIAL "Build tutorials" ON)
option(BUILD_TEST "Build tests" ON)
option(BUILD_WITH_GPU "Build faiss with gpu (cuda) support" ON)

list(APPEND CMAKE_MODULE_PATH ${PROJECT_SOURCE_DIR}/cmake/Modules)

# OpenMP
find_package(OpenMP REQUIRED)

# openblas
find_package(BLAS REQUIRED)
#include_directories(${OpenBLAS_INCLUDE_DIR})
if (BLAS_FOUND)
message(STATUS "Found BLAS: ${BLAS_LIBRARIES}")
endif()

find_package(LAPACK REQUIRED)
if (LAPACK_FOUND)
message(STATUS "Found LAPACK: ${LAPACK_LIBRARIES}")
endif()

if (UNIX)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11 -fPIC -m64 -Wall -g -O3  -msse4 -mpopcnt -fopenmp -Wno-sign-compare")
else()
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /D NOMINMAX /openmp")
endif()
add_definitions(-DFINTEGER=int)

# specify output bin_path and lib_path
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

# specify header and cpp files
file(GLOB faiss_cpu_headers ${CMAKE_CURRENT_SOURCE_DIR}/*.h)
file(GLOB faiss_cpu_cpp ${CMAKE_CURRENT_SOURCE_DIR}/*.cpp)

set(faiss_lib faiss)
add_library(${faiss_lib} STATIC ${faiss_cpu_headers} ${faiss_cpu_cpp})
if(UNIX)
target_link_libraries(${faiss_lib} ${OpenMP_CXX_FLAGS} ${OpenBLAS_LIB})
else()
find_path(MMAN_DIR NAMES sys/mman.h PATH_SUFFIXES mman)
find_library(MMAN_LIB NAMES mman)
#find_library(OpenBLAS_LIB NAMES openblas)
find_package(pthreads REQUIRED)
target_include_directories(${faiss_lib} PRIVATE ${MMAN_DIR} ${PTHREADS_INCLUDE_DIR} ${CLAPACK_INCLUDE_DIR})
target_link_libraries(${faiss_lib} ${OpenMP_CXX_FLAGS} ${PTHREADS_LIBRARY} ${MMAN_LIB} ${LAPACK_LIBRARIES})
endif()

# build gpu lib
if(BUILD_WITH_GPU)
    include(cmake/Cuda.cmake)
    add_subdirectory(gpu)
endif(BUILD_WITH_GPU)

# build tutorial examples
if(BUILD_TUTORIAL)
    add_subdirectory(tutorial)
endif(BUILD_TUTORIAL)

# build tests
if(BUILD_TEST)
    add_subdirectory(tests)
endif(BUILD_TEST)
