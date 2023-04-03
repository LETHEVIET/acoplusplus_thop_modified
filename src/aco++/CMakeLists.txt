cmake_minimum_required(VERSION 3.0.0)
project(aco++_first_improvement_hill_climbing VERSION 0.1.0)

include(CTest)
enable_testing()

add_executable(aco++_first_improvement_hill_climbing main.cpp)

set(CPACK_PROJECT_NAME ${PROJECT_NAME})
set(CPACK_PROJECT_VERSION ${PROJECT_VERSION})
include(CPack)
