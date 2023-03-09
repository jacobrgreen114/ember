
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_CURRENT_SOURCE_DIR}/cmake")

add_executable(ember
        src/main.cpp
)

target_link_libraries(ember
    PUBLIC
        rapidxml
)