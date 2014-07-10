# Required for statically linking the MSVC runtime
set(CMAKE_USER_MAKE_RULES_OVERRIDE
   ${CMAKE_CURRENT_SOURCE_DIR}/cmake/c_flag_overrides.cmake)
set(CMAKE_USER_MAKE_RULES_OVERRIDE_CXX
   ${CMAKE_CURRENT_SOURCE_DIR}/cmake/cxx_flag_overrides.cmake)

project(VirtualChess)
cmake_minimum_required(VERSION 2.8)

set(CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/cmake)

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
list(APPEND EXAMPLE_LIBS glew)

# GLFW - Cross platform OpenGL window creation and input handling
set(GLFW_BUILD_EXAMPLES OFF CACHE BOOL "Build the GLFW example programs")
set(GLFW_BUILD_TESTS OFF CACHE BOOL "Build the GLFW test programs")
set(GLFW_BUILD_DOCS OFF CACHE BOOL "Build the GLFW documentation")
set(GLFW_INSTALL OFF CACHE BOOL "Generate installation target")
add_subdirectory(libraries/glfw)
set_target_properties(glfw PROPERTIES FOLDER "3rdparty")
include_directories(${CMAKE_SOURCE_DIR}/libraries/glfw/include)
list(APPEND EXAMPLE_LIBS glfw ${GLFW_LIBRARIES})


###############################################################################
#
# Oculus VR SDK dependency 
#

add_subdirectory(libraries/OculusSDK/3rdParty/EDID)
set_target_properties(edid PROPERTIES FOLDER "3rdparty")

add_subdirectory(libraries/OculusSDK/LibOVR)
set_target_properties(OculusVR PROPERTIES FOLDER "3rdparty")

add_subdirectory(libraries/OculusSDK/Bindings/C)
set_target_properties(OVR_C PROPERTIES FOLDER "3rdparty")
include_directories(libraries/OculusSDK/Bindings/C/Include)
list(APPEND EXAMPLE_LIBS OVR_C)

# OpenCTM - a 3D mesh compression
#add_subdirectory(libraries/OpenCTM)
#set_target_properties(OpenCTM PROPERTIES FOLDER "3rdparty")
#include_directories(${CMAKE_SOURCE_DIR}/libraries/OpenCTM)
#list(APPEND EXAMPLE_LIBS OpenCTM)

# Computer vision library with advanced image loading and manipulation 
# functionality.  
# OpenCV is OPTIONAL.  
find_package( OpenCV QUIET REQUIRED)
include_directories(${OpenCV_INCLUDE_DIRS})
list(APPEND EXAMPLE_LIBS ${OpenCV_LIBS})

find_package(Threads)
list(APPEND EXAMPLE_LIBS ${CMAKE_THREAD_LIBS_INIT} )

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
set_target_properties(ExampleResources PROPERTIES FOLDER "Examples/Shared")
include_directories(resources/cpp)
include_directories(${CMAKE_BINARY_DIR}/resources)
list(APPEND EXAMPLE_LIBS ExampleResources)

###############################################################################
#
# The examples themselves
#
add_subdirectory(src)

