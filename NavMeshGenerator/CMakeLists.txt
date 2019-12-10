cmake_minimum_required(VERSION 3.15.0)

# Librerias a linkear e includes
set(INCLUDES)
#set(LIBRARIES)

set(INCLUDES ${INCLUDES} .)
set(INCLUDES ${INCLUDES} dependencies/polypartition/)
set(INCLUDES ${INCLUDES} dependencies/clipper_ver5.1.6/)

set(LIBRARIES ${LIBRARIES} sfml-graphics)

get_target_property(SFML_INCLUDE sfml-graphics INTERFACE_INCLUDE_DIRECTORIES)
set(INCLUDES ${INCLUDES} "${SFML_INCLUDE}")

include_directories(${INCLUDES})

# Proyecto de ejecutable a generar con codigos
file(GLOB_RECURSE NAVMESH_GENERATOR_SOURCES RELATIVE ${PROJECT_SOURCE_DIR}/NavMeshGenerator/ *.cpp *.h *.hpp)
add_library(NavMeshGenerator STATIC ${NAVMESH_GENERATOR_SOURCES})

# Folder filters
create_filters("${NAVMESH_GENERATOR_SOURCES}" ".")

# Capetas de salida
set_target_properties(NavMeshGenerator PROPERTIES PDB_OUTPUT_DIRECTORY "${PROJECT_SOURCE_DIR}/bin/NavMeshGenerator/$<CONFIG>")
set_target_properties(NavMeshGenerator PROPERTIES ARCHIVE_OUTPUT_DIRECTORY "${PROJECT_SOURCE_DIR}/bin/NavMeshGenerator/$<CONFIG>")# ARCHIVE_OUTPUT_DIRECTORY for static libs
set_target_properties(NavMeshGenerator PROPERTIES LIBRARY_OUTPUT_DIRECTORY "${PROJECT_SOURCE_DIR}/bin/NavMeshGenerator/$<CONFIG>")# LIBRARY_OUTPUT_DIRECTORY for dynamic libs
set_target_properties(NavMeshGenerator PROPERTIES RUNTIME_OUTPUT_DIRECTORY "${PROJECT_SOURCE_DIR}/bin/NavMeshGenerator/$<CONFIG>")