file(GLOB SOURCE_FILES *.cpp)
list(REMOVE_ITEM SOURCE_FILES Common.cpp)
file(GLOB HEADER_FILES *.h)
list(APPEND SOURCE_FILES ${HEADER_FILES})

configure_file(Config.h.in Config.h)
list(APPEND HEADER_FILES ${CMAKE_CURRENT_BINARY_DIR}/Config.h )
include_directories(${CMAKE_CURRENT_BINARY_DIR})
include_directories(${CMAKE_CURRENT_SOURCE_DIR})

set(EXECUTABLE VirtualChess)

if(WIN32)
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

target_link_libraries(${EXECUTABLE} ${EXAMPLE_LIBS})
