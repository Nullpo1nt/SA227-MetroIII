# specify the C standard
set(CMAKE_C_STANDARD 11)

set(THREADS_PREFER_PTHREAD_FLAG ON)
set(CURSES_NEED_NCURSES True)

add_executable(engine test.c pid.c idealgas.c)

find_package(Curses REQUIRED)
find_package(Threads REQUIRED)

target_include_directories(engine PRIVATE ${CURSES_INCLUDE_DIR})

target_link_libraries(engine m)
target_link_libraries(engine Threads::Threads)
target_link_libraries(engine ${CURSES_LIBRARIES})