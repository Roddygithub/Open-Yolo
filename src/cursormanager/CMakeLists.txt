# Fichier CMake pour le module cursormanager

# Définir les sources
set(CURSOR_MANAGER_SOURCES
    CursorManager.cpp
    ShaderProgram.cpp
)

# Créer la bibliothèque
add_library(cursormanager ${CURSOR_MANAGER_SOURCES})

# Définir les propriétés
target_include_directories(cursormanager PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/..
    ${SDL2_INCLUDE_DIRS}
    ${OPENGL_INCLUDE_DIRS}
    ${GLEW_INCLUDE_DIRS}
    ${GLEW_INCLUDE_DIRS}/..  # Pour inclure le répertoire parent de GLEW
    ${X11_INCLUDE_DIRS}
    ${X11_INCLUDE_DIR}  # Ajout du répertoire d'inclusion X11
    ${CAIROMM_INCLUDE_DIRS}
    ${GTKMM3_INCLUDE_DIRS}
    ${GDKMM3_INCLUDE_DIRS}
    ${GLIBMM2_4_INCLUDE_DIRS}
)

# Lier les dépendances
# Ajouter les répertoires de bibliothèques
target_link_directories(cursormanager PRIVATE
    ${GTKMM3_LIBRARY_DIRS}
    ${CAIROMM_LIBRARY_DIRS}
    ${GDKMM3_LIBRARY_DIRS}
    ${GLIBMM2_4_LIBRARY_DIRS}
    ${SIGC2_LIBRARY_DIRS}
)

# Lier les bibliothèques
target_link_libraries(cursormanager PRIVATE
    ${SDL2_LIBRARIES}
    ${OPENGL_LIBRARIES}
    ${GLEW_LIBRARIES}
    ${GLEW_LIBRARY}  # Lien direct vers la bibliothèque GLEW
    ${GIF_LIBRARIES}
    ${X11_LIBRARIES}
    ${X11_X11_LIB}  # Lien direct vers X11
    ${X11_Xfixes_LIB}  # Pour XFixes
    ${X11_Xcursor_LIB}  # Pour Xcursor
    ${GTKMM3_LIBRARIES}
    ${CAIROMM_LIBRARIES}
    ${GDKMM3_LIBRARIES}
    ${GLIBMM2_4_LIBRARIES}
    ${SIGC2_LIBRARIES}
)

# Ajouter SDL2_image si activé
if(ENABLE_SDL2_IMAGE)
    target_link_libraries(cursormanager PRIVATE ${SDL2_IMAGE_LIBRARIES})
    target_compile_definitions(cursormanager PRIVATE HAS_SDL2_IMAGE)
endif()

# Ajouter les bibliothèques système
target_link_libraries(cursormanager PRIVATE
    dl      # Pour dlopen, dlsym, etc.
    pthread # Pour le support des threads POSIX
)

# Installation de la bibliothèque
install(TARGETS cursormanager
    ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)
