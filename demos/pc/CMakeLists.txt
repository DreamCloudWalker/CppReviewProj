cmake_minimum_required(VERSION 3.20)
project(pc)

set(CMAKE_CXX_STANDARD 11)
set(SDL_DIR /usr/local/Cellar/sdl2/2.0.18/)     # 这个SDL开发包的路径，可以通过brew info sdl2查出来
include_directories(${SDL_DIR}/include/)        # 添加SDL头文件搜索路径
link_directories(${SDL_DIR}/lib/)                 # 增加SDL链接库目录

add_executable(pc main.cpp ../../lib/utils/test_shared_ptr.h ../../lib/utils/simple_ring_buffer.h)

link_libraries(SDL2)
target_link_libraries(pc PRIVATE SDL2 SDL2_test SDL2main) # 链接目标库