cmake_minimum_required(VERSION 3.10)

project(MandelbrotZoom LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(COMPUTE_SET_SOURCES
	src/compute_set.cpp
	src/Stopwatch.cpp
	src/fixedpoint1024.cpp
)
set(GENERATE_FRAMES_SOURCES
	src/generate_frames.cpp
	src/stb_image_write.cpp
)

add_executable(compute-set ${COMPUTE_SET_SOURCES})
add_executable(generate-frames ${GENERATE_FRAMES_SOURCES})

find_package(OpenMP REQUIRED)
if(OpenMP_CXX_FOUND)
    target_link_libraries(compute-set PRIVATE OpenMP::OpenMP_CXX)
	target_link_libraries(generate-frames PRIVATE OpenMP::OpenMP_CXX)
endif()