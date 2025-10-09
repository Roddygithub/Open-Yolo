# Configuration pour les exemples
add_executable(cursor_example
    cursor_example.cpp
)

target_link_libraries(cursor_example
    PRIVATE
        inputmanager
        ${X11_LIBRARIES}
        Xcursor
)

target_include_directories(cursor_example
    PRIVATE
        ${CMAKE_SOURCE_DIR}/include
)

# Créer un répertoire pour les ressources
execute_process(
    COMMAND ${CMAKE_COMMAND} -E make_directory
    ${CMAKE_CURRENT_BINARY_DIR}/resources/cursors
)

# Copier les exemples de curseurs
file(GLOB CURSOR_FILES "${CMAKE_SOURCE_DIR}/resources/cursors/*")
foreach(CURSOR_FILE ${CURSOR_FILES})
    file(COPY ${CURSOR_FILE} DESTINATION ${CMAKE_CURRENT_BINARY_DIR}/resources/cursors)
endforeach()
