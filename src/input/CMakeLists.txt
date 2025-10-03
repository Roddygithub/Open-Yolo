# Fichier CMake pour le module input

# Définir les sources
# Les fichiers de protocole Wayland sont générés et doivent être inclus, mais pas listés comme sources directes.
set(INPUT_MANAGER_SOURCES
    InputManager.cpp
    backends/X11Backend.cpp
    backends/WaylandBackend.cpp
)

# Créer la bibliothèque
add_library(inputmanager ${INPUT_MANAGER_SOURCES})

# Définir les propriétés
target_include_directories(inputmanager PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_SOURCE_DIR}/include
    ${GTKMM_INCLUDE_DIRS}
    ${GLIBMM_INCLUDE_DIRS}
    ${GDKMM_INCLUDE_DIRS}
    ${SIGC++_INCLUDE_DIRS}
)

# Ajouter les chemins d'inclusion pour Wayland et XKBcommon
target_include_directories(inputmanager PRIVATE
    ${WAYLAND_CLIENT_INCLUDE_DIRS}
    ${XKBCOMMON_INCLUDE_DIRS}
)

# Définitions de compilation
target_compile_definitions(inputmanager
    PRIVATE
        INPUTMANAGER_EXPORTS
    PUBLIC
        ${GTKMM_DEFINITIONS}
        ${GLIBMM_DEFINITIONS}
)

# Activer C++17
target_compile_features(inputmanager PRIVATE cxx_std_17)
set_target_properties(inputmanager PROPERTIES CXX_STANDARD 17 CXX_STANDARD_REQUIRED ON CXX_EXTENSIONS OFF)

# Trouver les paquets nécessaires via pkg-config
find_package(PkgConfig REQUIRED)
pkg_check_modules(WAYLAND_CLIENT wayland-client)
pkg_check_modules(XKBCOMMON xkbcommon)
pkg_check_modules(GIO_UNIX gio-unix-2.0)

# Lier les dépendances
target_link_libraries(inputmanager PRIVATE
    # Utiliser les cibles importées est plus moderne et robuste
    PkgConfig::GTKMM3
    PkgConfig::GDKMM3
    PkgConfig::SIGC2
    PkgConfig::GLIBMM2_4
    PkgConfig::WAYLAND_CLIENT
    PkgConfig::XKBCOMMON
    PkgConfig::GIO_UNIX
    X11
)