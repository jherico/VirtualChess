# Required for statically linking the MSVC runtime
#set(CMAKE_USER_MAKE_RULES_OVERRIDE
#   ${CMAKE_CURRENT_SOURCE_DIR}/cmake/c_flag_overrides.cmake)
#set(CMAKE_USER_MAKE_RULES_OVERRIDE_CXX
#   ${CMAKE_CURRENT_SOURCE_DIR}/cmake/cxx_flag_overrides.cmake)

project(VirtualChess)
cmake_minimum_required(VERSION 2.8)

set(BUILD_SHARED_LIBS OFF)
set(CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/cmake)
include (cotire)

# First for the generic no-config case (e.g. with mingw)
set( CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/output )
set( CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/output )
set( CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/output )

# Second, for multi-config builds (e.g. msvc)
foreach( OUTPUTCONFIG ${CMAKE_CONFIGURATION_TYPES} )
    string( TOUPPER ${OUTPUTCONFIG} OUTPUTCONFIG )
    set( CMAKE_RUNTIME_OUTPUT_DIRECTORY_${OUTPUTCONFIG} ${CMAKE_BINARY_DIR}/output )
    set( CMAKE_LIBRARY_OUTPUT_DIRECTORY_${OUTPUTCONFIG} ${CMAKE_BINARY_DIR}/output )
    set( CMAKE_ARCHIVE_OUTPUT_DIRECTORY_${OUTPUTCONFIG} ${CMAKE_BINARY_DIR}/output )
endforeach( OUTPUTCONFIG CMAKE_CONFIGURATION_TYPES )

# Comment out to disable the debug build
set(CMAKE_BUILD_TYPE DEBUG)
set(BUILD_SHARED_LIBS OFF)
set(CMAKE_DEBUG_POSTFIX "d")
set_property(GLOBAL PROPERTY USE_FOLDERS ON)
set_property(GLOBAL PROPERTY PREDEFINED_TARGETS_FOLDER "CMakeTargets")


string( TOLOWER "${CMAKE_BUILD_TYPE}" BUILD_TYPE_LOWER )
if(BUILD_TYPE_LOWER STREQUAL "debug")
endif()

# Verbosity is good, especially if a command fails
set(CMAKE_VERBOSE_MAKEFILE ON)

if(CMAKE_COMPILER_IS_GNUCXX)
    # Ensure we use C++ 11 
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++0x")
    # Ensure we generate position independent code 
    if("${CMAKE_SYSTEM_PROCESSOR}" STREQUAL "x86_64")
        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fPIC")
        set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -fPIC")
    endif()
endif()

if(WIN32)
    add_definitions(-DUNICODE -D_UNICODE)
elseif(APPLE)
    # Ensure we use C++ 11 
    set(CMAKE_XCODE_ATTRIBUTE_CLANG_CXX_LANGUAGE_STANDARD "c++0x")
    set(CMAKE_XCODE_ATTRIBUTE_CLANG_CXX_LIBRARY "libc++")
    set(CMAKE_XCODE_ATTRIBUTE_GCC_VERSION "com.apple.compilers.llvm.clang.1_0")
else()
endif()

# Cross platform access to OpenGL
add_definitions(-DGLEW_STATIC)
add_subdirectory(libraries/glew)
set_target_properties(glew PROPERTIES FOLDER "3rdparty")
include_directories(${CMAKE_SOURCE_DIR}/libraries/glew/include)
list(APPEND PROJECT_LIBS glew)

include_directories(${CMAKE_SOURCE_DIR}/libraries/SDL2/include)
link_directories(${CMAKE_SOURCE_DIR}/libraries/SDL2/lib/x86/)
list(APPEND PROJECT_LIBS SDL2)

###############################################################################
#
# Oculus VR SDK dependency 
#

if(${CMAKE_SYSTEM_NAME} MATCHES "Linux") 
  add_subdirectory(libraries/OculusSDK/3rdParty/EDID)
  set_target_properties(edid PROPERTIES FOLDER "3rdparty")
endif()

add_subdirectory(libraries/OculusSDK/LibOVR)
set_target_properties(OculusVR PROPERTIES FOLDER "3rdparty")

add_subdirectory(libraries/OculusSDK/Bindings/C)
set_target_properties(OVR_C PROPERTIES FOLDER "3rdparty")
include_directories(libraries/OculusSDK/Bindings/C/Include)
list(APPEND PROJECT_LIBS OVR_C)

# OpenCTM - a 3D mesh compression
add_subdirectory(libraries/OpenCTM)
set_target_properties(OpenCTM PROPERTIES FOLDER "3rdparty")
include_directories(${CMAKE_SOURCE_DIR}/libraries/OpenCTM)
list(APPEND PROJECT_LIBS OpenCTM)

# Freetype
add_subdirectory(libraries/freetype)
include_directories(libraries/freetype/include)
list(APPEND PROJECT_LIBS freetype)
set_target_properties(freetype PROPERTIES FOLDER "3rdparty")

set(FREETYPE_LIBRARY freetype)
set(FREETYPE_INCLUDE_DIRS ${CMAKE_SOURCE_DIR}/libraries/freetype/include)

add_subdirectory(libraries/rocket)
set_target_properties(Rocket PROPERTIES FOLDER "3rdparty")
include_directories(libraries/rocket/Include)
list(APPEND PROJECT_LIBS Rocket)
#list(APPEND PROJECT_LIBS RocketControls)
#list(APPEND PROJECT_LIBS RocketDebugger)

#add_subdirectory(libraries/sdl2)
#include_directories(libraries/sdl2/include)
#list(APPEND PROJECT_LIBS SDL2)
#list(APPEND PROJECT_LIBS ${SDL2-LIB_DEPENDS})


if (WIN32)
    set(Boost_USE_STATIC_LIBS    ON)
endif()
set(Boost_USE_MULTITHREADED  ON)
set(Boost_USE_STATIC_RUNTIME ON)
find_package(Boost COMPONENTS filesystem system thread)
include_directories(${Boost_INCLUDE_DIR})
link_directories(${Boost_LIBRARY_DIR})
list(APPEND PROJECT_LIBS ${Boost_LIBRARIES})

if((WIN32 OR APPLE))
    # zlib
    add_subdirectory(libraries/zlib)
    set(ZLIB_INCLUDE_DIR ${CMAKE_SOURCE_DIR}/libraries/zlib)
    include_directories(${CMAKE_BINARY_DIR}/libraries/zlib)
    include_directories(${CMAKE_SOURCE_DIR}/libraries/zlib)
    list(APPEND PROJECT_LIBS zlib)

    # png
    set(PNG_STATIC ON CACHE BOOL "Build PNG static library")
    set(PNG_TESTS OFF CACHE BOOL "Build PNG tests")
    set(PNG_SHARED OFF CACHE BOOL "Build PNG shared library")
    add_subdirectory(libraries/libpng)
    include_directories(${CMAKE_SOURCE_DIR}/libraries/libpng)
    include_directories(${CMAKE_BINARY_DIR}/libraries/libpng)
    list(APPEND PROJECT_LIBS png)
    set_target_properties(zlib PROPERTIES FOLDER "3rdparty")
    set_target_properties(png PROPERTIES FOLDER "3rdparty")
else()
    find_package(PkgConfig)
    pkg_check_modules(PNG REQUIRED png)
    include_directories(${PNG_INCLUDE_DIR})
    list(APPEND PROJECT_LIBS png)
    
    pkg_check_modules(SDL2 REQUIRED sdl2)
    include_directories(${SDL2_INCLUDE_DIRS})
    list(APPEND PROJECT_LIBS ${SDL2_LIBRARIES})

    #get_cmake_property(_variableNames VARIABLES)
    #foreach (_variableName ${_variableNames})
    #    message(STATUS "${_variableName}=${${_variableName}}")
    #endforeach()
endif()


find_package(Threads)
list(APPEND PROJECT_LIBS ${CMAKE_THREAD_LIBS_INIT} )

###############################################################################
#
# GLM - Vector / matrix header only math library based on the syntax of GLSL
#
include_directories(${CMAKE_SOURCE_DIR}/libraries/glm)

###############################################################################
#
# OpenGL object wrappers  
#
include_directories(${CMAKE_SOURCE_DIR}/libraries/oglplus/include)
include_directories(${CMAKE_SOURCE_DIR}/libraries/oglplus/implement)


###############################################################################
#
# Non-C++ resources that are shared by the examples
# (shaders, meshes, images, fonts, etc)
#
# On Windows these will be pulled from a common resource DLL.  
#
# On OSX, they will be embedded in each application bundle, owing to CMakes
# lack of support for shared frameworks
# 
# On Linux or if you enable the debug build, they will be pulled from the
# source location at runtime.  
#
add_subdirectory(resources)
include_directories(resources/cpp)
include_directories(${CMAKE_BINARY_DIR}/resources)
list(APPEND PROJECT_LIBS ChessResources)

###############################################################################
#
# The examples themselves
#
add_subdirectory(src)

