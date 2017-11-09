cmake_minimum_required(VERSION 3.6)
set(APP_NAME "mandelbulb")
project(${APP_NAME})

find_package(OpenGL)

set(CMAKE_CXX_STANDARD 11)

file(GLOB SOURCE_FILES src/main.cpp)

set(GLFW_SOURCE_DIR "ext/glfw")

# Building only the GLFW lib
set(BUILD_SHARED_LIBS OFF CACHE BOOL "")
set(GLFW_BUILD_EXAMPLES OFF CACHE BOOL "")
set(GLFW_BUILD_TESTS OFF CACHE BOOL "")
set(GLFW_BUILD_DOCS OFF CACHE BOOL "")
set(GLFW_INSTALL OFF CACHE BOOL "")

add_subdirectory(${GLFW_SOURCE_DIR})

set(INCLUDE_DIRS ${GLFW_SOURCE_DIR}/include
                 ${GLFW_SOURCE_DIR}/deps)

if (APPLE)
    find_package(X11)
    if (NOT ${X11_FOUND})
        message(FATAL "X11 not found")
    endif ()
    set(INCLUDE_DIRS ${INCLUDE_DIRS} ${X11_INCLUDE_DIR} ${OPENGL_INCLUDE_DIR})
    set(EXTRA_LIBRARIES ${OPENGL_LIBRARIES} ${X11_LIBRARIES})
    set(EXTRA_LIBRARIES "-framework CoreFoundation -framework CoreGraphics -framework Cocoa")
else ()
    set(EXTRA_LIBRARIES ${OPENGL_gl_LIBRARY} X11 m)
endif ()

include_directories(${INCLUDE_DIRS})

#message(WARNING "All include dirs: ${INCLUDE_DIRS}")
#message(WARNING "All source files: ${SOURCE_FILES}")

add_executable(${APP_NAME} ${SOURCE_FILES})
target_link_libraries(${APP_NAME} glfw ${GLFW_LIBRARIES} ${EXTRA_LIBRARIES})
