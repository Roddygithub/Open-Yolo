# Fichier CMake pour le module UI

# Définir les sources
set(UI_SOURCES
    CursorWindow.cpp
)

# Créer la bibliothèque
add_library(ui ${UI_SOURCES})

# Définir les propriétés
target_include_directories(ui PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_SOURCE_DIR}/include
    ${GTKMM_INCLUDE_DIRS}
    ${GLIBMM_INCLUDE_DIRS}
    ${GDKMM_INCLUDE_DIRS}
    ${SIGC++_INCLUDE_DIRS}
)

# Lier les dépendances
target_link_libraries(ui PRIVATE
    inputmanager
    ${GTKMM_LIBRARIES}
    ${GLIBMM_LIBRARIES}
    ${GDKMM_LIBRARIES}
    ${SIGC++_LIBRARIES}
    ${X11_LIBRARIES}
)

# Activer C++17
target_compile_features(ui PRIVATE cxx_std_17)
set_target_properties(ui PROPERTIES 
    CXX_STANDARD 17 
    CXX_STANDARD_REQUIRED ON 
    CXX_EXTENSIONS OFF
)

# Installation
install(TARGETS ui
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)
