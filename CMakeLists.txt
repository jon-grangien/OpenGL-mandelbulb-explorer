cmake_minimum_required(VERSION 3.6)
set(APP_NAME "mandelbulb")
project(${APP_NAME})
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/cmake/Modules")

find_package(OpenGL REQUIRED)
if (${OPENGL_FOUND})
    message(STATUS "OpenGL found")
else()
    message(FATAL "OpenGL not found")
endif ()

find_package(GLEW REQUIRED)
if (${GLEW_FOUND})
    message(STATUS "GLEW found")
else()
    message(FATAL "GLEW not found")
endif ()

find_package(GLM REQUIRED)
if (${GLM_FOUND})
    message(STATUS "GLM found")
else()
    message(FATAL "GLM not found")
endif ()

set(CMAKE_CXX_STANDARD 14)

file(GLOB SOURCE_FILES src/*.cc)

set(GLFW_SOURCE_DIR "ext/glfw")

# Building only the GLFW lib
set(BUILD_SHARED_LIBS OFF CACHE BOOL "")
set(GLFW_BUILD_EXAMPLES OFF CACHE BOOL "")
set(GLFW_BUILD_TESTS OFF CACHE BOOL "")
set(GLFW_BUILD_DOCS OFF CACHE BOOL "")
set(GLFW_INSTALL OFF CACHE BOOL "")
add_subdirectory(${GLFW_SOURCE_DIR})

set(INCLUDE_DIRS ${CMAKE_SOURCE_DIR}/include
        ${GLFW_SOURCE_DIR}/include
        ${GLFW_SOURCE_DIR}/deps
        ${OPENGL_INCLUDE_DIR}
        ${GLEW_INCLUDE_DIRS}
        ${GLM_INCLUDE_DIR} )

if (APPLE)
    find_package(X11)
    if (NOT ${X11_FOUND})
        message(FATAL "X11 not found")
    endif ()
    set(INCLUDE_DIRS ${INCLUDE_DIRS} ${X11_INCLUDE_DIR})
    set(EXTRA_LIBRARIES ${OPENGL_LIBRARIES} ${X11_LIBRARIES})
    set(EXTRA_LIBRARIES "-framework CoreFoundation -framework CoreGraphics -framework Cocoa")
else ()
    set(EXTRA_LIBRARIES ${OPENGL_gl_LIBRARY} ${OPENGL_glu_LIBRARY} ${GLEW_LIBRARIES} X11 m)
endif ()

include_directories(${INCLUDE_DIRS})

#message(WARNING "All include dirs: ${INCLUDE_DIRS}")
#message(WARNING "All source files: ${SOURCE_FILES}")
#message(WARNING "extra: ${EXTRA_LIBRARIES}")

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra -Wpedantic -DGLEW_STATIC")
add_executable(${APP_NAME} ${SOURCE_FILES})
target_link_libraries(${APP_NAME} glfw ${GLFW_LIBRARIES} ${EXTRA_LIBRARIES})
