cmake_minimum_required(VERSION 3.15)

# Configuration du projet
project(OpenYolo
    VERSION 1.0.0
    DESCRIPTION "Gestionnaire de curseurs personnalisés pour Linux"
    LANGUAGES CXX C
)

# Ajout du répertoire cmake au chemin de recherche des modules
list(APPEND CMAKE_MODULE_PATH ${CMAKE_SOURCE_DIR}/cmake)

# Configuration C++
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_POSITION_INDEPENDENT_CODE ON)

# Détection du compilateur
if (CMAKE_CXX_COMPILER_ID MATCHES "GNU")
    if (CMAKE_CXX_COMPILER_VERSION VERSION_LESS "13.2.1")
        message(WARNING "GCC version ${CMAKE_CXX_COMPILER_VERSION} is older than the recommended 13.2.1 for C++17 support")
    endif()
elseif (CMAKE_CXX_COMPILER_ID MATCHES "Clang")
    if (CMAKE_CXX_COMPILER_VERSION VERSION_LESS "16.0.0")
        message(WARNING "Clang version ${CMAKE_CXX_COMPILER_VERSION} is older than the recommended 16.0.0 for full C++17 support")
    endif()
endif()

# Détection de CachyOS/Arch Linux
if (EXISTS "/etc/arch-release")
    set(ARCH_LINUX TRUE)
    message(STATUS "Arch Linux/CachyOS detected")
    add_definitions(-DARCH_LINUX)
endif()

# Options de construction
option(ENABLE_LOGGING "Activer les logs de débogage" ON)
option(ENABLE_WARNINGS_AS_ERRORS "Traiter les avertissements comme des erreurs" ON)
option(ENABLE_LTO "Activer l'optimisation de l'éditeur de liens" ON)
option(BUILD_TESTS "Construire les tests unitaires" OFF)
option(ENABLE_COVERAGE "Activer la couverture de code" OFF)
option(ENABLE_ASAN "Activer AddressSanitizer" OFF)

# Configuration des avertissements
if(MSVC)
    add_compile_options(/W4 /WX)
    add_definitions(-D_CRT_SECURE_NO_WARNINGS)
else()
    add_compile_options(
        -Wall 
        -Wextra 
        -Wno-pedantic 
        -Wno-conversion 
        -Wno-sign-conversion
        -Wno-deprecated-declarations
    )
    # Désactiver temporairement les avertissements traités comme des erreurs
    # if(ENABLE_WARNINGS_AS_ERRORS)
    #     add_compile_options(-Werror)
    # endif()
    
    # Optimisation LTO (Link Time Optimization)
    if(ENABLE_LTO)
        include(CheckIPOSupported)
        check_ipo_supported(RESULT LTO_SUPPORTED OUTPUT output)
        if(LTO_SUPPORTED)
            set(CMAKE_INTERPROCEDURAL_OPTIMIZATION TRUE)
            message(STATUS "LTO activé")
        else()
            message(STATUS "LTO non supporté: ${output}")
        endif()
    endif()
endif()

# Désactiver les avertissements de dépréciation pour GTKmm
add_definitions(-DGTKMM_DISABLE_DEPRECATED)
add_compile_options($<$<COMPILE_LANGUAGE:CXX>:-Wno-deprecated-declarations>)

# Configuration des chemins d'inclusion
include_directories(
    ${CMAKE_SOURCE_DIR}/include
    ${CMAKE_SOURCE_DIR}/src
    ${GTKMM3_INCLUDE_DIRS}
    ${GIOMM2_4_INCLUDE_DIRS}
    ${GLIBMM2_4_INCLUDE_DIRS}
    ${CAIROMM1_0_INCLUDE_DIRS}
    ${SIGC2_INCLUDE_DIRS}
    ${SDL2_INCLUDE_DIRS}
    ${SDL2_IMAGE_INCLUDE_DIRS}
    ${OPENGL_INCLUDE_DIR}
    ${GLEW_INCLUDE_DIRS}
    ${X11_INCLUDE_DIR}
)

# Configuration des bibliothèques requises
find_package(PkgConfig REQUIRED)

# GTKmm et dépendances
pkg_check_modules(GTKMM3 REQUIRED gtkmm-3.0)
pkg_check_modules(GIOMM2_4 REQUIRED giomm-2.4)
pkg_check_modules(GLIBMM2_4 REQUIRED glibmm-2.4)
pkg_check_modules(CAIROMM1_0 REQUIRED cairomm-1.0)
pkg_check_modules(SIGC2 REQUIRED sigc++-2.0)

# SDL2 et SDL2_image via pkg-config
find_package(PkgConfig REQUIRED)
pkg_check_modules(SDL2 REQUIRED sdl2)
pkg_check_modules(SDL2_IMAGE REQUIRED SDL2_image)

# OpenGL
find_package(OpenGL REQUIRED)
find_package(GLEW REQUIRED)

# X11
find_package(X11 REQUIRED)

# Ajout du sous-répertoire src
add_subdirectory(src)

# Ajout des tests si activés
if(BUILD_TESTS)
    enable_testing()
    add_subdirectory(tests)
endif()

# Configuration des chemins d'installation
include(GNUInstallDirs)
set(CMAKE_INSTALL_LIBDIR ${CMAKE_INSTALL_LIBDIR})
set(CMAKE_INSTALL_BINDIR ${CMAKE_INSTALL_BINDIR})
set(CMAKE_INSTALL_INCLUDEDIR ${CMAKE_INSTALL_INCLUDEDIR})

# Configuration CPack pour la génération de paquets
set(CPACK_PACKAGE_NAME "open-yolo")
set(CPACK_PACKAGE_VENDOR "Open-Yolo Project")
set(CPACK_PACKAGE_DESCRIPTION_SUMMARY "Gestionnaire de curseurs personnalisés pour Linux")
set(CPACK_PACKAGE_VERSION_MAJOR "1")
set(CPACK_PACKAGE_VERSION_MINOR "0")
set(CPACK_PACKAGE_VERSION_PATCH "0")
set(CPACK_PACKAGE_VERSION "${CPACK_PACKAGE_VERSION_MAJOR}.${CPACK_PACKAGE_VERSION_MINOR}.${CPACK_PACKAGE_VERSION_PATCH}")
set(CPACK_PACKAGE_CONTACT "your.email@example.com")
set(CPACK_PACKAGE_HOMEPAGE_URL "https://github.com/Roddygithub/Open-Yolo")

# Configuration DEB
set(CPACK_DEBIAN_PACKAGE_MAINTAINER "Your Name <your.email@example.com>")
set(CPACK_DEBIAN_PACKAGE_DEPENDS "libgtkmm-3.0-1v5, libsdl2-2.0-0, libsdl2-image-2.0-0, libglew2.2, libgif7, libx11-6, libxrandr2, libxcursor1, libxinerama1, libxi6, libgl1")
set(CPACK_DEBIAN_PACKAGE_SECTION "x11")
set(CPACK_DEBIAN_PACKAGE_PRIORITY "optional")
set(CPACK_DEBIAN_FILE_NAME DEB-DEFAULT)

# Configuration RPM
set(CPACK_RPM_PACKAGE_LICENSE "MIT")
set(CPACK_RPM_PACKAGE_GROUP "User Interface/X")
set(CPACK_RPM_PACKAGE_REQUIRES "gtkmm30, SDL2, SDL2_image, glew, giflib, libX11, libXrandr, libXcursor, libXinerama, libXi, mesa-libGL")
set(CPACK_RPM_FILE_NAME RPM-DEFAULT)

# Générateurs de paquets
set(CPACK_GENERATOR "TGZ;DEB;RPM")

include(CPack)
