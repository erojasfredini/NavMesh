cmake_minimum_required(VERSION 3.15.0)

# Librerias a linkear e includes
set(INCLUDES)
set(LIBRARIES)
set(LIBRARY_DIRS)

set(INCLUDES ${INCLUDES} .)
set(INCLUDES ${INCLUDES} ../NavMeshGenerator/src/)

set(LIBRARIES ${LIBRARIES} sfml-window sfml-system sfml-main sfml-graphics)
set(LIBRARIES ${LIBRARIES} glu32)
set(LIBRARIES ${LIBRARIES} NavMeshGenerator)

include_directories(${INCLUDES})

# Proyecto de ejecutable a generar con codigos
file(GLOB_RECURSE NAVMESH_EDITOR_SOURCES RELATIVE ${PROJECT_SOURCE_DIR}/NavMeshEditor/ *.cpp *.h *.hpp)
add_executable(NavMeshEditor ${NAVMESH_EDITOR_SOURCES})
target_link_libraries(NavMeshEditor ${LIBRARIES})

# Folder filters
create_filters("${NAVMESH_EDITOR_SOURCES}" ".")

# Capetas de salida
set_target_properties(NavMeshEditor PROPERTIES PDB_OUTPUT_DIRECTORY "${PROJECT_SOURCE_DIR}/bin/NavMeshEditor/$<CONFIG>")
set_target_properties(NavMeshEditor PROPERTIES RUNTIME_OUTPUT_DIRECTORY "${PROJECT_SOURCE_DIR}/bin/NavMeshEditor/$<CONFIG>")