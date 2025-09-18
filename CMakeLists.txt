cmake_minimum_required(VERSION 3.10)

project(MandelbrotZoom LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(SOURCES
	src/main.cpp
	src/stb_image_write.cpp
	src/Stopwatch.cpp
)

add_executable(${PROJECT_NAME} ${SOURCES})

find_package(OpenMP REQUIRED)
if(OpenMP_CXX_FOUND)
    target_link_libraries(${PROJECT_NAME} PRIVATE OpenMP::OpenMP_CXX)
endif()