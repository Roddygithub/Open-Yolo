# Fichier CMake pour le module displaymanager

# Version minimale requise de CMake
cmake_minimum_required(VERSION 3.15)

# Définir les sources du module
set(SOURCES
    DisplayManager.cpp
)

# Créer une bibliothèque statique pour ce module
add_library(displaymanager STATIC ${SOURCES})

# Définir les propriétés de la cible
target_include_directories(displaymanager PRIVATE
    ${CMAKE_SOURCE_DIR}/include
    ${CMAKE_SOURCE_DIR}/include/displaymanager
    ${GTKMM_INCLUDE_DIRS}
    ${X11_INCLUDE_DIRS}
)

# Lier les dépendances
target_link_libraries(displaymanager PRIVATE
    ${GTKMM_LIBRARIES}
    ${X11_LIBRARIES}
)

# Définir les propriétés de compilation
target_compile_features(displaymanager PRIVATE cxx_std_20)
set_target_properties(displaymanager PROPERTIES
    CXX_STANDARD 20
    CXX_STANDARD_REQUIRED ON
    CXX_EXTENSIONS OFF
)

# Installation de la bibliothèque
install(TARGETS displaymanager
    ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)

# Installation des en-têtes
install(DIRECTORY ${CMAKE_SOURCE_DIR}/include/displaymanager/
    DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/displaymanager
    FILES_MATCHING PATTERN "*.hpp"
)
