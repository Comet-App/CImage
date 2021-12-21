cmake_minimum_required(VERSION 3.16)
project(CImage)

# Set c++ version
set(CMAKE_CXX_STANDARD 17)

# Build shared libraries by default.
if(NOT BUILD_SHARED_LIBS)
    set(BUILD_SHARED_LIBS ON)
endif()

set(SOURCE_FILES src/main.cpp src/CImage.cpp)

add_executable(CImage ${SOURCE_FILES})

# Setting up magick locations
add_definitions( -DMAGICKCORE_QUANTUM_DEPTH=16 )
add_definitions( -DMAGICKCORE_HDRI_ENABLE=0 )
find_package(ImageMagick COMPONENTS Magick++)
include_directories(${ImageMagick_INCLUDE_DIRS})

target_link_libraries(CImage curl ${ImageMagick_LIBRARIES})
