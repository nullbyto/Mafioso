add_executable(client main.cpp game.cpp connection.cpp)

include_directories(${CMAKE_SOURCE_DIR}/server) # include files from server

find_package(ftxui CONFIG REQUIRED)

target_link_libraries(client
    PRIVATE ftxui::component
)