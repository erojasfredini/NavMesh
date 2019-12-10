cmake_minimum_required(VERSION 3.15.0)

##################################################################
#############               FUNCTIONS                #############
##################################################################

# Create filters for a project
function(create_filters sources rootPath)
    foreach(_source IN ITEMS ${sources})
        get_filename_component(_source_path "${_source}" DIRECTORY)
        string(REPLACE "${CMAKE_SOURCE_DIR}" "" _group_path "${_source_path}")

        string(REPLACE "/${rootPath}" "" _group_path "${_group_path}")
        string(REPLACE "${rootPath}" "" _group_path "${_group_path}")

        string(REPLACE "/" "\\" _group_path "${_group_path}")
        source_group("${_group_path}" FILES "${_source}")
    endforeach()
endfunction(create_filters)

##################################################################
#############               SOLUTION                 #############
##################################################################

project(NavMesh LANGUAGES CXX)
# Version
set (NavMesh_VERSION_MAJOR 1)
set (NavMesh_VERSION_MINOR 0)

##################################################################
#############                CONFIG                  #############
##################################################################

option(SFML_STATIC_LIBRARIES "Usar biblioteca estática para SFML" ON)

find_package(SFML COMPONENTS system window graphics main REQUIRED)

##################################################################
#############        NavMeshGenerator PROJECT        #############
##################################################################

add_subdirectory(NavMeshGenerator/)

##################################################################
#############         NavMeshEditor PROJECT          #############
##################################################################

add_subdirectory(NavMeshEditor/)

##################################################################
#############            DEPENDENCIES                #############
##################################################################

add_dependencies(NavMeshEditor NavMeshGenerator)