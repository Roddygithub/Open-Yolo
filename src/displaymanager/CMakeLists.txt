# Fichier CMake pour le module displaymanager

# Définir les sources
set(DISPLAY_MANAGER_SOURCES
    DisplayManager.cpp
)

# Créer la bibliothèque
add_library(displaymanager ${DISPLAY_MANAGER_SOURCES})

# Définir les propriétés
target_include_directories(displaymanager PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_SOURCE_DIR}/include
    ${X11_INCLUDE_DIR}
    ${GTKMM_INCLUDE_DIRS}
    ${GLIBMM_INCLUDE_DIRS}
    ${SIGC++_INCLUDE_DIRS}
)

# Définitions de compilation
target_compile_definitions(displaymanager
    PRIVATE
        DISPLAYMANAGER_EXPORTS
    PUBLIC
        ${GTKMM_DEFINITIONS}
        ${GLIBMM_DEFINITIONS}
)

# Activer C++17
target_compile_features(displaymanager PRIVATE cxx_std_17)
set_target_properties(displaymanager PROPERTIES
    CXX_STANDARD 17
    CXX_STANDARD_REQUIRED ON
    CXX_EXTENSIONS OFF
    POSITION_INDEPENDENT_CODE ON
)

# Lier les dépendances
target_link_libraries(displaymanager PRIVATE
    ${X11_LIBRARIES}
    ${X11_Xrandr_LIB}
    ${X11_X11_LIB}
    ${GTKMM_LIBRARIES}
    ${GLIBMM_LIBRARIES}
    ${SIGC++_LIBRARIES}
    ${CMAKE_THREAD_LIBS_INIT}
    Xrandr
    gio-2.0
    gobject-2.0
    glib-2.0
)

# Installation de la bibliothèque
install(TARGETS displaymanager
    ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)
