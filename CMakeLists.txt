cmake_minimum_required(VERSION 3.15)

# Configuration du projet
project(OpenYolo
    VERSION 1.1.0
    DESCRIPTION "Gestionnaire de curseurs personnalisés"
    LANGUAGES CXX C
)

# Configuration des options
option(BUILD_TESTS "Build the test suite" ON)
option(BUILD_EXAMPLES "Build examples" ON)
option(ENABLE_LOGGING "Enable debug logging" ON)
option(ENABLE_COVERAGE "Enable code coverage" OFF)
option(ENABLE_ASAN "Enable AddressSanitizer" OFF)
option(ENABLE_UBSAN "Enable UndefinedBehaviorSanitizer" OFF)
option(ENABLE_LTO "Enable Link Time Optimization" ON)
option(ENABLE_UNITY_BUILD "Enable Unity builds for faster compilation" OFF)

# Configuration des chemins d'installation
set(CMAKE_INSTALL_PREFIX "/usr" CACHE PATH "Installation prefix")
set(CMAKE_INSTALL_BINDIR "bin" CACHE STRING "Binary installation directory")
set(CMAKE_INSTALL_LIBDIR "lib" CACHE STRING "Library installation directory")
set(CMAKE_INSTALL_INCLUDEDIR "include" CACHE STRING "Header files installation directory")
set(CMAKE_INSTALL_DATADIR "share" CACHE STRING "Data files installation directory")
set(CMAKE_INSTALL_DOCDIR "${CMAKE_INSTALL_DATADIR}/doc/${PROJECT_NAME}" CACHE STRING "Documentation installation directory")

# Configuration des chemins d'inclusion
include_directories(
    ${CMAKE_SOURCE_DIR}/include
    ${CMAKE_SOURCE_DIR}/src
    ${CMAKE_BINARY_DIR}
)

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

# Set default build type to Debug if not specified
if(NOT CMAKE_BUILD_TYPE AND NOT CMAKE_CONFIGURATION_TYPES)
    set(CMAKE_BUILD_TYPE Debug CACHE STRING "Choose the type of build" FORCE)
    set_property(CACHE CMAKE_BUILD_TYPE PROPERTY STRINGS "Debug" "Release" "MinSizeRel" "RelWithDebInfo")
endif()

# Ajout du répertoire cmake au chemin de recherche des modules
list(APPEND CMAKE_MODULE_PATH ${CMAKE_SOURCE_DIR}/cmake)

# Configuration C++
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_POSITION_INDEPENDENT_CODE ON)

# Windows-specific settings
if(WIN32)
    # Enable multi-threaded compilation
    set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>DLL")
    
    # Set output directories
    set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
    set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
    set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
    
    # Set Windows SDK version (use latest available)
    set(CMAKE_VS_WINDOWS_TARGET_PLATFORM_VERSION "10.0.19041.0" CACHE STRING "Windows SDK version to use")
    
    # Set Windows-specific compiler flags
    add_compile_options(
        /MP           # Multi-processor compilation
        /W4           # Warning level 4
        /WX-          # Don't treat warnings as errors
        /EHsc         # C++ exceptions: SEH, Yes with Extern C functions
        /permissive-  # Standards conformance mode for MSVC
    )
    
    # Set Windows-specific linker flags
    set(CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} /INCREMENTAL:NO")
    set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} /INCREMENTAL:NO")
    
    # Find Windows SDK
    find_package(WindowsSDK REQUIRED)
    
    # Set Windows-specific definitions
    add_definitions(-DWIN32_LEAN_AND_MEAN -DNOMINMAX)
    
    # Disable Unix-specific options
    set(UNIX OFF)
    set(ENABLE_COVERAGE OFF)
    
    message(STATUS "Windows build detected")
else()
    # Unix/Linux specific settings
    set(UNIX ON)
    message(STATUS "Unix/Linux build detected")
    
    # Détection de CachyOS/Arch Linux
    if (EXISTS "/etc/arch-release")
        set(ARCH_LINUX TRUE)
        message(STATUS "Arch Linux/CachyOS detected")
        add_definitions(-DARCH_LINUX)
    endif()
    
    # Détection de X11 et Wayland
    find_package(X11)
    if(X11_FOUND)
        message(STATUS "X11 support enabled")
        add_definitions(-DX11_ENABLED)
        
        # Détection de Wayland
        pkg_check_modules(WAYLAND_CLIENT wayland-client)
        pkg_check_modules(WAYLAND_EGL wayland-egl)
        pkg_check_modules(WAYLAND_CURSOR wayland-cursor)
        pkg_check_modules(XKBCOMMON xkbcommon)
        
        if(WAYLAND_CLIENT_FOUND AND WAYLAND_EGL_FOUND AND WAYLAND_CURSOR_FOUND AND XKBCOMMON_FOUND)
            message(STATUS "Wayland support enabled")
            add_definitions(-DHAVE_WAYLAND)
        else()
            message(STATUS "Wayland support disabled - required libraries not found")
        endif()
    endif()
endif()

# Configuration du compilateur
if(MSVC)
    # MSVC compiler settings
    add_compile_options(
        $<$<CONFIG:Debug>:/Od /Zi>  # Debug info and optimizations
        $<$<CONFIG:Release>:/O2>    # Optimize for speed
    )
    
    # Enable parallel builds
    add_compile_options(/MP)
    
    # Disable some common warnings
    add_compile_options(
        /wd4251  # class 'std::XXX' needs to have dll-interface
        /wd4275  # non dll-interface class used as base for dll-interface class
    )
    
    # Use Unicode character set
    add_definitions(-DUNICODE -D_UNICODE)
    
    # Disable some deprecated warnings for external libraries
    add_definitions(-D_SILENCE_ALL_CXX17_DEPRECATION_WARNINGS)
    
    message(STATUS "Using MSVC compiler: ${CMAKE_CXX_COMPILER_ID} ${CMAKE_CXX_COMPILER_VERSION}")
    
elseif(CMAKE_CXX_COMPILER_ID MATCHES "GNU")
    # GCC compiler settings
    if (CMAKE_CXX_COMPILER_VERSION VERSION_LESS "13.2.1")
        message(WARNING "GCC version ${CMAKE_CXX_COMPILER_VERSION} is older than the recommended 13.2.1 for C++17 support")
    endif()
    
    add_compile_options(
        -Wall -Wextra -Wpedantic
        -Wno-pedantic -Wno-conversion -Wno-sign-conversion -Wno-deprecated-declarations
    )
    
    if(ENABLE_WARNINGS_AS_ERRORS)
        add_compile_options(-Werror)
    endif()
    
    message(STATUS "Using GCC compiler: ${CMAKE_CXX_COMPILER_ID} ${CMAKE_CXX_COMPILER_VERSION}")
    
elseif(CMAKE_CXX_COMPILER_ID MATCHES "Clang")
    # Clang compiler settings
    if (CMAKE_CXX_COMPILER_VERSION VERSION_LESS "16.0.0")
        message(WARNING "Clang version ${CMAKE_CXX_COMPILER_VERSION} is older than the recommended 16.0.0 for full C++17 support")
    endif()
    
    add_compile_options(
        -Wall -Wextra -Wpedantic
        -Wno-pedantic -Wno-conversion -Wno-sign-conversion -Wno-deprecated-declarations
    )
    
    if(ENABLE_WARNINGS_AS_ERRORS)
        add_compile_options(-Werror)
    endif()
    
    message(STATUS "Using Clang compiler: ${CMAKE_CXX_COMPILER_ID} ${CMAKE_CXX_COMPILER_VERSION}")
else()
    message(WARNING "Unsupported compiler: ${CMAKE_CXX_COMPILER_ID}")
endif()

# Link Time Optimization (LTO)
if(ENABLE_LTO AND NOT WIN32)  # Disable LTO on Windows by default as it can cause issues
    include(CheckIPOSupported)
    check_ipo_supported(RESULT LTO_SUPPORTED OUTPUT output)
    if(LTO_SUPPORTED)
        set(CMAKE_INTERPROCEDURAL_OPTIMIZATION TRUE)
        message(STATUS "LTO activated")
    else()
        message(STATUS "LTO not supported: ${output}")
    endif()
endif()

# Désactiver les avertissements de dépréciation pour GTKmm
add_definitions(-DGTKMM_DISABLE_DEPRECATED)
add_compile_options($<$<COMPILE_LANGUAGE:CXX>:-Wno-deprecated-declarations>)

# Configuration des bibliothèques requises
if(WIN32)
    # Windows-specific package finding
    find_package(OpenGL REQUIRED)
    find_package(CURL REQUIRED)
    
    # Set paths to your GTK and SDL2 installations
    set(GTK3_PATH "C:/gtkmm64" CACHE PATH "Path to GTK3 installation")
    set(SDL2_PATH "C:/SDL2" CACHE PATH "Path to SDL2 installation")
    
    # Add include and library paths
    include_directories(
        ${GTK3_PATH}/include
        ${GTK3_PATH}/include/gtkmm-3.0
        ${GTK3_PATH}/lib/gtkmm-3.0/include
        ${GTK3_PATH}/include/atkmm-1.6
        ${GTK3_PATH}/include/gdkmm-3.0
        ${GTK3_PATH}/lib/gdkmm-3.0/include
        ${GTK3_PATH}/include/giomm-2.4
        ${GTK3_PATH}/lib/giomm-2.4/include
        ${GTK3_PATH}/include/glib-2.0
        ${GTK3_PATH}/lib/glib-2.0/include
        ${GTK3_PATH}/include/glibmm-2.4
        ${GTK3_PATH}/lib/glibmm-2.4/include
        ${GTK3_PATH}/include/gtk-3.0
        ${GTK3_PATH}/include/cairo
        ${GTK3_PATH}/include/cairomm-1.0
        ${GTK3_PATH}/lib/cairomm-1.0/include
        ${GTK3_PATH}/include/pangomm-1.4
        ${GTK3_PATH}/lib/pangomm-1.4/include
        ${GTK3_PATH}/include/atk-1.0
        ${GTK3_PATH}/include/gdk-pixbuf-2.0
        ${GTK3_PATH}/include/pango-1.0
        ${GTK3_PATH}/include/harfbuzz
        ${GTK3_PATH}/include/fribidi
        ${GTK3_PATH}/include/freetype2
        ${GTK3_PATH}/include/libpng16
        ${GTK3_PATH}/include/pixman-1
        ${GTK3_PATH}/include/sigc++-2.0
        ${GTK3_PATH}/lib/sigc++-2.0/include
        
        # SDL2 includes
        ${SDL2_PATH}/include
        ${SDL2_PATH}/include/SDL2
    )
    
    # Link directories
    link_directories(
        ${GTK3_PATH}/lib
        ${SDL2_PATH}/lib/x64
    )
    
    # Find libraries
    find_library(GTKMM3_LIBRARY NAMES gtkmm-3.0 PATHS ${GTK3_PATH}/lib)
    find_library(GIOMM2_4_LIBRARY NAMES giomm-2.4 PATHS ${GTK3_PATH}/lib)
    find_library(GLIBMM2_4_LIBRARY NAMES glibmm-2.4 PATHS ${GTK3_PATH}/lib)
    find_library(CAIROMM1_0_LIBRARY NAMES cairomm-1.0 PATHS ${GTK3_PATH}/lib)
    find_library(SIGC2_LIBRARY NAMES sigc-2.0 PATHS ${GTK3_PATH}/lib)
    find_library(SDL2_LIBRARY NAMES SDL2 PATHS ${SDL2_PATH}/lib/x64)
    find_library(SDL2_IMAGE_LIBRARY NAMES SDL2_image PATHS ${SDL2_PATH}/lib/x64)
    
    # Set variables for later use
    set(GTKMM3_LIBRARIES 
        ${GTKMM3_LIBRARY} 
        ${GIOMM2_4_LIBRARY} 
        ${GLIBMM2_4_LIBRARY}
        ${CAIROMM1_0_LIBRARY} 
        ${SIGC2_LIBRARY} 
        ${OPENGL_gl_LIBRARY}
    )
    
    set(SDL2_LIBRARIES 
        ${SDL2_LIBRARY} 
        ${SDL2_IMAGE_LIBRARY} 
        ${OPENGL_gl_LIBRARY}
    )
    
    # Windows-specific libraries
    set(WINDOWS_LIBS
        gdi32
        shell32
        ole32
        oleaut32
        uuid
        comdlg32
        advapi32
        winmm
        imm32
        setupapi
        version
        dinput8
        dxguid
        d3d11
        d3dcompiler
    )
    
    # Combine all libraries
    set(ALL_LIBS ${GTKMM3_LIBRARIES} ${SDL2_LIBRARIES} ${WINDOWS_LIBS})
    
    # Add resource files if any
    if(EXISTS "${CMAKE_SOURCE_DIR}/resources/OpenYolo.rc")
        add_custom_command(
            OUTPUT ${CMAKE_BINARY_DIR}/OpenYolo.res
            COMMAND windres.exe -i ${CMAKE_SOURCE_DIR}/resources/OpenYolo.rc -o ${CMAKE_BINARY_DIR}/OpenYolo.res
            DEPENDS ${CMAKE_SOURCE_DIR}/resources/OpenYolo.rc
        )
        set(ALL_RESOURCES ${CMAKE_BINARY_DIR}/OpenYolo.res)
    endif()
    
    message(STATUS "Using Windows-specific library paths")
    
else()  # Linux/Unix
    # Détection des dépendances avec pkg-config
    find_package(PkgConfig REQUIRED)
    
    # Détection de SDL2 (requis) et SDL2_image (optionnel)
    pkg_check_modules(SDL2 REQUIRED sdl2)
    option(ENABLE_SDL2_IMAGE "Enable SDL2_image support" OFF)
    if(ENABLE_SDL2_IMAGE)
        pkg_check_modules(SDL2_IMAGE REQUIRED sdl2_image)
        add_definitions(-DHAS_SDL2_IMAGE)
        message(STATUS "SDL2_image support enabled")
    else()
        message(STATUS "SDL2_image support disabled")
        set(SDL2_IMAGE_LIBRARIES "")
    endif()
    
    # Détection de GTKmm et dépendances
    pkg_check_modules(GTKMM3 REQUIRED gtkmm-3.0)
    pkg_check_modules(GIOMM2_4 REQUIRED giomm-2.4)
    pkg_check_modules(GLIBMM2_4 REQUIRED glibmm-2.4)
    pkg_check_modules(CAIROMM1_0 REQUIRED cairomm-1.0)
    pkg_check_modules(SIGC2 REQUIRED sigc++-2.0)
    
    # Détection d'OpenGL et CURL
    find_package(OpenGL REQUIRED)
    find_package(CURL REQUIRED)
    
    # Configuration pour OpenGL
    add_compile_definitions(USE_OPENGL)
    
    # Configuration des bibliothèques
    set(ALL_LIBS 
        ${GTKMM3_LIBRARIES}
        ${GIOMM2_4_LIBRARIES}
        ${GLIBMM2_4_LIBRARIES}
        ${CAIROMM1_0_LIBRARIES}
        ${SIGC2_LIBRARIES}
        ${SDL2_LIBRARIES}
        ${SDL2_IMAGE_LIBRARIES}
        ${OPENGL_LIBRARIES}
        ${CURL_LIBRARIES}
    )
    
    # Ajouter les chemins d'inclusion nécessaires
    include_directories(
        ${GTKMM3_INCLUDE_DIRS}
        ${GIOMM2_4_INCLUDE_DIRS}
        ${GLIBMM2_4_INCLUDE_DIRS}
        ${CAIROMM1_0_INCLUDE_DIRS}
        ${SIGC2_INCLUDE_DIRS}
        ${SDL2_INCLUDE_DIRS}
        ${SDL2_IMAGE_INCLUDE_DIRS}
        ${OPENGL_INCLUDE_DIR}
        ${CURL_INCLUDE_DIRS}
    )
    
    # Ajouter les répertoires de bibliothèques
    link_directories(
        ${GTKMM3_LIBRARY_DIRS}
        ${GIOMM2_4_LIBRARY_DIRS}
        ${GLIBMM2_4_LIBRARY_DIRS}
        ${CAIROMM1_0_LIBRARY_DIRS}
        ${SIGC2_LIBRARY_DIRS}
        ${SDL2_LIBRARY_DIRS}
        ${SDL2_IMAGE_LIBRARY_DIRS}
        ${CURL_LIBRARY_DIRS}
    )
    
    message(STATUS "Using Unix/Linux library paths")
endif()

# Inclusion des sous-répertoires
add_subdirectory(src)

# Configuration des chemins d'installation
set(CMAKE_INSTALL_PREFIX "/usr/local" CACHE PATH "Installation directory")
set(CMAKE_INSTALL_BINDIR "${CMAKE_INSTALL_PREFIX}/bin" CACHE PATH "Binary directory")
set(CMAKE_INSTALL_LIBDIR "${CMAKE_INSTALL_PREFIX}/lib" CACHE PATH "Library directory")
set(CMAKE_INSTALL_INCLUDEDIR "${CMAKE_INSTALL_PREFIX}/include" CACHE PATH "Include directory")
set(CMAKE_INSTALL_DATADIR "${CMAKE_INSTALL_PREFIX}/share" CACHE PATH "Data directory")

# Configuration CPack pour la génération de paquets
include(InstallRequiredSystemLibraries)
set(CPACK_PACKAGE_NAME "open-yolo")
set(CPACK_PACKAGE_VENDOR "OpenYolo Team")
set(CPACK_PACKAGE_DESCRIPTION_SUMMARY "Gestionnaire de curseurs personnalisés")
set(CPACK_PACKAGE_VERSION_MAJOR 1)
set(CPACK_PACKAGE_VERSION_MINOR 0)
set(CPACK_PACKAGE_VERSION_PATCH 0)
set(CPACK_PACKAGE_CONTACT "contact@openyolo.org")
set(CPACK_DEBIAN_PACKAGE_MAINTAINER "OpenYolo Team <contact@openyolo.org>")
set(CPACK_DEBIAN_PACKAGE_DEPENDS 
    "libgtkmm-3.0-dev, libsdl2-dev, libsdl2-image-dev, libcurl4-openssl-dev"
)
set(CPACK_RPM_PACKAGE_REQUIRES 
    "gtkmm30-devel, SDL2-devel, SDL2_image-devel, libcurl-devel"
)
set(CPACK_STRIP_FILES TRUE)
set(CPACK_SOURCE_IGNORE_FILES "/.git/;/build/;/.vscode/;/.idea/;*.swp;*.swo")

# Inclure CPack après avoir défini toutes les variables
include(CPack)

# Message de fin de configuration
message(STATUS "Configuration terminée pour ${PROJECT_NAME} ${PROJECT_VERSION}")
message(STATUS "Build type: ${CMAKE_BUILD_TYPE}")
message(STATUS "Installation directory: ${CMAKE_INSTALL_PREFIX}")
message(STATUS "Executables will be installed to: ${CMAKE_INSTALL_BINDIR}")
message(STATUS "Libraries will be installed to: ${CMAKE_INSTALL_LIBDIR}")
message(STATUS "Header files will be installed to: ${CMAKE_INSTALL_INCLUDEDIR}")

# Afficher un message si la configuration est terminée avec succès
if(CMAKE_PROJECT_NAME)
    message(STATUS "${PROJECT_NAME} - Configuration terminée avec succès")
else()
    message(STATUS "Configuration terminée avec succès")
endif()
