# Fichier CMake pour le module de configuration

# Définir les sources
set(CONFIG_SOURCES
    ConfigManager.cpp
)

# Créer la bibliothèque
add_library(config ${CONFIG_SOURCES})

# Définir les propriétés
target_include_directories(config PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/..
)

# Lier les dépendances
target_link_libraries(config PRIVATE
    ${GLIBMM_LIBRARIES}
    ${GIOMM_LIBRARIES}
)

# Installation de la bibliothèque
install(TARGETS config
    ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)
