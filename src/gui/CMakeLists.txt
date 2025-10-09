# Fichier CMake pour le module GUI

# Définir les sources
set(GUI_SOURCES
    MainWindow.cpp
)

# Créer la bibliothèque
add_library(gui ${GUI_SOURCES})

# Définir les propriétés
target_include_directories(gui PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/..
    ${GTKMM3_INCLUDE_DIRS}
    ${GDKMM3_INCLUDE_DIRS}
    ${CAIROMM_INCLUDE_DIRS}
    ${GLIBMM2_4_INCLUDE_DIRS}
    ${SIGC2_INCLUDE_DIRS}
)

# Lier les dépendances
target_link_libraries(gui PRIVATE
    ${GTKMM3_LIBRARIES}
    ${GDKMM3_LIBRARIES}
    ${CAIROMM_LIBRARIES}
    ${GLIBMM2_4_LIBRARIES}
    ${SIGC2_LIBRARIES}
)

# Installation de la bibliothèque
install(TARGETS gui
    ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)
