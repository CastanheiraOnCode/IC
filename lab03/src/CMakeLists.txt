cmake_minimum_required(VERSION 3.16)
project(guiao3)
find_package( OpenCV REQUIRED )
include_directories( ${OpenCV_INCLUDE_DIRS} )

SET (CMAKE_BUILD_TYPE "Release")
#SET (CMAKE_BUILD_TYPE "Debug")

SET (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra -std=c++17")
SET (CMAKE_CXX_FLAGS_RELEASE "-O3")
SET (CMAKE_CXX_FLAGS_DEBUG "-g3 -fsanitize=address")

SET (BASE_DIR ${CMAKE_SOURCE_DIR} )
SET (CMAKE_RUNTIME_OUTPUT_DIRECTORY ${BASE_DIR}/../bin)

add_executable (videoEncoderIntra videoEncoderIntra.cpp)
target_link_libraries( videoEncoderIntra ${OpenCV_LIBS} )

add_executable (videoDecoderIntra videoDecoderIntra.cpp)
target_link_libraries( videoDecoderIntra ${OpenCV_LIBS} )

add_executable (videoEncoderMC videoEncoderMC.cpp)
target_link_libraries( videoEncoderMC ${OpenCV_LIBS} )

add_executable (videoDecoderMC videoDecoderMC.cpp)
target_link_libraries( videoDecoderMC ${OpenCV_LIBS} )

add_executable (videoEncoderQuantized videoEncoderQuantized.cpp)
target_link_libraries( videoEncoderQuantized ${OpenCV_LIBS} )

add_executable (video_cmp video_cmp.cpp)
target_link_libraries( video_cmp ${OpenCV_LIBS} )












