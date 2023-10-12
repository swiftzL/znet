project(znet)
cmake_minimum_required(VERSION 3.22)
aux_source_directory(./src DIR_SRCS)
add_definitions(-std=c++11)
# 指定生成目标文件
add_executable(znet ${DIR_SRCS})

link_directories(${cmake_CURRENT_SOURCE_DIR}/3rd/lua-5.3.5/src/)

#Lua头文件、库

include_directories(./3rd/lua-5.3.5/src)
target_link_libraries(znet liblua.a)
target_link_libraries(znet dl)

target_link_libraries(znet -lpthread)
add_definitions("-Wall -g -Wunused-variable -Wunused-but-set-variable")
target_include_directories(znet PRIVATE ${PROJECT_SOURCE_DIR}/include)


