cmake_minimum_required(VERSION 3.20)

project(fsh-viewer)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

include_directories(include)
set(SDL_SHARED OFF CACHE INTERNAL "" FORCE)
set(SDL_STATIC ON CACHE INTERNAL "" FORCE)
set(SDL2_DISABLE_INSTALL ON CACHE INTERNAL "" FORCE)
set(SDL_TEST OFF CACHE INTERNAL "" FORCE)
add_subdirectory(vendor/SDL EXCLUDE_FROM_ALL)
add_subdirectory(vendor/LibOpenNFS EXCLUDE_FROM_ALL)

file(GLOB_RECURSE SRC src/*.cpp)

add_executable(${PROJECT_NAME} WIN32 ${SRC})

target_link_libraries(${PROJECT_NAME} PRIVATE SDL3-static LibOpenNFS)

if (NOT WIN32)
  install(TARGETS ${PROJECT_NAME} DESTINATION bin)
endif()