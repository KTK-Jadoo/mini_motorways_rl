cmake_minimum_required(VERSION 3.10)
project(MiniMotorwaysRL)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Apple Silicon Mac specific paths
set(HOMEBREW_PREFIX "/opt/homebrew")

# Add Homebrew paths
include_directories(${HOMEBREW_PREFIX}/include)
link_directories(${HOMEBREW_PREFIX}/lib)

# Silence OpenGL deprecation warnings on Mac
add_definitions(-DGL_SILENCE_DEPRECATION)

# Find OpenGL (built into macOS)
find_library(OPENGL_LIBRARY OpenGL REQUIRED)

# Find GLFW
find_library(GLFW_LIBRARY 
    NAMES glfw glfw3
    PATHS ${HOMEBREW_PREFIX}/lib
    REQUIRED
)

# Find GLEW
find_library(GLEW_LIBRARY
    NAMES GLEW glew
    PATHS ${HOMEBREW_PREFIX}/lib
    REQUIRED
)

# Find GLM (header-only)
find_path(GLM_INCLUDE_DIR glm/glm.hpp
    PATHS ${HOMEBREW_PREFIX}/include
    REQUIRED
)

# Source files
set(SOURCES
    main.cpp
    renderer.cpp
    mini_motorways_env.cpp
)

# Create executable
add_executable(mini_motorways_rl ${SOURCES})

# Include directories
target_include_directories(mini_motorways_rl PRIVATE
    ${HOMEBREW_PREFIX}/include
    ${GLM_INCLUDE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}
)

# Link libraries
target_link_libraries(mini_motorways_rl
    ${OPENGL_LIBRARY}
    ${GLFW_LIBRARY}
    ${GLEW_LIBRARY}
)

# Print configuration
message(STATUS "Building Mini Motorways RL for Apple Silicon Mac")
message(STATUS "OpenGL: ${OPENGL_LIBRARY}")
message(STATUS "GLFW: ${GLFW_LIBRARY}")
message(STATUS "GLEW: ${GLEW_LIBRARY}")
message(STATUS "GLM: ${GLM_INCLUDE_DIR}")

# Set build type to Release for better performance
if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release)
endif()

message(STATUS "Build type: ${CMAKE_BUILD_TYPE}")