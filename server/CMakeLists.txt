add_executable(server main.cpp connection.cpp)

include_directories(${CMAKE_SOURCE_DIR}/client) # include files from client

find_package(nlohmann_json CONFIG REQUIRED)
target_link_libraries(server PRIVATE nlohmann_json::nlohmann_json)