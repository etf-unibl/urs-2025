cmake_minimum_required(VERSION 3.10)
project(sqlite_test C)

find_package(SQLite3 REQUIRED)

add_executable(sqlite-test sqlite-test.c)

target_link_libraries(sqlite-test PRIVATE SQLite::SQLite3)