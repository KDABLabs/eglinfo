cmake_minimum_required(VERSION 3.1)
project(eglinfo VERSION 0.1)

set(CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/cmake")
include(GNUInstallDirs)

option(USE_INTERNAL_HEADERS "Using internal Khronos EGL headers." OFF)

set(EGLINFO_SOURCES main.cpp)
if (USE_INTERNAL_HEADERS)
    message(STATUS "Using internal Khronos EGL headers.")
    set(EGLINFO_INCLUDE_DIRECTORIES 3rdparty/khronos)
    set(EGLINFO_LIBRARIES -lEGL)
else ()
    find_package(EGL REQUIRED)
    message(STATUS "Using system EGL headers.")
    set(EGLINFO_INCLUDE_DIRECTORIES ${EGL_INCLUDE_DIRS})
    set(EGLINFO_LIBRARIES ${EGL_LIBRARIES})
    add_definitions(${EGL_DEFINITIONS})
endif ()

add_executable(eglinfo ${EGLINFO_SOURCES})
set_property(TARGET eglinfo PROPERTY CXX_STANDARD 11)
target_include_directories(eglinfo PUBLIC ${EGLINFO_INCLUDE_DIRECTORIES})
target_link_libraries (eglinfo ${EGLINFO_LIBRARIES})

install(TARGETS eglinfo DESTINATION ${CMAKE_INSTALL_BINDIR} COMPONENT "runtime")
