# Fichier CMake pour le module input

# Détection des dépendances requises
find_package(PkgConfig REQUIRED)
find_package(X11 REQUIRED)
find_package(X11 COMPONENTS Xcursor Xfixes)
find_package(nlohmann_json 3.7.3 REQUIRED)
find_package(Threads REQUIRED)

# Détection des bibliothèques GTK/GLib
pkg_check_modules(GTKMM3 REQUIRED gtkmm-3.0)
pkg_check_modules(GDKMM3 REQUIRED gdkmm-3.0)
pkg_check_modules(GLIBMM2_4 REQUIRED glibmm-2.4)
pkg_check_modules(GIOMM2_4 REQUIRED giomm-2.4)
pkg_check_modules(CAIROMM REQUIRED cairomm-1.0)
pkg_check_modules(SIGC2 REQUIRED sigc++-2.0)

# Détection de Wayland
pkg_check_modules(WAYLAND_CLIENT wayland-client)
pkg_check_modules(WAYLAND_EGL wayland-egl)
pkg_check_modules(WAYLAND_CURSOR wayland-cursor)
pkg_check_modules(XKBCOMMON xkbcommon)

# Définir les sources communes
set(INPUT_MANAGER_SOURCES
    InputManager.cpp
    backends/X11Backend.cpp
    backends/WaylandBackend.cpp
    ShortcutManager.cpp
    ShortcutConfig.cpp
)

# Activer Wayland si toutes les dépendances sont trouvées
if(WAYLAND_CLIENT_FOUND AND WAYLAND_EGL_FOUND AND 
   WAYLAND_CURSOR_FOUND AND XKBCOMMON_FOUND AND
   GTKMM3_FOUND AND GDKMM3_FOUND AND GLIBMM2_4_FOUND AND GIOMM2_4_FOUND)
    message(STATUS "Wayland support enabled")
    add_definitions(-DHAVE_WAYLAND=1)
    
    # Ajouter les chemins d'inclusion pour Wayland
    include_directories(
        ${WAYLAND_CLIENT_INCLUDE_DIRS}
        ${WAYLAND_EGL_INCLUDE_DIRS}
        ${WAYLAND_CURSOR_INCLUDE_DIRS}
        ${XKBCOMMON_INCLUDE_DIRS}
    )
else()
    message(STATUS "Wayland support disabled - required libraries not found")
    message(STATUS "  WAYLAND_CLIENT: ${WAYLAND_CLIENT_FOUND}")
    message(STATUS "  WAYLAND_EGL: ${WAYLAND_EGL_FOUND}")
    message(STATUS "  WAYLAND_CURSOR: ${WAYLAND_CURSOR_FOUND}")
    message(STATUS "  XKBCOMMON: ${XKBCOMMON_FOUND}")
    message(STATUS "  GTKMM3: ${GTKMM3_FOUND}")
    message(STATUS "  GDKMM3: ${GDKMM3_FOUND}")
    message(STATUS "  GLIBMM2_4: ${GLIBMM2_4_FOUND}")
    message(STATUS "  GIOMM2_4: ${GIOMM2_4_FOUND}")
    add_definitions(-DHAVE_WAYLAND=0)
endif()

# Créer la bibliothèque
add_library(inputmanager ${INPUT_MANAGER_SOURCES})

# Configuration de la cible
set_target_properties(inputmanager PROPERTIES
    CXX_STANDARD 17
    CXX_STANDARD_REQUIRED ON
    CXX_EXTENSIONS OFF
)

# Options de compilation
target_compile_options(inputmanager PRIVATE
    $<$<CONFIG:Debug>:-g -O0>
    $<$<CONFIG:Release>:-O3>
    -Wall -Wextra -Wpedantic -Wno-pedantic
)

# Définir les répertoires d'inclusion
target_include_directories(inputmanager PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/..
    ${X11_INCLUDE_DIR}
    ${X11_X11_INCLUDE_PATH}
    ${X11_Xinput2_INCLUDE_PATH}
    ${GTKMM3_INCLUDE_DIRS}
    ${GDKMM3_INCLUDE_DIRS}
    ${GLIBMM2_4_INCLUDE_DIRS}
    ${GIOMM2_4_INCLUDE_DIRS}
    ${CAIROMM_INCLUDE_DIRS}
    ${SIGC2_INCLUDE_DIRS}
    ${WAYLAND_CLIENT_INCLUDE_DIRS}
    ${WAYLAND_EGL_INCLUDE_DIRS}
    ${WAYLAND_CURSOR_INCLUDE_DIRS}
    ${XKBCOMMON_INCLUDE_DIRS}
)

# Lier les bibliothèques
target_link_libraries(inputmanager
    PRIVATE
        ${X11_LIBRARIES}
        ${X11_X11_LIB}
        ${X11_Xinput2_LIB}
        ${X11_Xfixes_LIBRARY}
        ${X11_Xcursor_LIBRARY}
        ${GTKMM3_LIBRARIES}
        ${GDKMM3_LIBRARIES}
        ${GLIBMM2_4_LIBRARIES}
        ${GIOMM2_4_LIBRARIES}
        ${CAIROMM_LIBRARIES}
        ${SIGC2_LIBRARIES}
        ${WAYLAND_CLIENT_LIBRARIES}
        ${WAYLAND_EGL_LIBRARIES}
        ${WAYLAND_CURSOR_LIBRARIES}
        ${XKBCOMMON_LIBRARIES}
        wayland-client
        wayland-egl
        wayland-cursor
        xkbcommon
        ${CMAKE_THREAD_LIBS_INIT}
        nlohmann_json::nlohmann_json
        -ldl
        -lrt
        -lm
        -lstdc++fs
)

# Installer la bibliothèque
install(TARGETS inputmanager
    ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
    PUBLIC_HEADER DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/input
)

# Installer les en-têtes
install(DIRECTORY ${CMAKE_SOURCE_DIR}/include/input/
    DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/input
    FILES_MATCHING PATTERN "*.hpp"
)