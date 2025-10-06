cmake_minimum_required(VERSION 3.15)

# Configuration du projet
project(OpenYolo
    VERSION 1.1.0
    DESCRIPTION "Gestionnaire de curseurs personnalisés"
    LANGUAGES CXX C
)

# Configuration des options
option(BUILD_TESTS "Build the test suite" OFF)  # Désactivé temporairement
option(BUILD_EXAMPLES "Build examples" OFF)
option(ENABLE_LOGGING "Enable debug logging" ON)
option(ENABLE_COVERAGE "Enable code coverage" OFF)
option(ENABLE_ASAN "Enable AddressSanitizer" OFF)
option(ENABLE_UBSAN "Enable UndefinedBehaviorSanitizer" OFF)

# Configuration des chemins d'inclusion
include_directories(${CMAKE_SOURCE_DIR}/include)
include_directories(${CMAKE_SOURCE_DIR}/src)

# Recherche des dépendances avec pkg-config
find_package(PkgConfig REQUIRED)
pkg_check_modules(CAIROMM REQUIRED cairomm-1.0)
pkg_check_modules(GTKMM3 REQUIRED gtkmm-3.0)
pkg_check_modules(GDKMM3 REQUIRED gdk-3.0)
pkg_check_modules(GLIBMM2_4 REQUIRED glibmm-2.4)
pkg_check_modules(SIGC2 REQUIRED sigc++-2.0)

# Afficher les chemins trouvés
message(STATUS "CairoMM include dirs: ${CAIROMM_INCLUDE_DIRS}")
message(STATUS "GTKMM3 include dirs: ${GTKMM3_INCLUDE_DIRS}")
message(STATUS "GDKMM3 include dirs: ${GDKMM3_INCLUDE_DIRS}")

# Inclusion des sous-répertoires
add_subdirectory(src)

# Configuration du type de build
if(NOT CMAKE_BUILD_TYPE AND NOT CMAKE_CONFIGURATION_TYPES)
    set(CMAKE_BUILD_TYPE Debug CACHE STRING "Choose the type of build" FORCE)
    set_property(CACHE CMAKE_BUILD_TYPE PROPERTY STRINGS "Debug" "Release" "MinSizeRel" "RelWithDebInfo")
endif()

# Configuration du standard C++
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Configuration des répertoires de sortie
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

# Détection du système d'exploitation
if(WIN32)
    add_definitions(-DWIN32_LEAN_AND_MEAN -DNOMINMAX)
    set(UNIX OFF)
    set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")
    
    # Options de compilation pour Windows
    add_compile_options(
        /W4
        /WX-
        /EHsc
        /MP
        /permissive-
    )
    
    # Options d'édition des liens pour Windows
    set(CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} /INCREMENTAL:NO")
    set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} /INCREMENTAL:NO")
    
    # Détection des dépendances Windows
    find_package(OpenGL REQUIRED)
    
    # Configuration des chemins pour les bibliothèques Windows
    # (À adapter selon votre configuration)
    
else()
    # Configuration pour Linux/Unix
    set(UNIX ON)
    
    # Options de compilation pour Linux/Unix
    add_compile_options(
        -Wall
        -Wextra
        -Wpedantic
        -Werror=return-type
        -fPIC
    )
    
    # Détection des dépendances avec pkg-config
    find_package(PkgConfig REQUIRED)
    
    # Détection de GLEW
    find_package(PkgConfig REQUIRED)
    pkg_check_modules(GLEW REQUIRED glew)
    
    # Détection de X11
    if(UNIX AND NOT APPLE)
        find_package(X11 REQUIRED)
        if(X11_FOUND)
            message(STATUS "X11 support enabled")
            
            # Détection de Wayland
            pkg_check_modules(WAYLAND_CLIENT wayland-client)
            pkg_check_modules(WAYLAND_EGL wayland-egl)
            pkg_check_modules(WAYLAND_CURSOR wayland-cursor)
            pkg_check_modules(XKBCOMMON xkbcommon)
            
            if(WAYLAND_CLIENT_FOUND AND WAYLAND_EGL_FOUND AND WAYLAND_CURSOR_FOUND AND XKBCOMMON_FOUND)
                message(STATUS "Wayland support enabled")
                add_definitions(-DHAVE_WAYLAND=1)
            else()
                message(STATUS "Wayland support disabled - required libraries not found")
            endif()
            include_directories(${X11_INCLUDE_DIR})
            add_definitions(-DX11_ENABLED)
        endif()
    endif()
    
    # Détection des dépendances SDL2
    find_package(SDL2 REQUIRED)
    find_package(SDL2_image REQUIRED)
    
    # Configuration des chemins pour SDL2
    if(NOT TARGET SDL2::SDL2)
        add_library(SDL2::SDL2 INTERFACE IMPORTED)
        set_target_properties(SDL2::SDL2 PROPERTIES
            INTERFACE_INCLUDE_DIRECTORIES "${SDL2_INCLUDE_DIRS}"
            INTERFACE_LINK_LIBRARIES "${SDL2_LIBRARIES}"
        )
    endif()
    
    # Configuration des chemins pour SDL2_image
    if(NOT TARGET SDL2_image::SDL2_image)
        add_library(SDL2_image::SDL2_image INTERFACE IMPORTED)
        set_target_properties(SDL2_image::SDL2_image PROPERTIES
            INTERFACE_INCLUDE_DIRECTORIES "${SDL2_IMAGE_INCLUDE_DIRS}"
            INTERFACE_LINK_LIBRARIES "${SDL2_IMAGE_LIBRARIES}"
        )
    endif()
    
    # Détection des dépendances GTK
    pkg_check_modules(GTKMM3 REQUIRED gtkmm-3.0)
    pkg_check_modules(GIOMM2_4 REQUIRED giomm-2.4)
    pkg_check_modules(GLIBMM2_4 REQUIRED glibmm-2.4)
    pkg_check_modules(CAIROMM1_0 REQUIRED cairomm-1.0)
    pkg_check_modules(SIGC2 REQUIRED sigc++-2.0)
    
    # Détection d'autres dépendances
    find_package(OpenGL REQUIRED)
    find_package(CURL REQUIRED)
    
    # Utilisation d'OpenGL directement au lieu de GLEW
    find_package(OpenGL REQUIRED)
    
    # Création d'une cible vide pour GLEW pour la compatibilité
    add_library(GLEW::GLEW INTERFACE IMPORTED)
    target_link_libraries(GLEW::GLEW INTERFACE OpenGL::GL)
    
    # Définition pour utiliser OpenGL au lieu de GLEW
    add_compile_definitions(USE_OPENGL)
    
    # Configuration pour Wayland (optionnel)
    pkg_check_modules(WAYLAND_CLIENT wayland-client)
    pkg_check_modules(XKBCOMMON xkbcommon)
    if(WAYLAND_CLIENT_FOUND AND XKBCOMMON_FOUND)
        add_definitions(-DWAYLAND_ENABLED)
    endif()
    
    # Configuration pour la couverture de code
    if(ENABLE_COVERAGE)
        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} --coverage -fprofile-arcs -ftest-coverage")
        set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} --coverage")
    endif()
    
    # Configuration des sanitizers
    if(ENABLE_ASAN)
        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fsanitize=address -fno-omit-frame-pointer")
        set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -fsanitize=address")
    endif()
    
    if(ENABLE_UBSAN)
        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fsanitize=undefined -fno-sanitize-recover=all")
        set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -fsanitize=undefined")
    endif()
endif()

# Configuration des définitions de compilation
if(ENABLE_LOGGING)
    add_definitions(-DENABLE_LOGGING)
endif()

# Configuration des répertoires d'inclusion
include_directories(
    ${CMAKE_SOURCE_DIR}/include
    ${CMAKE_BINARY_DIR}
    ${SDL2_INCLUDE_DIRS}
    ${SDL2_IMAGE_INCLUDE_DIRS}
    ${GTKMM3_INCLUDE_DIRS}
    ${GIOMM2_4_INCLUDE_DIRS}
    ${GLEW_INCLUDE_DIRS}
    ${CURL_INCLUDE_DIRS}
    ${X11_INCLUDE_DIR}
)

# Définition des bibliothèques à lier
set(COMMON_LIBS
    ${GTKMM3_LIBRARIES}
    ${GIOMM2_4_LIBRARIES}
    ${GLIBMM2_4_LIBRARIES}
    ${CAIROMM1_0_LIBRARIES}
    ${SIGC2_LIBRARIES}
    ${SDL2_LIBRARIES}
    ${SDL2_IMAGE_LIBRARIES}
    ${CURL_LIBRARIES}
    ${X11_LIBRARIES}
    X11 Xrandr Xcursor Xinerama Xi
    OpenGL::GL
    dl
    pthread
    ${WAYLAND_CLIENT_LIBRARIES}
    ${XKBCOMMON_LIBRARIES}
    ${GIO_UNIX_LIBRARIES}
)

# Définition des bibliothèques à lier
set(COMMON_LIBS
    ${GTKMM3_LIBRARIES}
    ${GIOMM2_4_LIBRARIES}
    ${GLIBMM2_4_LIBRARIES}
    ${CAIROMM1_0_LIBRARIES}
    ${SIGC2_LIBRARIES}
    SDL2::SDL2
    SDL2_image::SDL2_image
    ${WAYLAND_CLIENT_LIBRARIES}
    ${XKBCOMMON_LIBRARIES}
    ${GIO_UNIX_LIBRARIES}
    GLEW::GLEW
    ${CURL_LIBRARIES}
    ${X11_LIBRARIES}
    X11 Xrandr Xcursor Xinerama Xi
    OpenGL::GL
    dl
    pthread
)

# Installation des fichiers
install(TARGETS ${PROJECT_NAME}
    RUNTIME DESTINATION bin
    LIBRARY DESTINATION lib
    ARCHIVE DESTINATION lib
)

# Installation des ressources
install(DIRECTORY ${CMAKE_SOURCE_DIR}/resources/
    DESTINATION ${CMAKE_INSTALL_DATADIR}/open-yolo
)

# Construction des exemples
if(BUILD_EXAMPLES)
    add_subdirectory(examples)
endif()

# Construction des tests
if(BUILD_TESTS)
    enable_testing()
    add_subdirectory(tests)
    
    # Ajout d'une cible pour exécuter les tests
    add_custom_target(run-tests
        COMMAND ${CMAKE_CTEST_COMMAND} --output-on-failure
        WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
        DEPENDS ${PROJECT_NAME}-tests
    )
    
    # Configuration de CPack pour les packages
    include(InstallRequiredSystemLibraries)
    set(CPACK_RESOURCE_FILE_LICENSE "${CMAKE_SOURCE_DIR}/LICENSE")
    set(CPACK_PACKAGE_VERSION_MAJOR "1")
    set(CPACK_PACKAGE_VERSION_MINOR "1")
    set(CPACK_PACKAGE_VERSION_PATCH "0")
    set(CPACK_PACKAGE_DESCRIPTION_FILE "${CMAKE_SOURCE_DIR}/README.md")
    set(CPACK_PACKAGE_DESCRIPTION_SUMMARY "Gestionnaire de curseurs personnalisés")
    include(CPack)
endif()

# Message de fin de configuration
message(STATUS "Configuration de OpenYolo terminée")
message(STATUS "  Build type: ${CMAKE_BUILD_TYPE}")
message(STATUS "  Install prefix: ${CMAKE_INSTALL_PREFIX}")
message(STATUS "  Build tests: ${BUILD_TESTS}")
message(STATUS "  Build examples: ${BUILD_EXAMPLES}")
message(STATUS "  Enable logging: ${ENABLE_LOGGING}")
message(STATUS "  Enable coverage: ${ENABLE_COVERAGE}")
