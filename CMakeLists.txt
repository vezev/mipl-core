cmake_minimum_required(VERSION 3.20)

project(mipl_sessions
    VERSION 0.1.0
    DESCRIPTION "Extracto del nucleo de Mipl: gestion de sesiones de juego"
    LANGUAGES CXX
)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

include(CTest)

# Logica pura: no depende de Discord ni de configuracion local.
add_library(mipl_sessions STATIC
    src/core/session_manager.cpp
)

target_include_directories(mipl_sessions PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/include
)

target_compile_options(mipl_sessions PRIVATE -Wall -Wextra -Wpedantic)

if(BUILD_TESTING)
    add_executable(session_manager_tests tests/session_manager_tests.cpp)
    target_link_libraries(session_manager_tests PRIVATE mipl_sessions)
    target_compile_options(session_manager_tests PRIVATE -Wall -Wextra -Wpedantic)
    add_test(NAME session_manager_tests COMMAND session_manager_tests)
endif()
