cmake_minimum_required(VERSION 3.15)
project(http_server C)

set(CMAKE_C_STANDARD 11)

add_executable(http_server src/main.c src/main.h src/logger.h src/logger.c src/utils.c src/utils.h src/epoll.c src/epoll.h)