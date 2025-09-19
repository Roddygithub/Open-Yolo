# Fichier CMake pour le module GUI

# Version minimale requise de CMake
cmake_minimum_required(VERSION 3.15)

# Définir les sources du module
set(SOURCES
    MainWindow.cpp
)

# Créer une bibliothèque statique pour ce module
add_library(gui STATIC ${SOURCES})

# Définir les propriétés de la cible
target_include_directories(gui PRIVATE
    ${CMAKE_SOURCE_DIR}/include
    ${CMAKE_SOURCE_DIR}/include/gui
    ${GTKMM_INCLUDE_DIRS}
    ${CAIROMM_INCLUDE_DIRS}
)

# Lier les dépendances
target_link_libraries(gui PRIVATE
    ${GTKMM_LIBRARIES}
    ${CAIROMM_LIBRARIES}
)

# Définir les propriétés de compilation
target_compile_features(gui PRIVATE cxx_std_20)
set_target_properties(gui PROPERTIES
    CXX_STANDARD 20
    CXX_STANDARD_REQUIRED ON
    CXX_EXTENSIONS OFF
)

# Installation de la bibliothèque
install(TARGETS gui
    ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)

# Installation des en-têtes
install(DIRECTORY ${CMAKE_SOURCE_DIR}/include/gui/
    DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/gui
    FILES_MATCHING PATTERN "*.hpp"
)
