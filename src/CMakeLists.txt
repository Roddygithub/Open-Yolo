# Fichier CMake pour le répertoire src

# =============================================================================
# 0. En-têtes précompilés (PCH)
# =============================================================================
# Crée un en-tête précompilé pour accélérer la compilation.
set(PCH_HEADER "${CMAKE_SOURCE_DIR}/include/stdafx.h")

# Applique le PCH à toutes les cibles de ce répertoire et des sous-répertoires.
# Note: Chaque cible doit inclure "stdafx.h" en premier.
target_precompile_headers(OpenYolo PUBLIC $<BUILD_INTERFACE:${PCH_HEADER}>)

# =============================================================================
# 1. Définition des modules en tant que bibliothèques d'objets
# =============================================================================
# Cette approche améliore la modularité et les temps de compilation.

# Module de configuration
add_library(config OBJECT config/ConfigManager.cpp)
target_include_directories(config PUBLIC ${CMAKE_SOURCE_DIR}/include)

# Module de gestion du curseur
add_library(cursormanager OBJECT cursormanager/CursorManager.cpp cursormanager/ShaderProgram.cpp)
target_include_directories(cursormanager PUBLIC ${CMAKE_SOURCE_DIR}/include)
target_link_libraries(cursormanager PUBLIC config OpenGL::GL GLEW::GLEW)

# Module de gestion de l'affichage
add_library(displaymanager OBJECT displaymanager/DisplayManager.cpp)
target_include_directories(displaymanager PUBLIC ${CMAKE_SOURCE_DIR}/include)
target_link_libraries(displaymanager PUBLIC
    ${X11_LIBRARIES}
    ${X11_X11_LIB}
    ${X11_Xrandr_LIB}
)

# Module de l'interface graphique
add_library(gui OBJECT gui/MainWindow.cpp)
target_include_directories(gui PUBLIC ${CMAKE_SOURCE_DIR}/include)
target_include_directories(gui PUBLIC
    ${GTKMM3_INCLUDE_DIRS}
)
target_link_libraries(gui PUBLIC
    config cursormanager displaymanager input log
    ${GTKMM3_LIBRARIES}
)

# Module de gestion des entrées
add_library(input OBJECT input/InputManager.cpp)
target_include_directories(input PUBLIC ${CMAKE_SOURCE_DIR}/include)

# Module de journalisation
add_library(log OBJECT log/Logger.cpp)
target_include_directories(log PUBLIC ${CMAKE_SOURCE_DIR}/include)

# =============================================================================
# 2. Création de l'exécutable principal
# =============================================================================

# Création de l'exécutable
add_executable(OpenYolo main.cpp)

# Liaison des bibliothèques d'objets à l'exécutable
target_link_libraries(OpenYolo PRIVATE
    gui # La liaison de 'gui' inclut transitivement les autres modules
)

# Configuration des propriétés de la cible
# Les includes sont maintenant gérés par les bibliothèques d'objets,
# mais on peut le laisser pour la compatibilité avec certains IDE.
target_include_directories(OpenYolo PRIVATE
    ${CMAKE_SOURCE_DIR}/include
    ${CMAKE_SOURCE_DIR}/src
)

# Configuration des propriétés de compilation
target_compile_features(OpenYolo PRIVATE cxx_std_17)
set_target_properties(OpenYolo PROPERTIES CXX_STANDARD 17)

target_compile_options(OpenYolo PRIVATE
    $<$<CONFIG:Debug>:-g -O0>
    $<$<CONFIG:Release>:-O3>
    $<$<CONFIG:RelWithDebInfo>:-O2 -g>
    $<$<CONFIG:MinSizeRel>:-Os>
    -Wall 
    -Wextra 
    -Wno-pedantic 
    -Wno-conversion 
    -Wno-sign-conversion
    -Wno-deprecated-declarations
)

# Liaison des bibliothèques externes restantes
# Les dépendances principales (GTK, OpenGL, X11) sont maintenant liées
# aux bibliothèques d'objets qui les utilisent.
target_link_libraries(OpenYolo PRIVATE
    ${SDL2_LIBRARIES}
    ${SDL2_IMAGE_LIBRARIES}
    dl
    pthread
)

# =============================================================================
# 3. Configuration de l'installation
# =============================================================================
target_include_directories(OpenYolo PUBLIC
    $<BUILD_INTERFACE:${CMAKE_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:include>
)

install(TARGETS OpenYolo
    RUNTIME DESTINATION bin
    LIBRARY DESTINATION lib
    ARCHIVE DESTINATION lib
)

install(DIRECTORY ${CMAKE_SOURCE_DIR}/include/ DESTINATION include
    FILES_MATCHING PATTERN "*.hpp" PATTERN "*.h"
)
