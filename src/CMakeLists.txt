file(GLOB SOURCE_FILES *.cpp)
list(REMOVE_ITEM SOURCE_FILES Common.cpp)
file(GLOB HEADER_FILES *.h )
list(APPEND SOURCE_FILES ${HEADER_FILES})

configure_file(Config.h.in Config.h)
list(APPEND HEADER_FILES ${CMAKE_CURRENT_BINARY_DIR}/Config.h )
include_directories(${CMAKE_CURRENT_BINARY_DIR})
include_directories(${CMAKE_CURRENT_SOURCE_DIR})

set(EXECUTABLE VirtualChess)

MACRO(ADD_MSVC_PRECOMPILED_HEADER PrecompiledHeader PrecompiledSource SourcesVar)
  IF(MSVC)
    GET_FILENAME_COMPONENT(PrecompiledBasename ${PrecompiledHeader} NAME_WE)
    SET(PrecompiledBinary "${CMAKE_CURRENT_BINARY_DIR}/${PrecompiledBasename}.pch")
    #message(FATAL_ERROR ${${SourcesVar}})
    SET(Sources ${${SourcesVar}})

    SET_SOURCE_FILES_PROPERTIES(${PrecompiledSource}
                                PROPERTIES COMPILE_FLAGS "/Yc\"${PrecompiledHeader}\" /Fp\"${PrecompiledBinary}\""
                                           OBJECT_OUTPUTS "${PrecompiledBinary}")
    SET_SOURCE_FILES_PROPERTIES(${Sources}
                                PROPERTIES COMPILE_FLAGS "/Yu\"${PrecompiledHeader}\" /FI\"${PrecompiledHeader}\" /Fp\"${PrecompiledBinary}\""
                                           OBJECT_DEPENDS "${PrecompiledBinary}")  
    # Add precompiled header to SourcesVar
    LIST(APPEND ${SourcesVar} ${PrecompiledSource})
  ENDIF(MSVC)
ENDMACRO(ADD_MSVC_PRECOMPILED_HEADER)

if(WIN32)
    list(REMOVE_ITEM SOURCE_FILES ${CMAKE_CURRENT_SOURCE_DIR}/Common.cpp)
    ADD_MSVC_PRECOMPILED_HEADER("Common.h" "Common.cpp" SOURCE_FILES)
    SET( CMAKE_CXX_FLAGS  "${CMAKE_CXX_FLAGS} /Zm200" )
    add_executable(${EXECUTABLE} WIN32 ${SOURCE_FILES})
elseif(APPLE)
    list(APPEND SOURCE_FILES ${ALL_RESOURCES})
    foreach(resource_file ${ALL_RESOURCES}) 
        file(RELATIVE_PATH relative_path ${RESOURCE_ROOT} ${resource_file})
        get_filename_component(relative_path ${relative_path} PATH)
        set_source_files_properties(${resource_file} 
            PROPERTIES MACOSX_PACKAGE_LOCATION Resources/${relative_path})
        source_group("Resources\\\\${relative_path}" FILES ${resource_file})
    endforeach()
    add_executable(${EXECUTABLE} MACOSX_BUNDLE ${SOURCE_FILES} )
else()
    add_executable(${EXECUTABLE} ${SOURCE_FILES} )
endif()

target_link_libraries(${EXECUTABLE} ${PROJECT_LIBS})
