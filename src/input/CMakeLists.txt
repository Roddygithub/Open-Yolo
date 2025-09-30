# Fichier CMake pour le module d'entrée

# Définir les sources
set(INPUT_SOURCES
    InputManager.cpp
)

# Créer la bibliothèque
add_library(input ${INPUT_SOURCES})

# Définir les propriétés
target_include_directories(input PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/..
    ${SDL2_INCLUDE_DIRS}
    ${X11_INCLUDE_DIRS}
)

# Lier les dépendances
target_link_libraries(input PRIVATE
    ${SDL2_LIBRARIES}
    ${X11_LIBRARIES}
    ${CMAKE_THREAD_LIBS_INIT}
)

# Installation de la bibliothèque
install(TARGETS input
    ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)
