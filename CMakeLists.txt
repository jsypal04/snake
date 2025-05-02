cmake_minimum_required(VERSION 3.28)
project(snake)

include_directories(${CMAKE_SOURCE_DIR})

add_executable(snake
    main.cpp
    game.cpp
    snake.cpp
    glad.c
)

target_link_libraries(snake
    ${CMAKE_SOURCE_DIR}/lib/libglfw3.a
)

add_executable(tests
    tests.cpp
    game.cpp
    snake.cpp
    glad.c
)

target_link_libraries(tests
    ${CMAKE_SOURCE_DIR}/lib/libglfw3.a
)
