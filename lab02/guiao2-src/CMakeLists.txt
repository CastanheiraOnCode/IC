cmake_minimum_required(VERSION 3.16)
project(guiao2)
find_package( OpenCV REQUIRED )
include_directories( ${OpenCV_INCLUDE_DIRS} )

SET (CMAKE_BUILD_TYPE "Release")
#SET (CMAKE_BUILD_TYPE "Debug")

SET (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra -std=c++17")
SET (CMAKE_CXX_FLAGS_RELEASE "-O3")
SET (CMAKE_CXX_FLAGS_DEBUG "-g3 -fsanitize=address")

SET (BASE_DIR ${CMAKE_SOURCE_DIR} )
SET (CMAKE_RUNTIME_OUTPUT_DIRECTORY ${BASE_DIR}/../guiao2-bin)

add_executable (ppm_copy ppm_copy.cpp)
target_link_libraries( ppm_copy ${OpenCV_LIBS} )

add_executable (ppm_effects ppm_effects.cpp)
target_link_libraries( ppm_effects ${OpenCV_LIBS} )

add_executable (golombTest golombTest.cpp)
target_link_libraries( golombTest ${OpenCV_LIBS} )

add_executable (golombImgEncoder golombImgEncoder.cpp)
target_link_libraries( golombImgEncoder ${OpenCV_LIBS} )

add_executable (encoder encoder.cpp)
target_link_libraries( encoder ${OpenCV_LIBS} sndfile)

add_executable (decoder decoder.cpp)
target_link_libraries( decoder ${OpenCV_LIBS} sndfile)

add_executable (golombImgDecoder golombImgDecoder.cpp)
target_link_libraries( golombImgDecoder ${OpenCV_LIBS} )




