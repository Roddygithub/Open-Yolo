# Fichier CMake pour le répertoire src

# Ajout des sous-répertoires pour chaque module.
# Chaque sous-répertoire contient son propre CMakeLists.txt,
# ce qui rend le build plus modulaire et plus facile à maintenir.
add_subdirectory(config)
add_subdirectory(log) # Pas de fichier, mais on le garde pour la structure
add_subdirectory(cursormanager)
add_subdirectory(displaymanager)
add_subdirectory(input)
add_subdirectory(gui)

# =============================================================================
# 2. Création de l'exécutable principal
# =============================================================================

# Création de l'exécutable
add_executable(OpenYolo main.cpp)

# Liaison des bibliothèques d'objets à l'exécutable
target_link_libraries(OpenYolo PRIVATE
    gui
    inputmanager
    cursormanager
    displaymanager
    config
)

# Configuration des propriétés de la cible
# Les includes sont maintenant gérés par les bibliothèques d'objets,
# mais on peut le laisser pour la compatibilité avec certains IDE.
target_include_directories(OpenYolo PRIVATE
    ${CMAKE_SOURCE_DIR}/include
    ${CMAKE_SOURCE_DIR}/src
)

# En-têtes précompilés (optionnel, mais bon à avoir)
# set(PCH_HEADER "${CMAKE_SOURCE_DIR}/include/stdafx.h")
# target_precompile_headers(OpenYolo PUBLIC $<BUILD_INTERFACE:${PCH_HEADER}>)


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
