# Fichier CMake pour le répertoire source

# Récupération des fichiers sources
target_sources(OpenYoloLib PRIVATE
    main.cpp
    gui/MainWindow.cpp
    input/InputManager.cpp
    displaymanager/DisplayManager.cpp
    displaymanager/display.cpp
    cursormanager/CursorManager.cpp
    cursormanager/ShaderProgram.cpp
)

# Définir les sources à compiler
set(SOURCES OpenYoloLib)

# Création de la bibliothèque partagée
add_library(OpenYoloLib STATIC)

# Définition des propriétés de la cible
target_include_directories(OpenYoloLib PRIVATE
    ${CMAKE_SOURCE_DIR}/include
    ${CMAKE_BINARY_DIR}/include
)

# Lier les dépendances
target_link_libraries(OpenYoloLib
    ${GTKMM_LIBRARIES}
    ${SDL2_LIBRARIES}
    ${SDL2IMAGE_LIBRARIES}
    ${OPENGL_LIBRARIES}
    ${GLEW_LIBRARIES}
    ${GIF_LIBRARIES}
)

# Ajouter les répertoires d'inclusion
target_include_directories(OpenYoloLib PRIVATE
    ${GTKMM_INCLUDE_DIRS}
    ${SDL2_INCLUDE_DIRS}
    ${SDL2IMAGE_INCLUDE_DIRS}
    ${OPENGL_INCLUDE_DIRS}
    ${GLEW_INCLUDE_DIRS}
    ${GIF_INCLUDE_DIRS}
)

# Création de l'exécutable
add_executable(OpenYolo $<TARGET_OBJECTS:OpenYoloLib>)

# Lier la bibliothèque à l'exécutable
target_link_libraries(OpenYolo
    OpenYoloLib
)

# Installation des cibles
install(TARGETS OpenYolo OpenYoloLib
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
)
