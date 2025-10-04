# Fichier CMake pour le module input

# Définir les sources
set(INPUT_MANAGER_SOURCES
    InputManager.cpp
    backends/X11Backend.cpp
    WaylandBackend.cpp
    ShortcutManager.cpp
    ShortcutConfig.cpp
)

# Créer la bibliothèque
add_library(inputmanager ${INPUT_MANAGER_SOURCES})

# Trouver les dépendances requises
find_package(PkgConfig REQUIRED)
find_package(X11 REQUIRED)
find_package(X11 COMPONENTS Xcursor Xfixes)
find_package(nlohmann_json 3.7.3 REQUIRED)
find_package(Threads REQUIRED)

# Vérifier si Xinput2 est disponible
find_package(X11 COMPONENTS Xinput2)
if(X11_Xinput2_FOUND)
    message(STATUS "XInput2 support enabled")
    add_definitions(-DX11_XINPUT2_ENABLED)
else()
    message(STATUS "XInput2 support disabled")
endif()

# Configurer pkg-config pour les dépendances
pkg_check_modules(GTKMM3 REQUIRED gtkmm-3.0)
pkg_check_modules(GDKMM3 REQUIRED gdk-3.0)
pkg_check_modules(GLIBMM2_4 REQUIRED glibmm-2.4)
pkg_check_modules(SIGC2 REQUIRED sigc++-2.0)
pkg_check_modules(X11_XFIXES REQUIRED xfixes)

# Définir les propriétés
target_include_directories(inputmanager PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_SOURCE_DIR}/include
    ${GTKMM3_INCLUDE_DIRS}
    ${GLIBMM2_4_INCLUDE_DIRS}
    ${GDKMM3_INCLUDE_DIRS}
    ${SIGC2_INCLUDE_DIRS}
    ${X11_INCLUDE_DIR}
)

# Définitions de compilation
target_compile_definitions(inputmanager
    PRIVATE
        INPUTMANAGER_EXPORTS
    PUBLIC
        ${GTKMM3_CFLAGS_OTHER}
        ${GLIBMM2_4_CFLAGS_OTHER}
        ${X11_DEFINITIONS}
)

# Activer C++17
target_compile_features(inputmanager PRIVATE cxx_std_17)
set_target_properties(inputmanager PROPERTIES 
    CXX_STANDARD 17 
    CXX_STANDARD_REQUIRED ON 
    CXX_EXTENSIONS OFF
)

# Lier les dépendances
target_link_libraries(inputmanager
    PRIVATE
        ${X11_LIBRARIES}
        ${X11_Xfixes_LIB}
        ${X11_Xcursor_LIB}
        ${X11_Xinput_LIB}
        ${X11_X11_LIB}
        ${GTKMM3_LIBRARIES}
        ${GLIBMM2_4_LIBRARIES}
        ${GDKMM3_LIBRARIES}
        ${SIGC2_LIBRARIES}
        nlohmann_json::nlohmann_json
        Threads::Threads
)

# Installer la bibliothèque
install(TARGETS inputmanager
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)

# Installer les en-têtes
install(DIRECTORY ${CMAKE_SOURCE_DIR}/include/input/
    DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/input
    FILES_MATCHING PATTERN "*.hpp"
)