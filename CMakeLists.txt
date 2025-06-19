cmake_minimum_required(VERSION 3.10)
project(BinaryMaskEstimator)

# Set C++ standard
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Add compiler flags for better debugging and warnings
set(CMAKE_CXX_FLAGS_DEBUG "-g -O0 -Wall -Wextra -pedantic")
set(CMAKE_CXX_FLAGS_RELEASE "-O3 -DNDEBUG")

# Set default build type to Release if not specified
if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release)
endif()

# Find OpenCV package - try different methods for Rocky Linux
find_package(OpenCV QUIET)

if(NOT OpenCV_FOUND)
    # Try common Rocky Linux installation paths
    set(OpenCV_PATHS
        /usr/local/lib64/cmake/opencv4
        /usr/lib64/cmake/opencv4
        /usr/local/share/opencv4
        /usr/share/opencv4
    )
    
    foreach(path ${OpenCV_PATHS})
        if(EXISTS ${path})
            set(OpenCV_DIR ${path})
            find_package(OpenCV QUIET)
            if(OpenCV_FOUND)
                break()
            endif()
        endif()
    endforeach()
endif()

if(NOT OpenCV_FOUND)
    message(FATAL_ERROR "OpenCV not found! Please install OpenCV development packages:
    sudo dnf install opencv-devel opencv-contrib-devel
    
    Or if built from source, set OpenCV_DIR to point to your OpenCV build directory:
    cmake -DOpenCV_DIR=/path/to/opencv/build ..")
endif()


# Print OpenCV information
message(STATUS "OpenCV library status:")
message(STATUS "    version: ${OpenCV_VERSION}")
message(STATUS "    libraries: ${OpenCV_LIBS}")
message(STATUS "    include path: ${OpenCV_INCLUDE_DIRS}")

# Include OpenCV headers
include_directories(${OpenCV_INCLUDE_DIRS})

# Add your lib directory to include path
include_directories(lib)

# Define source files
set(SOURCES
    src/binaryMaskEstimator.cpp
    src/objectCounter.cpp 
)

set(HEADERS
    lib/binaryMaskEstimator.hh
    lib/objectCounter.hh
)

# Create the main executable
add_executable(${PROJECT_NAME} 
    src/main.cpp 
    ${SOURCES}
)

# Link OpenCV libraries
target_link_libraries(${PROJECT_NAME} ${OpenCV_LIBS})

# Add compile definitions for OpenCV version compatibility
if(OpenCV_VERSION VERSION_GREATER_EQUAL "4.0")
    target_compile_definitions(${PROJECT_NAME} PRIVATE OPENCV_VERSION_4)
endif()

# Set output directory
set_target_properties(${PROJECT_NAME} PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin
)

# Installation rules
install(TARGETS ${PROJECT_NAME} 
    RUNTIME DESTINATION bin
    COMPONENT runtime
)

install(FILES ${HEADERS}
    DESTINATION include
    COMPONENT development
)


# Create sample usage documentation
file(WRITE ${CMAKE_BINARY_DIR}/README_BUILD.txt
"Build Instructions:
==================

1. Create build directory:
   mkdir build && cd build

2. Configure the project:
   cmake ..
   
   If OpenCV is not found automatically, specify the path:
   cmake -DOpenCV_DIR=/usr/local/lib64/cmake/opencv4 ..

3. Build the project:
   make

4. Run the program:
   ./bin/BinaryMaskEstimator <input_image_path> [output_mask_path]

Example:
   ./bin/BinaryMaskEstimator ../test_image.jpg output_mask.png

Dependencies:
=============
- OpenCV 3.4+ (recommended: OpenCV 4.x)
- CMake 3.10+
- C++11 compatible compiler

For Rocky Linux:
   sudo dnf install opencv-devel opencv-contrib-devel cmake gcc-c++
")

# Print build information
message(STATUS "Build type: ${CMAKE_BUILD_TYPE}")
message(STATUS "Compiler: ${CMAKE_CXX_COMPILER}")
message(STATUS "Build flags: ${CMAKE_CXX_FLAGS_${CMAKE_BUILD_TYPE}}")
message(STATUS "")
message(STATUS "To build:")
message(STATUS "  mkdir build && cd build")
message(STATUS "  cmake ..")
message(STATUS "  make")
message(STATUS "")
message(STATUS "To run:")
message(STATUS "  ./bin/BinaryMaskEstimator <input_image> [output_mask]")
