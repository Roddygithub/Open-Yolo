# Fichier CMake pour le module de journalisation

# Définir les sources
set(LOG_SOURCES
    Logger.cpp
)

# Créer la bibliothèque
add_library(log ${LOG_SOURCES})

# Définir les propriétés
target_include_directories(log PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/..
)

# Installation de la bibliothèque
install(TARGETS log
    ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)
