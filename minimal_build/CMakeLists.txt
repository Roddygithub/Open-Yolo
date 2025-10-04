cmake_minimum_required(VERSION 3.10)
project(OpenYoloMinimal)

# Définir la version C++ requise
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Trouver les packages nécessaires
find_package(PkgConfig REQUIRED)
pkg_check_modules(GTKMM3 REQUIRED gtkmm-3.0)
pkg_check_modules(GTK3 REQUIRED gtk+-3.0)

# Définir les répertoires d'inclusion
include_directories(
    ${GTKMM3_INCLUDE_DIRS}
    ${GTK3_INCLUDE_DIRS}
)

# Définir les options de compilation
add_compile_options(
    ${GTKMM3_CFLAGS_OTHER}
    ${GTK3_CFLAGS_OTHER}
)

# Trouver la bibliothèque JSON
find_package(nlohmann_json 3.2.0 REQUIRED)

# Ajouter les fichiers source
set(SOURCES
    main_minimal.cpp
    Logger.cpp
    ThemeManager.cpp
    ProfileManager.cpp
)

# Ajouter l'exécutable
add_executable(open-yolo-minimal ${SOURCES})

# Lier les bibliothèques
target_link_libraries(open-yolo-minimal
    ${GTKMM3_LIBRARIES}
    ${GTK3_LIBRARIES}
    nlohmann_json::nlohmann_json
    pthread
    stdc++fs
)

# Options de compilation pour le mode debug
set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -g -O0 -Wall -Wextra -pthread")
set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -O3 -DNDEBUG -pthread")

# Activer C++17
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Activer les avertissements
if(MSVC)
    add_compile_options(/W4 /WX)
else()
    add_compile_options(-Wall -Wextra -Wpedantic -Werror)
endif()
