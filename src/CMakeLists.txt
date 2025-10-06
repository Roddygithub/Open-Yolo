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

# Création de l'exécutable avec la version simplifiée
add_executable(OpenYolo 
    main_simple.cpp
    openyolo/AppCore.cpp
)

# Trouver les dépendances nécessaires
find_package(PkgConfig REQUIRED)
pkg_check_modules(GTKMM3 REQUIRED gtkmm-3.0)
pkg_check_modules(GDKMM3 REQUIRED gdkmm-3.0)
pkg_check_modules(GLIBMM2_4 REQUIRED glibmm-2.4)
pkg_check_modules(GIOMM2_4 REQUIRED giomm-2.4)
pkg_check_modules(CAIROMM REQUIRED cairomm-1.0)
pkg_check_modules(PANGOMM REQUIRED pangomm-1.4)
pkg_check_modules(ATKMM REQUIRED atkmm-1.6)

# Liaison des bibliothèques d'objets à l'exécutable
target_link_libraries(OpenYolo PRIVATE
    gui
    inputmanager
    cursormanager
    displaymanager
    config
    ${GTKMM3_LIBRARIES}
    ${GDKMM3_LIBRARIES}
    ${GLIBMM2_4_LIBRARIES}
    ${GIOMM2_4_LIBRARIES}
    ${CAIROMM_LIBRARIES}
    ${PANGOMM_LIBRARIES}
    ${ATKMM_LIBRARIES}
    ${X11_LIBRARIES}
    ${X11_X11_LIB}
    ${X11_Xfixes_LIB}
    ${X11_Xcursor_LIB}
    dl
    pthread
)

# Configuration des propriétés de la cible
target_include_directories(OpenYolo PRIVATE
    ${CMAKE_SOURCE_DIR}/include
    ${CMAKE_SOURCE_DIR}/src
    ${GTKMM3_INCLUDE_DIRS}
    ${GDKMM3_INCLUDE_DIRS}
    ${GLIBMM2_4_INCLUDE_DIRS}
    ${GIOMM2_4_INCLUDE_DIRS}
    ${CAIROMM_INCLUDE_DIRS}
    ${PANGOMM_INCLUDE_DIRS}
    ${ATKMM_INCLUDE_DIRS}
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
