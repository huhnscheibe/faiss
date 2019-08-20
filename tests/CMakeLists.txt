file(GLOB srcs ${CMAKE_CURRENT_SOURCE_DIR}/*.cpp)

# Build each source file independently
include_directories(../../)	# faiss root directory

# gtest
find_package(GTest REQUIRED)
include_directories(${GTEST_INCLUDE_DIRS})

set(name tests)
add_executable(${name} ${srcs})
if(UNIX)
    target_link_libraries(${name} ${faiss_lib} ${OpenBLAS_LIB} ${GTEST_BOTH_LIBRARIES})
else()
    target_link_libraries(${name} ${faiss_lib} ${GTEST_BOTH_LIBRARIES} ${PTHREADS_LIBRARY} ${LAPACK_LIBRARIES})
endif()
# Install
install(TARGETS ${name} DESTINATION test)

