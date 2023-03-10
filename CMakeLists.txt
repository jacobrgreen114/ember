# Copyright (C) 2022-2023 Jacob R. Green
# All Rights Reserved.

find_package(CMakeModule REQUIRED MODULE)
add_module_path("${CMAKE_CURRENT_SOURCE_DIR}/cmake")

set(CMAKE_C_STANDARD 23)
set(CMAKE_CXX_STANDARD 23)
set(CMAKE_INTERPROCEDURAL_OPTIMIZATION ON)

add_executable(ember
        src/main.cpp
)

target_link_libraries(ember
    PUBLIC
        rapidxml
        CLI11
)