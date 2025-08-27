cmake_minimum_required(VERSION 3.15)

# Configuration du projet
project(OpenYolo
    VERSION 1.0.0
    DESCRIPTION "Gestionnaire de curseurs personnalisés pour Linux"
    LANGUAGES CXX
)

# Options de compilation
option(BUILD_TESTS "Construire les tests" ON)
option(ENABLE_LOGGING "Activer les logs de débogage" ON)
option(INSTALL_GTK_SCHEMA "Installer le schéma GSettings" ON)
option(ENABLE_WARNINGS "Activer les avertissements de compilation" OFF)

# Configuration C++
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Gestion des avertissements
if(ENABLE_WARNINGS)
    if(MSVC)
        add_compile_options(/W4 /WX)
    else()
        add_compile_options(-Wall -Wextra -Wpedantic -Werror)
    endif()
else()
    if(MSVC)
        add_compile_options(/W0)
    else()
        add_compile_options(-w)
    endif()
endif()

# Configuration des chemins
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)

# Inclure les répertoires d'en-têtes
include_directories(
    ${CMAKE_SOURCE_DIR}/include
    ${CMAKE_SOURCE_DIR}/include/cursormanager
    ${CMAKE_SOURCE_DIR}/include/input
    ${CMAKE_SOURCE_DIR}/include/displaymanager
    ${CMAKE_SOURCE_DIR}/include/gui
)

# Détection des dépendances
find_package(PkgConfig REQUIRED)

# Ajouter le répertoire cmake au chemin des modules
list(APPEND CMAKE_MODULE_PATH ${CMAKE_SOURCE_DIR}/cmake)

# Désactiver temporairement les avertissements
if(MSVC)
    add_compile_options(/W0)
else()
    add_compile_options(-w)
endif()

# GTK3 et dépendances
pkg_check_modules(GTK3 REQUIRED gtk+-3.0)
pkg_check_modules(GTKMM REQUIRED gtkmm-3.0)
pkg_check_modules(GIOMM REQUIRED giomm-2.4)
pkg_check_modules(GLIBMM REQUIRED glibmm-2.4)

# SDL2 (pour la gestion des entrées et du rendu)
pkg_check_modules(SDL2 REQUIRED sdl2>=2.0.10)
pkg_check_modules(SDL2IMAGE REQUIRED SDL2_image>=2.0.5)

# OpenGL & GLEW (pour les effets graphiques)
find_package(OpenGL REQUIRED)
find_package(GLEW REQUIRED)

# giflib (pour le support des GIF animés)
find_package(GIF REQUIRED)

# Ajouter les chemins d'inclusion
include_directories(
    ${GTK3_INCLUDE_DIRS}
    ${GTKMM_INCLUDE_DIRS}
    ${GIOMM_INCLUDE_DIRS}
    ${GLIBMM_INCLUDE_DIRS}
    ${SDL2_INCLUDE_DIRS}
    ${SDL2IMAGE_INCLUDE_DIRS}
    ${GLEW_INCLUDE_DIRS}
    ${GIF_INCLUDE_DIRS}
    ${OPENGL_INCLUDE_DIR}
)

# Ajouter les chemins des bibliothèques
link_directories(
    ${GTK3_LIBRARY_DIRS}
    ${GTKMM_LIBRARY_DIRS}
    ${GIOMM_LIBRARY_DIRS}
    ${GLIBMM_LIBRARY_DIRS}
    ${SDL2_LIBRARY_DIRS}
    ${SDL2IMAGE_LIBRARY_DIRS}
    ${GLEW_LIBRARY_DIRS}
    ${GIF_LIBRARY_DIRS}
    ${OPENGL_LIBRARY_DIRS}
)

# Définir les variables de bibliothèques
set(GTK4_LIBRARIES ${GTK4_LIBRARIES} ${GTK4_LDFLAGS_OTHER})
set(GTKMM_LIBRARIES ${GTKMM_LIBRARIES} ${GTKMM_LDFLAGS_OTHER})
set(GDKMM_LIBRARIES ${GDKMM_LIBRARIES} ${GDKMM_LDFLAGS_OTHER})
set(GIOMM_LIBRARIES ${GIOMM_LIBRARIES} ${GIOMM_LDFLAGS_OTHER})
set(GLIBMM_LIBRARIES ${GLIBMM_LIBRARIES} ${GLIBMM_LDFLAGS_OTHER})
set(SDL2_LIBRARIES ${SDL2_LIBRARIES} ${SDL2_LDFLAGS_OTHER})
set(SDL2IMAGE_LIBRARIES ${SDL2IMAGE_LIBRARIES} ${SDL2IMAGE_LDFLAGS_OTHER})

# Définir les options de compilation
add_compile_options(
    ${GTK4_CFLAGS_OTHER}
    ${GTKMM_CFLAGS_OTHER}
    ${GDKMM_CFLAGS_OTHER}
    ${GIOMM_CFLAGS_OTHER}
    ${GLIBMM_CFLAGS_OTHER}
)

# Définitions de compilation
add_definitions(
    ${GTK4_DEFINITIONS}
    ${GTKMM_DEFINITIONS}
    ${GDKMM_DEFINITIONS}
    ${GIOMM_DEFINITIONS}
    ${GLIBMM_DEFINITIONS}
)

# Configuration des avertissements
if(MSVC)
    add_compile_options(/W4 /WX)
else()
    add_compile_options(-Wall -Wextra -Wpedantic -Werror)
    if(ENABLE_LOGGING)
        add_compile_definitions(ENABLE_LOGGING)
    endif()
endif()

# Configuration de la version
configure_file(
    ${CMAKE_SOURCE_DIR}/include/version.h.in
    ${CMAKE_BINARY_DIR}/include/version.h
)

# Ajout du sous-répertoire des sources
add_subdirectory(src)

# Configuration des tests
if(BUILD_TESTS)
    enable_testing()
    add_subdirectory(tests)
endif()

# Installation
include(GNUInstallDirs)
install(TARGETS OpenYolo
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
)

# Installation des ressources
install(DIRECTORY resources/ DESTINATION ${CMAKE_INSTALL_DATADIR}/open-yolo)

# Installation du schéma GSettings
if(INSTALL_GTK_SCHEMA AND EXISTS "${CMAKE_SOURCE_DIR}/data/org.openyolo.gschema.xml")
    install(FILES ${CMAKE_SOURCE_DIR}/data/org.openyolo.gschema.xml
        DESTINATION ${CMAKE_INSTALL_DATADIR}/glib-2.0/schemas/
    )
    message(STATUS "GSchema will be installed to ${CMAKE_INSTALL_DATADIR}/glib-2.0/schemas/")
endif()

# Génération du fichier .desktop
configure_file(
    ${CMAKE_SOURCE_DIR}/data/open-yolo.desktop.in
    ${CMAKE_BINARY_DIR}/open-yolo.desktop
    @ONLY
)

# Installation du fichier .desktop
install(FILES ${CMAKE_BINARY_DIR}/open-yolo.desktop
    DESTINATION ${CMAKE_INSTALL_DATADIR}/applications/
)

# Génération du fichier .pc pour pkg-config
configure_file(
    ${CMAKE_SOURCE_DIR}/open-yolo.pc.in
    ${CMAKE_BINARY_DIR}/open-yolo.pc
    @ONLY
)

# Installation du fichier .pc
install(FILES ${CMAKE_BINARY_DIR}/open-yolo.pc
    DESTINATION ${CMAKE_INSTALL_LIBDIR}/pkgconfig/
)

# Message de configuration
message(STATUS "\nConfiguration de OpenYolo ${OpenYolo_VERSION}")
message(STATUS "----------------------------------------")
message(STATUS "Build type: ${CMAKE_BUILD_TYPE}")
message(STATUS "Install prefix: ${CMAKE_INSTALL_PREFIX}")
message(STATUS "Compiler: ${CMAKE_CXX_COMPILER_ID} ${CMAKE_CXX_COMPILER_VERSION}")
message(STATUS "Build tests: ${BUILD_TESTS}")
message(STATUS "Install GSettings schema: ${INSTALL_GTK_SCHEMA}")
message(STATUS "\nDépendances trouvées:")
message(STATUS "- GTKMM: ${GTKMM_VERSION}")
message(STATUS "- SDL2: ${SDL2_VERSION}")
message(STATUS "- SDL2_image: ${SDL2IMAGE_VERSION}")
message(STATUS "- OpenGL: ${OPENGL_VERSION}")
message(STATUS "- GLEW: ${GLEW_VERSION}")
message(STATUS "- giflib: ${GIF_VERSION}")
message(STATUS "\nPour construire et installer:")
message(STATUS "  mkdir build && cd build")
message(STATUS "  cmake -DCMAKE_BUILD_TYPE=Release ..")
message(STATUS "  make -j$(nproc)")
message(STATUS "  sudo make install")
message(STATUS "----------------------------------------\n")

