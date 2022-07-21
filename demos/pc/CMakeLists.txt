cmake_minimum_required(VERSION 3.20)
project(pc)

set(CMAKE_CXX_STANDARD 11)
set(MY_LIBRARY_DIR /usr/local/Cellar)
set(SDL_DIR ${MY_LIBRARY_DIR}/sdl2/2.0.18)
set(FFMPEG_DIR ${MY_LIBRARY_DIR}/ffmpeg/4.4_2)
set(BOOST_DIR ${MY_LIBRARY_DIR}/boost/1.76.0)

include_directories(
        ${FFMPEG_DIR}/include/
        ${SDL_DIR}/include/SDL2/
        ${BOOST_DIR}/include/
)
link_directories(
        ${FFMPEG_DIR}/lib/
        ${SDL_DIR}/lib/
        ${BOOST_DIR}/lib/
)
find_package(Threads REQUIRED)

add_executable(utils test_utils.cpp ../../lib/utils/test_shared_ptr.h ../../lib/utils/simple_ring_buffer.h)
add_executable(algorithm fuck_algorithm.cpp)
add_executable(ffmpeg test_ffmpeg.cpp)
add_executable(multi-thread test_multithread.cpp)
add_executable(coroutines test_coroutines.cpp)
add_executable(fileio test_fileio.cpp)
add_executable(log test_log.cpp ../../lib/utils/raii_log.h ../../lib/utils/raii_log.cpp)

#link_libraries(SDL2)

target_link_libraries(ffmpeg PRIVATE SDL2
        avcodec
        avdevice
        avfilter
        avformat) # 链接目标库
target_link_libraries(coroutines boost_thread-mt boost_system boost_coroutine boost_context-mt)