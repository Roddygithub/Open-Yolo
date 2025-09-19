# Fichier CMake pour le répertoire source

# Version minimale requise de CMake
cmake_minimum_required(VERSION 3.15)

# Définir les sources
set(SOURCES
    main.cpp
)

# Désactiver les avertissements spécifiques pour les dépendances
add_compile_options(
    $<$<CONFIG:DEBUG>:-g -O0>
    $<$<CONFIG:RELEASE>:-O3>
    $<$<CONFIG:RELWITHDEBINFO>:-O2 -g>
    $<$<CONFIG:MINSIZEREL>:-Os>
)

# Options de compilation spécifiques au projet
add_compile_definitions(
    $<$<CONFIG:DEBUG>:DEBUG=1>
    $<$<CONFIG:RELEASE>:NDEBUG=1>
    $<$<CONFIG:RELWITHDEBINFO>:NDEBUG=1>
    $<$<CONFIG:MINSIZEREL>:NDEBUG=1>
)

# Configuration des avertissements
if(MSVC)
    add_compile_options(/W4 /WX)
else()
    add_compile_options(-Wall -Wextra -Wpedantic -Werror)
endif()

# Créer la bibliothèque de configuration
add_library(config STATIC
    config/ConfigManager.cpp
    log/Logger.cpp
)

# Définir les propriétés de la cible
set_target_properties(config PROPERTIES
    CXX_STANDARD 20
    CXX_STANDARD_REQUIRED ON
    CXX_EXTENSIONS OFF
    POSITION_INDEPENDENT_CODE ON
    VERSION ${PROJECT_VERSION}
    SOVERSION ${PROJECT_VERSION_MAJOR}
)

# Définir les répertoires d'inclusion
target_include_directories(config
    PUBLIC 
        $<BUILD_INTERFACE:${CMAKE_SOURCE_DIR}/include>
        $<INSTALL_INTERFACE:include>
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}
)

# Lier les dépendances de la bibliothèque de configuration
target_link_libraries(config PRIVATE
    ${CMAKE_THREAD_LIBS_INIT}
    ${GTKMM_LIBRARIES}
    ${GLIBMM_LIBRARIES}
    ${GIOMM_LIBRARIES}
    ${SIGC++_LIBRARIES}
)

# Ajouter les sous-répertoires
add_subdirectory(gui)
add_subdirectory(input)
add_subdirectory(displaymanager)
add_subdirectory(cursormanager)

# Création de l'exécutable
add_executable(OpenYolo ${SOURCES})

# Définition des propriétés de la cible
set_target_properties(OpenYolo PROPERTIES
    CXX_STANDARD 20
    CXX_STANDARD_REQUIRED ON
    CXX_EXTENSIONS OFF
    VERSION ${PROJECT_VERSION}
    RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin
    RUNTIME_OUTPUT_DIRECTORY_DEBUG ${CMAKE_BINARY_DIR}/bin
    RUNTIME_OUTPUT_DIRECTORY_RELEASE ${CMAKE_BINARY_DIR}/bin
)

# Définir les répertoires d'inclusion pour l'exécutable
target_include_directories(OpenYolo PRIVATE
    ${CMAKE_SOURCE_DIR}/include
    ${CMAKE_BINARY_DIR}/include
    ${GTK3_INCLUDE_DIRS}
    ${GTKMM3_INCLUDE_DIRS}
    ${GLIBMM2_4_INCLUDE_DIRS}
    ${GIOMM2_0_INCLUDE_DIRS}
    ${SIGC++_INCLUDE_DIRS}
    ${SDL2_INCLUDE_DIRS}
    ${SDL2_IMAGE_INCLUDE_DIRS}
    ${OPENGL_INCLUDE_DIR}
    ${GLEW_INCLUDE_DIRS}
    ${X11_INCLUDE_DIR}
)

# Lier les bibliothèques internes
target_link_libraries(OpenYolo PRIVATE
    gui
    input
    displaymanager
    cursormanager
    config
    ${CMAKE_THREAD_LIBS_INIT}
)

# Lier les dépendances externes
target_link_libraries(OpenYolo PRIVATE
    ${GTKMM3_LIBRARIES}
    ${GLIBMM2_4_LIBRARIES}
    ${GIOMM2_0_LIBRARIES}
    ${SIGC++_LIBRARIES}
    ${SDL2_LIBRARIES}
    ${SDL2_IMAGE_LIBRARIES}
    ${OPENGL_LIBRARIES}
    ${GLEW_LIBRARIES}
    ${GIF_LIBRARIES}
    ${X11_LIBRARIES}
    ${X11_Xi_LIB}
    ${X11_Xcursor_LIB}
    ${X11_Xrandr_LIB}
    ${X11_Xfixes_LIB}
    ${X11_Xinerama_LIB}
    ${X11_Xxf86vm_LIB}
    ${X11_Xext_LIB}
    ${X11_Xrender_LIB}
    ${X11_X11_LIB}
    ${X11_xf86misc_LIB}
    ${X11_Xtst_LIB}
    dl
    pthread
    m
)

# Définir les propriétés de la cible
set_target_properties(OpenYolo PROPERTIES
    CXX_STANDARD 20
    CXX_STANDARD_REQUIRED ON
    CXX_EXTENSIONS OFF
)

# Définition des chemins d'installation
include(GNUInstallDirs)
set(CMAKE_INSTALL_BINDIR "${CMAKE_INSTALL_PREFIX}/bin" CACHE PATH "Installation directory for binaries")
set(CMAKE_INSTALL_DATADIR "${CMAKE_INSTALL_PREFIX}/share" CACHE PATH "Installation directory for data")
set(CMAKE_INSTALL_SYSCONFDIR "/etc" CACHE PATH "Installation directory for configuration files")
set(CMAKE_INSTALL_SYSTEMD_SYSTEM_UNITDIR "/lib/systemd/system" CACHE PATH "Installation directory for systemd service files")

# Installation de l'exécutable
install(TARGETS OpenYolo
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
    BUNDLE DESTINATION ${CMAKE_INSTALL_BINDIR}
    COMPONENT applications
)

# Installation de la bibliothèque de configuration
install(TARGETS config
    ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    COMPONENT libraries
)

# Installation des en-têtes
install(DIRECTORY ${CMAKE_SOURCE_DIR}/include/
    DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
    COMPONENT development
    FILES_MATCHING PATTERN "*.hpp" PATTERN "*.h"
)

# Installation des ressources
if(EXISTS "${CMAKE_SOURCE_DIR}/data" AND IS_DIRECTORY "${CMAKE_SOURCE_DIR}/data")
    install(DIRECTORY ${CMAKE_SOURCE_DIR}/data/
        DESTINATION ${CMAKE_INSTALL_DATADIR}/open-yolo
        COMPONENT data
        USE_SOURCE_PERMISSIONS
        PATTERN ".git*" EXCLUDE
        PATTERN "CMake*" EXCLUDE
    )
    
    # Installation des schémas GSettings
    if(INSTALL_GTK_SCHEMA)
        install(FILES
            ${CMAKE_SOURCE_DIR}/data/org.openyolo.gschema.xml
            DESTINATION ${CMAKE_INSTALL_DATADIR}/glib-2.0/schemas/
            COMPONENT schemas
        )
        
        # Mise à jour du cache des schémas GSettings après l'installation
        install(CODE "message(STATUS \"Compiling GSettings schemas...\")"
                CODE "execute_process(COMMAND ${CMAKE_COMMAND} -E env \"PATH=${CMAKE_INSTALL_PREFIX}/bin:$ENV{PATH}\" \
                                     ${GLIB_COMPILE_SCHEMAS} ${CMAKE_INSTALL_DATADIR}/glib-2.0/schemas)"
                COMPONENT schemas
        )
    endif()
endif()

# Installation du fichier de configuration (uniquement s'il n'existe pas déjà)
if(EXISTS "${CMAKE_SOURCE_DIR}/open-yolo.conf")
    install(CODE "
        if(NOT EXISTS \"${CMAKE_INSTALL_PREFIX}/etc/open-yolo/config.ini\")
            message(STATUS \"Installing default configuration...\")
            file(INSTALL
                DESTINATION \"${CMAKE_INSTALL_PREFIX}/etc/open-yolo\"
                TYPE FILE
                RENAME \"config.ini\"
                FILES \"${CMAKE_SOURCE_DIR}/data/default-config.ini\"
            )
            file(CHMOD \"${CMAKE_INSTALL_PREFIX}/etc/open-yolo/config.ini\"
                PERMISSIONS OWNER_READ OWNER_WRITE GROUP_READ WORLD_READ
            )
        else()
            message(STATUS \"Configuration file already exists, preserving existing configuration\")
        endif()
    " COMPONENT config)
    
    # Installation d'un exemple de configuration
    install(FILES ${CMAKE_SOURCE_DIR}/open-yolo.conf
        DESTINATION ${CMAKE_INSTALL_DOCDIR}/examples
        RENAME open-yolo.conf.example
        COMPONENT doc
    )
endif()

# Installation du fichier de service systemd (Linux uniquement)
if(UNIX AND NOT APPLE AND EXISTS "${CMAKE_SOURCE_DIR}/open-yolo.service.in")
    # Créer le répertoire de sortie si nécessaire
    file(MAKE_DIRECTORY ${CMAKE_BINARY_DIR})
    
    # Configurer le fichier de service
    configure_file(
        ${CMAKE_SOURCE_DIR}/open-yolo.service.in
        ${CMAKE_BINARY_DIR}/open-yolo.service
        @ONLY
    )
    
    # Installer le fichier de service
    install(FILES ${CMAKE_BINARY_DIR}/open-yolo.service
        DESTINATION ${CMAKE_INSTALL_SYSTEMD_SYSTEM_UNITDIR}
        PERMISSIONS OWNER_READ OWNER_WRITE GROUP_READ WORLD_READ
        COMPONENT systemd
    )
    
    # Activer le service si demandé
    if(ENABLE_SYSTEMD_SERVICE)
        install(CODE "
            message(STATUS \"Enabling Open-Yolo systemd service...\")
            execute_process(
                COMMAND systemctl --user daemon-reload
                COMMAND systemctl --user enable open-yolo.service
                WORKING_DIRECTORY ${CMAKE_INSTALL_PREFIX}
            )
        " COMPONENT systemd)
    endif()
endif()

# Création du package
set(CPACK_PACKAGE_NAME "${PROJECT_NAME}")
set(CPACK_PACKAGE_VERSION "${PROJECT_VERSION}")
set(CPACK_PACKAGE_DESCRIPTION "Gestionnaire de curseurs personnalisés pour Linux")
set(CPACK_PACKAGE_VENDOR "Open-Yolo Team")
set(CPACK_PACKAGE_CONTACT "contact@open-yolo.org")
set(CPACK_DEBIAN_PACKAGE_DEPENDS 
    "libgtkmm-3.0-dev (>= 3.24.0), \
     libsdl2-dev (>= 2.0.10), \
     libsdl2-image-dev (>= 2.0.5), \
     libglew-dev (>= 2.1.0), \
     libx11-dev, \
     libxrandr-dev, \
     libxcursor-dev, \
     libxi-dev, \
     libxinerama-dev, \
     libxxf86vm-dev"
)
set(CPACK_DEBIAN_PACKAGE_SECTION "utils")
set(CPACK_DEBIAN_PACKAGE_HOMEPAGE "https://github.com/yourusername/open-yolo")
set(CPACK_RPM_PACKAGE_LICENSE "MIT")
set(CPACK_RPM_PACKAGE_GROUP "Applications/System")
set(CPACK_RPM_PACKAGE_URL "https://github.com/yourusername/open-yolo")
set(CPACK_RPM_PACKAGE_REQUIRES 
    "gtkmm30 >= 3.24.0, \
     SDL2 >= 2.0.10, \
     SDL2_image >= 2.0.5, \
     glew >= 2.1.0, \
     libX11, \
     libXrandr, \
     libXcursor, \
     libXi, \
     libXinerama, \
     libXxf86vm"
)

include(CPack)
