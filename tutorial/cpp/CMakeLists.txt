file(GLOB srcs ${CMAKE_CURRENT_SOURCE_DIR}/*.cpp)

# Build each source file independently
include_directories(../../../)	# faiss root directory

foreach(source ${srcs})
    get_filename_component(name ${source} NAME_WE)

    # target
    add_executable(${name} ${source})
if(UNIX)
    target_link_libraries(${name} ${faiss_lib})
else()
    target_link_libraries(${name} ${faiss_lib} ${PTHREADS_LIBRARY})
endif()

    # Install
    install(TARGETS ${name} DESTINATION bin)
endforeach(source)
