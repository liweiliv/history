project(World)
cmake_minimum_required(VERSION 2.8)

SET(CMAKE_C_FLAGS "$ENV{CXXFLAGS} -g --std=gnu99")
SET(CMAKE_CXX_FLAGS "$ENV{CXXFLAGS} -g --std=c++11")

SET(EXECUTABLE_OUTPUT_PATH bin)
SET(LIBRARY_OUTPUT_PATH lib)

aux_source_directory(. TOP_SRC)
aux_source_directory(./g G_SRC)
aux_source_directory(./util UTIL_SRC)
aux_source_directory(./util/memlib MEMLIB_SRC)

IF (CMAKE_SYSTEM_NAME MATCHES "Linux")
	 ADD_DEFINITIONS(-DUSE_GLFW)
    aux_source_directory(./util/linux_platform UTIL_PLATFORM)
ELSEIF (CMAKE_SYSTEM_NAME MATCHES "Windows")
    ADD_DEFINITIONS(-DUSE_GLFW)
    aux_source_directory(./util/windows_platform UTIL_PLATFORM)
ELSEIF (CMAKE_SYSTEM_NAME MATCHES "FreeBSD") 
    ADD_DEFINITIONS(-DUSE_GLFW)
    aux_source_directory(./util/linux_platform UTIL_PLATFORM)
ELSE ()
    MESSAGE(FATAL_ERROR "unknow platform: ${CMAKE_SYSTEM_NAME}")
ENDIF (CMAKE_SYSTEM_NAME MATCHES "Linux")

add_library(util SHARED ${UTIL_SRC} ${UTIL_PLATFORM})

include_directories(util util/memlib)
add_library(Gengin SHARED ${G_SRC})
target_link_libraries(Gengin util)

add_library(memlib SHARED ${MEMLIB_SRC})

include_directories(g)

ADD_EXECUTABLE(World ${TOP_SRC})
IF (CMAKE_SYSTEM_NAME MATCHES "Linux")
    target_link_libraries(World X11 Xrandr Xxf86vm Xcursor Xi Xinerama)
ENDIF (CMAKE_SYSTEM_NAME MATCHES "Linux")

IF (CMAKE_SYSTEM_NAME MATCHES "Linux")
    target_link_libraries(World pthread dl)
ELSEIF (CMAKE_SYSTEM_NAME MATCHES "Windows") 
    target_link_libraries(World windows)
ELSEIF (CMAKE_SYSTEM_NAME MATCHES "FreeBSD") 
    target_link_libraries(World pthread dl)
ELSE ()
    MESSAGE(FATAL_ERROR "unknow platform: ${CMAKE_SYSTEM_NAME}")
ENDIF (CMAKE_SYSTEM_NAME MATCHES "Linux")

target_link_libraries(World glfw3 GL GLEW)
target_link_libraries(World util Gengin memlib)


