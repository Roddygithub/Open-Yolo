# Fichier CMake pour le répertoire src

# Liste des fichiers sources
set(SOURCES
    main.cpp
    config/ConfigManager.cpp
    cursormanager/CursorManager.cpp
    cursormanager/ShaderProgram.cpp
    displaymanager/DisplayManager.cpp
    gui/MainWindow.cpp
    input/InputManager.cpp
    log/Logger.cpp
)

# Création de l'exécutable
add_executable(OpenYolo ${SOURCES})

# Configuration des propriétés de la cible
target_include_directories(OpenYolo PRIVATE
    ${CMAKE_SOURCE_DIR}/include
    ${CMAKE_SOURCE_DIR}/src
)

# Configuration des propriétés de compilation
target_compile_features(OpenYolo PRIVATE cxx_std_17)

target_compile_options(OpenYolo PRIVATE
    $<$<CONFIG:Debug>:-g -O0>
    $<$<CONFIG:Release>:-O3>
    $<$<CONFIG:RelWithDebInfo>:-O2 -g>
    $<$<CONFIG:MinSizeRel>:-Os>
    -Wall 
    -Wextra 
    -Wno-pedantic 
    -Wno-conversion 
    -Wno-sign-conversion
    -Wno-deprecated-declarations
)

# Configuration des avertissements
if(MSVC)
    target_compile_options(OpenYolo PRIVATE /W4 /WX)
    target_compile_definitions(OpenYolo PRIVATE _CRT_SECURE_NO_WARNINGS)
else()
    # Les avertissements sont déjà définis dans les options de compilation globales
    # Désactiver temporairement les avertissements traités comme des erreurs
    # if(ENABLE_WARNINGS_AS_ERRORS)
    #     target_compile_options(OpenYolo PRIVATE -Werror)
    # endif()
endif()

# Configuration des dépendances
find_package(PkgConfig REQUIRED)
pkg_check_modules(GTKMM3 REQUIRED gtkmm-3.0)
pkg_check_modules(GIOMM2_4 REQUIRED giomm-2.4)
pkg_check_modules(GLIBMM2_4 REQUIRED glibmm-2.4)
pkg_check_modules(CAIROMM1_0 REQUIRED cairomm-1.0)
pkg_check_modules(SIGC2 REQUIRED sigc++-2.0)
pkg_check_modules(SDL2 REQUIRED sdl2)
pkg_check_modules(SDL2_IMAGE REQUIRED SDL2_image)
find_package(OpenGL REQUIRED)
find_package(GLEW REQUIRED)
find_package(X11 REQUIRED)

# Liaison des bibliothèques
target_link_libraries(OpenYolo PRIVATE
    ${GTKMM3_LIBRARIES}
    ${GIOMM2_4_LIBRARIES}
    ${GLIBMM2_4_LIBRARIES}
    ${CAIROMM1_0_LIBRARIES}
    ${SIGC2_LIBRARIES}
    ${SDL2_LIBRARIES}
    ${SDL2_IMAGE_LIBRARIES}
    ${OPENGL_LIBRARIES}
    ${GLEW_LIBRARIES}
    ${X11_LIBRARIES}
    ${X11_X11_LIB}
    ${X11_Xrandr_LIB}
    ${X11_Xcursor_LIB}
    ${X11_Xinerama_LIB}
    ${X11_Xi_LIB}
    dl
    pthread
)

# Installation
target_include_directories(OpenYolo PUBLIC
    $<BUILD_INTERFACE:${CMAKE_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:include>
)

install(TARGETS OpenYolo
    RUNTIME DESTINATION bin
    LIBRARY DESTINATION lib
    ARCHIVE DESTINATION lib
)

install(DIRECTORY ${CMAKE_SOURCE_DIR}/include/ DESTINATION include
    FILES_MATCHING PATTERN "*.hpp" PATTERN "*.h"
)
