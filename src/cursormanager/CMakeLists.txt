# Fichier CMake pour le module cursormanager

# Version minimale requise de CMake
cmake_minimum_required(VERSION 3.15)

# Définir les sources du module
set(SOURCES
    CursorManager.cpp
    ShaderProgram.cpp
)

# Créer une bibliothèque statique pour ce module
add_library(cursormanager STATIC ${SOURCES})

# Définir les propriétés de la cible
target_include_directories(cursormanager PRIVATE
    ${CMAKE_SOURCE_DIR}/include
    ${CMAKE_SOURCE_DIR}/include/cursormanager
    ${GTKMM_INCLUDE_DIRS}
    ${SDL2_INCLUDE_DIRS}
    ${SDL2_IMAGE_INCLUDE_DIRS}
    ${OPENGL_INCLUDE_DIRS}
    ${GLEW_INCLUDE_DIRS}
    ${GIF_INCLUDE_DIRS}
    ${X11_INCLUDE_DIRS}
)

# Lier les dépendances
target_link_libraries(cursormanager PRIVATE
    ${GTKMM_LIBRARIES}
    ${SDL2_LIBRARIES}
    ${SDL2_IMAGE_LIBRARIES}
    ${OPENGL_LIBRARIES}
    ${GLEW_LIBRARIES}
    ${GIF_LIBRARIES}
    ${X11_LIBRARIES}
)

# Définir les propriétés de compilation
target_compile_features(cursormanager PRIVATE cxx_std_20)
set_target_properties(cursormanager PROPERTIES
    CXX_STANDARD 20
    CXX_STANDARD_REQUIRED ON
    CXX_EXTENSIONS OFF
)

# Installation de la bibliothèque
install(TARGETS cursormanager
    ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)

# Installation des en-têtes
install(DIRECTORY ${CMAKE_SOURCE_DIR}/include/cursormanager/
    DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/cursormanager
    FILES_MATCHING PATTERN "*.hpp"
)
