# Fichier CMake pour le module cursormanager

# Définir les sources
set(CURSOR_MANAGER_SOURCES
    CursorManager.cpp
    ShaderProgram.cpp
)

# Créer la bibliothèque
add_library(cursormanager ${CURSOR_MANAGER_SOURCES})

# Définir les propriétés
target_include_directories(cursormanager PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/..
    ${SDL2_INCLUDE_DIRS}
    ${OPENGL_INCLUDE_DIRS}
    ${GLEW_INCLUDE_DIRS}
    ${X11_INCLUDE_DIRS}
)

# Lier les dépendances
target_link_libraries(cursormanager PRIVATE
    ${SDL2_LIBRARIES}
    ${SDL2_IMAGE_LIBRARIES}
    ${OPENGL_LIBRARIES}
    ${GLEW_LIBRARIES}
    ${GIF_LIBRARIES}
    ${X11_LIBRARIES}
    ${GTKMM_LIBRARIES}
)

# Installation de la bibliothèque
install(TARGETS cursormanager
    ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)
