cmake_minimum_required(VERSION 3.15)

# Configuration du projet
project(OpenYolo
    VERSION 1.0.0
    DESCRIPTION "Gestionnaire de curseurs personnalisés pour Linux"
    LANGUAGES CXX
)

# Options de compilation
option(BUILD_TESTS "Construire les tests" ON)
option(ENABLE_LOGGING "Activer les logs de débogage" ON)
option(INSTALL_GTK_SCHEMA "Installer le schéma GSettings" ON)
option(ENABLE_COVERAGE "Activer la couverture de code" OFF)
option(ENABLE_ASAN "Activer l'AddressSanitizer" OFF)
option(ENABLE_UBSAN "Activer l'UndefinedBehaviorSanitizer" OFF)

# Configuration C++
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Configuration des avertissements
set(WARNING_FLAGS "-Wall -Wextra -Wpedantic -Werror -Wno-unused-parameter")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${WARNING_FLAGS}")

# Configuration des symboles de débogage
if(CMAKE_BUILD_TYPE STREQUAL "Debug")
    add_compile_options(-g -O0)
    add_definitions(-DDEBUG)
elseif(CMAKE_BUILD_TYPE STREQUAL "RelWithDebInfo")
    add_compile_options(-g -O2)
    add_definitions(-DNDEBUG)
else()
    add_compile_options(-O3)
    add_definitions(-DNDEBUG)
endif()

# Configuration des sanitizers
if(ENABLE_ASAN)
    add_compile_options(-fsanitize=address -fno-omit-frame-pointer)
    link_libraries(-fsanitize=address)
endif()

if(ENABLE_UBSAN)
    add_compile_options(-fsanitize=undefined -fno-sanitize-recover=all)
    link_libraries(-fsanitize=undefined)
endif()

# Configuration de la couverture de code
if(ENABLE_COVERAGE)
    add_compile_options(--coverage -O0 -g)
    link_libraries(--coverage)
endif()

# Configuration des chemins de sortie
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

# Créer les répertoires de sortie si nécessaire
file(MAKE_DIRECTORY ${CMAKE_ARCHIVE_OUTPUT_DIRECTORY})
file(MAKE_DIRECTORY ${CMAKE_LIBRARY_OUTPUT_DIRECTORY})
file(MAKE_DIRECTORY ${CMAKE_RUNTIME_OUTPUT_DIRECTORY})

# Configuration des chemins d'installation
set(CMAKE_INSTALL_PREFIX "/usr/local" CACHE PATH "Installation prefix")
set(CMAKE_INSTALL_BINDIR "${CMAKE_INSTALL_PREFIX}/bin" CACHE PATH "Binary installation directory")
set(CMAKE_INSTALL_LIBDIR "${CMAKE_INSTALL_PREFIX}/lib" CACHE PATH "Library installation directory")
set(CMAKE_INSTALL_INCLUDEDIR "${CMAKE_INSTALL_PREFIX}/include" CACHE PATH "Header files installation directory")
set(CMAKE_INSTALL_DATADIR "${CMAKE_INSTALL_PREFIX}/share" CACHE PATH "Data files installation directory")
set(CMAKE_INSTALL_DOCDIR "${CMAKE_INSTALL_DATADIR}/doc/open-yolo" CACHE PATH "Documentation installation directory")
set(CMAKE_INSTALL_LOCALEDIR "${CMAKE_INSTALL_DATADIR}/locale" CACHE PATH "Locale files installation directory")
set(CMAKE_INSTALL_SYSCONFDIR "/etc" CACHE PATH "Configuration files installation directory")

# Configuration des chemins d'installation
set(APP_CONFIG_DIR "${CMAKE_INSTALL_PREFIX}/etc/open-yolo" CACHE PATH "Répertoire de configuration")
set(APP_DATA_DIR "${CMAKE_INSTALL_PREFIX}/share/open-yolo" CACHE PATH "Répertoire de données")
set(APP_THEMES_DIR "${APP_DATA_DIR}/themes" CACHE PATH "Répertoire des thèmes")
set(APP_CURSORS_DIR "${APP_DATA_DIR}/cursors" CACHE PATH "Répertoire des curseurs")
set(APP_ICONS_DIR "${APP_DATA_DIR}/icons" CACHE PATH "Répertoire des icônes")
set(APP_LOCALE_DIR "${APP_DATA_DIR}/locale" CACHE PATH "Répertoire des fichiers de traduction")
set(APP_LOGS_DIR "/var/log/open-yolo" CACHE PATH "Répertoire des fichiers de log")

# Création des répertoires de configuration et de données
install(DIRECTORY DESTINATION ${APP_CONFIG_DIR}
        COMPONENT config
        DIRECTORY_PERMISSIONS OWNER_READ OWNER_WRITE OWNER_EXECUTE GROUP_READ GROUP_EXECUTE WORLD_READ WORLD_EXECUTE
        FILE_PERMISSIONS OWNER_READ OWNER_WRITE GROUP_READ WORLD_READ
        )

install(DIRECTORY DESTINATION ${APP_DATA_DIR}
        COMPONENT data
        DIRECTORY_PERMISSIONS OWNER_READ OWNER_WRITE OWNER_EXECUTE GROUP_READ GROUP_EXECUTE WORLD_READ WORLD_EXECUTE
        FILE_PERMISSIONS OWNER_READ OWNER_WRITE GROUP_READ WORLD_READ
        )

# Configuration des fichiers de configuration
configure_file(
    ${CMAKE_SOURCE_DIR}/cmake/config.h.in
    ${CMAKE_BINARY_DIR}/include/config.h
    @ONLY
)

# Configuration du fichier .pc pour pkg-config
set(PKG_CONFIG_LIBDIR "${CMAKE_INSTALL_LIBDIR}/pkgconfig")
set(PKG_CONFIG_INCLUDEDIR "${CMAKE_INSTALL_INCLUDEDIR}")
set(PKG_CONFIG_VERSION "${PROJECT_VERSION}")
set(PKG_CONFIG_DESCRIPTION "${PROJECT_DESCRIPTION}")

configure_file(
    ${CMAKE_SOURCE_DIR}/cmake/open-yolo.pc.in
    ${CMAKE_BINARY_DIR}/open-yolo.pc
    @ONLY
)

# Installation du fichier .pc
install(
    FILES ${CMAKE_BINARY_DIR}/open-yolo.pc
    DESTINATION ${CMAKE_INSTALL_LIBDIR}/pkgconfig
    COMPONENT pkgconfig
)

# Installation des fichiers de configuration
execute_process(
    COMMAND ${CMAKE_COMMAND} -E make_directory ${CMAKE_BINARY_DIR}/config
)

# Créer le répertoire de configuration s'il n'existe pas
install(CODE "
    message(STATUS \"Création du répertoire de configuration...\")
    file(MAKE_DIRECTORY \"${APP_CONFIG_DIR}\")
    # Définir les permissions (rwxr-x---)
    execute_process(COMMAND chmod 750 \"${APP_CONFIG_DIR}\")
")

# Installation du fichier de configuration par défaut
install(CODE "
    message(STATUS \"Installation de la configuration par défaut...\")
    
    # Copier le fichier de configuration si il n'existe pas déjà
    if(NOT EXISTS \"${APP_CONFIG_DIR}/config.ini\")
        file(COPY \"${CMAKE_SOURCE_DIR}/data/default-config.ini\" 
             DESTINATION \"${APP_CONFIG_DIR}\" 
             RENAME \"config.ini\"
        )
        # Définir les permissions (rw-r-----) 
        execute_process(COMMAND chmod 640 \"${APP_CONFIG_DIR}/config.ini\")
    else()
        message(STATUS \"Le fichier de configuration existe déjà, conservation de la version actuelle\")
    endif()
")

# Créer le répertoire de logs
install(CODE "
    message(STATUS \"Création du répertoire de logs...\")
    file(MAKE_DIRECTORY \"${APP_LOGS_DIR}\")
    # Définir les permissions (rwxr-x---)
    execute_process(COMMAND chmod 750 \"${APP_LOGS_DIR}\")
")

# Installation des fichiers de thème par défaut
install(DIRECTORY ${CMAKE_SOURCE_DIR}/data/themes/
        DESTINATION ${APP_THEMES_DIR}
        COMPONENT data
        PATTERN "*" PERMISSIONS OWNER_READ OWNER_WRITE GROUP_READ WORLD_READ
        )

# Installation des curseurs par défaut
install(DIRECTORY ${CMAKE_SOURCE_DIR}/data/cursors/
        DESTINATION ${APP_CURSORS_DIR}
        COMPONENT data
        PATTERN "*" PERMISSIONS OWNER_READ OWNER_WRITE GROUP_READ WORLD_READ
        )

# Installation des fichiers de documentation
install(FILES
    ${CMAKE_SOURCE_DIR}/README.md
    ${CMAKE_SOURCE_DIR}/LICENSE
    DESTINATION ${CMAKE_INSTALL_DOCDIR}
    COMPONENT doc
)

# Configuration des fichiers .desktop pour l'intégration avec le bureau
configure_file(
    ${CMAKE_SOURCE_DIR}/data/open-yolo.desktop.in
    ${CMAKE_BINARY_DIR}/open-yolo.desktop
    @ONLY
)

# Installation du fichier .desktop
install(FILES
    ${CMAKE_BINARY_DIR}/open-yolo.desktop
    DESTINATION ${CMAKE_INSTALL_DATADIR}/applications
    COMPONENT applications
)

# Installation de l'icône de l'application
install(FILES
    ${CMAKE_SOURCE_DIR}/data/icons/hicolor/scalable/apps/open-yolo.svg
    DESTINATION ${CMAKE_INSTALL_DATADIR}/icons/hicolor/scalable/apps/
    COMPONENT icons
)

# Installation des fichiers de schéma GSettings
if(INSTALL_GTK_SCHEMA)
    # Vérifier si glib-compile-schemas est disponible
    find_program(GLIB_COMPILE_SCHEMAS glib-compile-schemas)
    if(GLIB_COMPILE_SCHEMAS)
        install(FILES
            ${CMAKE_SOURCE_DIR}/data/org.openyolo.gschema.xml
            DESTINATION ${CMAKE_INSTALL_DATADIR}/glib-2.0/schemas/
            COMPONENT schemas
        )
        
        # Mise à jour du cache des schémas GSettings après l'installation
        install(CODE "message(STATUS \"Compilation des schémas GSettings...\")"
                CODE "execute_process(COMMAND ${CMAKE_COMMAND} -E env \"PATH=${CMAKE_INSTALL_PREFIX}/bin:$ENV{PATH}\" \
                                     ${GLIB_COMPILE_SCHEMAS} ${CMAKE_INSTALL_PREFIX}/share/glib-2.0/schemas)"
                COMPONENT schemas
        )
    else()
        message(WARNING "glib-compile-schemas introuvable, l'installation des schémas GSettings sera ignorée")
    endif()
endif()

# Installation des fichiers pour le gestionnaire de fenêtres
install(FILES
    ${CMAKE_SOURCE_DIR}/data/applications/open-yolo-wm.desktop
    DESTINATION ${CMAKE_INSTALL_DATADIR}/applications
    PERMISSIONS OWNER_READ OWNER_WRITE GROUP_READ WORLD_READ
    COMPONENT wm
)

# Installation des fichiers pour le gestionnaire d'affichage
install(FILES
    ${CMAKE_SOURCE_DIR}/data/xsessions/open-yolo.desktop
    DESTINATION ${CMAKE_INSTALL_DATADIR}/xsessions
    PERMISSIONS OWNER_READ OWNER_WRITE GROUP_READ WORLD_READ
    COMPONENT xsession
)

# Installation du fichier de configuration du gestionnaire de fenêtres
install(FILES
    ${CMAKE_SOURCE_DIR}/data/config/wm-config.ini
    DESTINATION ${CMAKE_INSTALL_SYSCONFDIR}/open-yolo
    RENAME config.ini
    PERMISSIONS OWNER_READ OWNER_WRITE GROUP_READ WORLD_READ
    COMPONENT config
)

# Installation du script de session
configure_file(
    ${CMAKE_SOURCE_DIR}/cmake/open-yolo-session.in
    ${CMAKE_BINARY_DIR}/open-yolo-session
    @ONLY
)

install(PROGRAMS
    ${CMAKE_BINARY_DIR}/open-yolo-session
    DESTINATION ${CMAKE_INSTALL_BINDIR}
    COMPONENT xsession
)

# Configuration de CPack pour la création de paquets
set(CPACK_PACKAGE_NAME "${PROJECT_NAME}")
set(CPACK_PACKAGE_VERSION "${PROJECT_VERSION}")
set(CPACK_PACKAGE_DESCRIPTION "${PROJECT_DESCRIPTION}")
set(CPACK_PACKAGE_VENDOR "OpenYolo Team")
set(CPACK_PACKAGE_CONTACT "contact@openyolo.org")
set(CPACK_RESOURCE_FILE_LICENSE "${CMAKE_SOURCE_DIR}/LICENSE")
set(CPACK_RESOURCE_FILE_README "${CMAKE_SOURCE_DIR}/README.md")
set(CPACK_PACKAGE_DESCRIPTION_FILE "${CMAKE_SOURCE_DIR}/README.md")
set(CPACK_PACKAGE_ICON "${CMAKE_SOURCE_DIR}/data/icons/hicolor/48x48/apps/open-yolo.png")

# Configuration pour les paquets DEB (Debian/Ubuntu)
set(CPACK_DEBIAN_PACKAGE_MAINTAINER "OpenYolo Team <contact@openyolo.org>")
set(CPACK_DEBIAN_PACKAGE_SECTION "utils")
set(CPACK_DEBIAN_PACKAGE_DEPENDS 
    "libgtkmm-3.0-dev (>= 3.24.0), \
     libsdl2-dev (>= 2.0.10), \
     libsdl2-image-dev (>= 2.0.5), \
     libgl1-mesa-dev, \
     libglew-dev (>= 2.1.0), \
     libgif-dev (>= 5.1.4), \
     xorg-dev, \
     libx11-xcb-dev, \
     libwayland-dev, \
     libxkbcommon-dev"
)
set(CPACK_DEBIAN_PACKAGE_RECOMMENDS "pulseaudio")
set(CPACK_DEBIAN_PACKAGE_SUGGESTS "pulseaudio")
set(CPACK_DEBIAN_PACKAGE_HOMEPAGE "https://github.com/yourusername/Open-Yolo")
set(CPACK_DEBIAN_FILE_NAME "${CPACK_PACKAGE_NAME}_${CPACK_PACKAGE_VERSION}_${CMAKE_SYSTEM_PROCESSOR}.deb")

# Configuration pour les paquets RPM (Fedora/RHEL)
set(CPACK_RPM_PACKAGE_LICENSE "MIT")
set(CPACK_RPM_PACKAGE_GROUP "Applications/System")
set(CPACK_RPM_PACKAGE_URL "https://github.com/yourusername/Open-Yolo")
set(CPACK_RPM_PACKAGE_REQUIRES 
    "gtkmm30-devel >= 3.24.0, \
     SDL2-devel >= 2.0.10, \
     SDL2_image-devel >= 2.0.5, \
     mesa-libGL-devel, \
     glew-devel >= 2.1.0, \
     giflib-devel >= 5.1.4, \
     xorg-x11-server-devel, \
     libxcb-devel, \
     wayland-devel, \
     libxkbcommon-devel"
)
set(CPACK_RPM_FILE_NAME "${CPACK_PACKAGE_NAME}-${CPACK_PACKAGE_VERSION}-${CMAKE_SYSTEM_PROCESSOR}.rpm")

# Inclure CPack
include(CPack)

# Inclure le script de désinstallation
include(cmake/uninstall.cmake)

# Trouver les packages nécessaires
find_package(PkgConfig REQUIRED)
pkg_check_modules(GTKMM3 REQUIRED gtkmm-3.0)
pkg_check_modules(GTK3 REQUIRED gtk+-3.0)
pkg_check_modules(GDK3 REQUIRED gdk-3.0)
pkg_check_modules(CAIROMM REQUIRED cairomm-1.0)  # Version corrigée de cairomm
pkg_check_modules(SDL2 REQUIRED sdl2)
pkg_check_modules(SDL2_IMAGE REQUIRED SDL2_image)

# Trouver OpenGL
find_package(OpenGL REQUIRED)
set(OPENGL_FOUND ${OPENGL_FOUND} CACHE BOOL "" FORCE)
set(OPENGL_INCLUDE_DIR ${OPENGL_INCLUDE_DIR} CACHE PATH "" FORCE)
set(OPENGL_gl_LIBRARY ${OPENGL_gl_LIBRARY} CACHE FILEPATH "" FORCE)
set(OPENGL_glu_LIBRARY ${OPENGL_glu_LIBRARY} CACHE FILEPATH "" FORCE)

# Trouver GLEW
find_package(GLEW REQUIRED)
set(GLEW_FOUND ${GLEW_FOUND} CACHE BOOL "" FORCE)
set(GLEW_INCLUDE_DIRS ${GLEW_INCLUDE_DIRS} CACHE PATH "" FORCE)
set(GLEW_LIBRARIES ${GLEW_LIBRARIES} CACHE FILEPATH "" FORCE)

# Trouver GIFLIB
find_package(GIF REQUIRED)
set(GIF_FOUND ${GIF_FOUND} CACHE BOOL "" FORCE)
set(GIF_INCLUDE_DIRS ${GIF_INCLUDE_DIRS} CACHE PATH "" FORCE)
set(GIF_LIBRARIES ${GIF_LIBRARIES} CACHE FILEPATH "" FORCE)

# Trouver X11
find_package(X11 REQUIRED)
set(X11_FOUND ${X11_FOUND} CACHE BOOL "" FORCE)
set(X11_INCLUDE_DIRS ${X11_INCLUDE_DIR} CACHE PATH "" FORCE)
set(X11_LIBRARIES ${X11_LIBRARIES} CACHE FILEPATH "" FORCE)

# Inclure les répertoires d'en-têtes
include_directories(
    ${CMAKE_SOURCE_DIR}
    ${CMAKE_SOURCE_DIR}/include
    ${CMAKE_SOURCE_DIR}/src
    ${GTKMM3_INCLUDE_DIRS}
    ${GTK3_INCLUDE_DIRS}
    ${GDK3_INCLUDE_DIRS}
    ${CAIROMM_INCLUDE_DIRS}
    ${SDL2_INCLUDE_DIRS}
    ${SDL2_IMAGE_INCLUDE_DIRS}
    ${OPENGL_INCLUDE_DIRS}
    ${GLEW_INCLUDE_DIRS}
    ${GIFLIB_INCLUDE_DIRS}
    ${X11_INCLUDE_DIRS}
)

# Configuration des chemins des modules
list(APPEND CMAKE_MODULE_PATH ${CMAKE_SOURCE_DIR}/cmake)

# Vérification du système d'exploitation
if(NOT CMAKE_SYSTEM_NAME STREQUAL "Linux")
    message(FATAL_ERROR "Ce projet est conçu spécifiquement pour Linux.")
endif()

# Configuration des drapeaux de compilation
if(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
    # Options de compilation communes
    set(COMMON_COMPILE_OPTIONS
        -fPIC
        -fvisibility=hidden
        -fstack-protector-strong
        -fno-common
        -D_FORTIFY_SOURCE=2
        -pthread
    )
    
    # Options de compilation spécifiques au mode Debug
    set(DEBUG_COMPILE_OPTIONS
        -g3
        -ggdb3
        -O0
        -DDEBUG
    )
    
    # Options de compilation spécifiques au mode Release
    set(RELEASE_COMPILE_OPTIONS
        -O3
        -DNDEBUG
        -fomit-frame-pointer
    )
    
    # Options de liaison communes
    set(COMMON_LINK_OPTIONS
        -Wl,-z,now
        -Wl,-z,relro
        -Wl,--as-needed
        -Wl,--no-undefined
        -Wl,--fatal-warnings
    )
    
    # Options spécifiques à GCC
    if(CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
        list(APPEND COMMON_COMPILE_OPTIONS
            -fstack-clash-protection
            -fcf-protection=full
        )
    # Options spécifiques à Clang
    elseif(CMAKE_CXX_COMPILER_ID MATCHES "Clang")
        list(APPEND COMMON_COMPILE_OPTIONS
            -fcolor-diagnostics
            -Wthread-safety
        )
    endif()
    
    # Appliquer les options de compilation
    add_compile_options(${COMMON_COMPILE_OPTIONS})
    
    # Configurer les options de compilation en fonction du type de build
    if(CMAKE_BUILD_TYPE STREQUAL "Debug")
        add_compile_options(${DEBUG_COMPILE_OPTIONS})
        add_definitions(-DDEBUG)
    else()
        add_compile_options(${RELEASE_COMPILE_OPTIONS})
        add_definitions(-DNDEBUG)
    endif()
    
    # Appliquer les options de liaison
    set(CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} ${COMMON_LINK_OPTIONS}")
    set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} ${COMMON_LINK_OPTIONS}")
    
    # Activer les avertissements supplémentaires
    if(ENABLE_STRICT_WARNINGS)
        add_compile_options(
            -Wall
            -Wextra
            -Wpedantic
            -Werror=return-type
            -Werror=non-virtual-dtor
            -Werror=missing-braces
            -Werror=logical-op
            -Werror=old-style-cast
            -Werror=format=2
        )
    endif()
    
    # Activer les sanitizers si demandé
    if(ENABLE_ASAN)
        add_compile_options(-fsanitize=address -fno-omit-frame-pointer)
        link_libraries(-fsanitize=address)
    endif()
    
    if(ENABLE_UBSAN)
        add_compile_options(-fsanitize=undefined -fno-sanitize-recover=all)
        link_libraries(-fsanitize=undefined)
    endif()
    
    # Activer la couverture de code si demandé
    if(ENABLE_COVERAGE)
        add_compile_options(--coverage -O0 -g)
        link_libraries(--coverage)
    endif()
endif()

# Détection des dépendances supplémentaires
pkg_check_modules(GIOMM REQUIRED giomm-2.4)
pkg_check_modules(GLIBMM REQUIRED glibmm-2.4)
pkg_check_modules(CAIRO REQUIRED cairo)
pkg_check_modules(CAIROMM REQUIRED cairomm-1.0)
pkg_check_modules(SIGC++ REQUIRED sigc++-2.0)

# SDL2 (pour la gestion des entrées et du rendu)
pkg_check_modules(SDL2 REQUIRED sdl2>=2.0.10)
pkg_check_modules(SDL2IMAGE REQUIRED SDL2_image>=2.0.5)

# OpenGL & GLEW (pour les effets graphiques)
find_package(OpenGL REQUIRED)
find_package(GLEW REQUIRED)

# giflib (pour le support des GIF animés)
find_package(GIF REQUIRED)

# Ajouter les chemins d'inclusion
include_directories(
    ${GTK3_INCLUDE_DIRS}
    ${GTKMM_INCLUDE_DIRS}
    ${GIOMM_INCLUDE_DIRS}
    ${GLIBMM_INCLUDE_DIRS}
    ${SDL2_INCLUDE_DIRS}
    ${SDL2IMAGE_INCLUDE_DIRS}
    ${GLEW_INCLUDE_DIRS}
    ${GIF_INCLUDE_DIRS}
    ${OPENGL_INCLUDE_DIR}
)

# Ajouter les chemins des bibliothèques
link_directories(
    ${GTK3_LIBRARY_DIRS}
    ${GTKMM_LIBRARY_DIRS}
    ${GIOMM_LIBRARY_DIRS}
    ${GLIBMM_LIBRARY_DIRS}
    ${SDL2_LIBRARY_DIRS}
    ${SDL2IMAGE_LIBRARY_DIRS}
    ${GLEW_LIBRARY_DIRS}
    ${GIF_LIBRARY_DIRS}
    ${OPENGL_LIBRARY_DIRS}
)

# Définir les variables de bibliothèques
set(GTK4_LIBRARIES ${GTK4_LIBRARIES} ${GTK4_LDFLAGS_OTHER})
set(GTKMM_LIBRARIES ${GTKMM_LIBRARIES} ${GTKMM_LDFLAGS_OTHER})
set(GDKMM_LIBRARIES ${GDKMM_LIBRARIES} ${GDKMM_LDFLAGS_OTHER})
set(GIOMM_LIBRARIES ${GIOMM_LIBRARIES} ${GIOMM_LDFLAGS_OTHER})
set(GLIBMM_LIBRARIES ${GLIBMM_LIBRARIES} ${GLIBMM_LDFLAGS_OTHER})
set(SDL2_LIBRARIES ${SDL2_LIBRARIES} ${SDL2_LDFLAGS_OTHER})
set(SDL2IMAGE_LIBRARIES ${SDL2IMAGE_LIBRARIES} ${SDL2IMAGE_LDFLAGS_OTHER})

# Définir les options de compilation
add_compile_options(
    ${GTK4_CFLAGS_OTHER}
    ${GTKMM_CFLAGS_OTHER}
    ${GDKMM_CFLAGS_OTHER}
    ${GIOMM_CFLAGS_OTHER}
    ${GLIBMM_CFLAGS_OTHER}
)

# Définitions de compilation
add_definitions(
    ${GTK4_DEFINITIONS}
    ${GTKMM_DEFINITIONS}
    ${GDKMM_DEFINITIONS}
    ${GIOMM_DEFINITIONS}
    ${GLIBMM_DEFINITIONS}
)

# Configuration des avertissements
if(MSVC)
    add_compile_options(/W4 /WX)
else()
    add_compile_options(-Wall -Wextra -Wpedantic -Werror)
    if(ENABLE_LOGGING)
        add_compile_definitions(ENABLE_LOGGING)
    endif()
endif()

# Configuration de la version
configure_file(
    ${CMAKE_SOURCE_DIR}/include/version.h.in
    ${CMAKE_BINARY_DIR}/include/version.h
)

# Ajout du sous-répertoire des sources
add_subdirectory(src)

# Configuration des tests
if(BUILD_TESTS)
    enable_testing()
    add_subdirectory(tests)
endif()

# Installation
include(GNUInstallDirs)
install(TARGETS OpenYolo
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
)

# Installation des ressources
install(DIRECTORY resources/ DESTINATION ${CMAKE_INSTALL_DATADIR}/open-yolo)

# Installation du schéma GSettings
if(INSTALL_GTK_SCHEMA AND EXISTS "${CMAKE_SOURCE_DIR}/data/org.openyolo.gschema.xml")
    install(FILES ${CMAKE_SOURCE_DIR}/data/org.openyolo.gschema.xml
        DESTINATION ${CMAKE_INSTALL_DATADIR}/glib-2.0/schemas/
    )
    message(STATUS "GSchema will be installed to ${CMAKE_INSTALL_DATADIR}/glib-2.0/schemas/")
endif()

# Génération du fichier .desktop
configure_file(
    ${CMAKE_SOURCE_DIR}/data/open-yolo.desktop.in
    ${CMAKE_BINARY_DIR}/open-yolo.desktop
    @ONLY
)

# Installation du fichier .desktop
install(FILES ${CMAKE_BINARY_DIR}/open-yolo.desktop
    DESTINATION ${CMAKE_INSTALL_DATADIR}/applications/
)

# Génération du fichier .pc pour pkg-config
configure_file(
    ${CMAKE_SOURCE_DIR}/open-yolo.pc.in
    ${CMAKE_BINARY_DIR}/open-yolo.pc
    @ONLY
)

# Installation du fichier .pc
install(FILES ${CMAKE_BINARY_DIR}/open-yolo.pc
    DESTINATION ${CMAKE_INSTALL_LIBDIR}/pkgconfig/
)

# Message de configuration
message(STATUS "\nConfiguration de OpenYolo ${OpenYolo_VERSION}")
message(STATUS "----------------------------------------")
message(STATUS "Build type: ${CMAKE_BUILD_TYPE}")
message(STATUS "Install prefix: ${CMAKE_INSTALL_PREFIX}")
message(STATUS "Compiler: ${CMAKE_CXX_COMPILER_ID} ${CMAKE_CXX_COMPILER_VERSION}")
message(STATUS "Build tests: ${BUILD_TESTS}")
message(STATUS "Install GSettings schema: ${INSTALL_GTK_SCHEMA}")
message(STATUS "\nDépendances trouvées:")
message(STATUS "- GTKMM: ${GTKMM_VERSION}")
message(STATUS "- SDL2: ${SDL2_VERSION}")
message(STATUS "- SDL2_image: ${SDL2IMAGE_VERSION}")
message(STATUS "- OpenGL: ${OPENGL_VERSION}")
message(STATUS "- GLEW: ${GLEW_VERSION}")
message(STATUS "- giflib: ${GIF_VERSION}")
message(STATUS "\nPour construire et installer:")
message(STATUS "  mkdir build && cd build")
message(STATUS "  cmake -DCMAKE_BUILD_TYPE=Release ..")
message(STATUS "  make -j$(nproc)")
message(STATUS "  sudo make install")
message(STATUS "----------------------------------------\n")

