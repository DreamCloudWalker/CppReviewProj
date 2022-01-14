cmake_minimum_required(VERSION 3.20)
project(pc)

set(CMAKE_CXX_STANDARD 11)
set(MY_LIBRARY_DIR /usr/local/Cellar)
set(SDL_DIR ${MY_LIBRARY_DIR}/sdl2/2.0.18)
set(FFMPEG_DIR ${MY_LIBRARY_DIR}/ffmpeg/4.4_2)
include_directories(${FFMPEG_DIR}/include/
        ${SDL_DIR}/include/SDL2/)
link_directories(${FFMPEG_DIR}/lib/
        ${SDL_DIR}/lib/)
find_package(Threads REQUIRED)

add_executable(utils test_utils.cpp ../../lib/utils/test_shared_ptr.h ../../lib/utils/simple_ring_buffer.h)
add_executable(algorithm fuck_algorithm.cpp)
add_executable(ffmpeg test_ffmpeg.cpp)

#link_libraries(SDL2)

target_link_libraries(ffmpeg PRIVATE SDL2
        avcodec
        avdevice
        avfilter
        avformat) # 链接目标库