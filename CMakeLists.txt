cmake_minimum_required(VERSION 3.15)

# Configuration du projet
project(OpenYolo
    VERSION 1.1.0
    DESCRIPTION "Gestionnaire de curseurs personnalisés"
    LANGUAGES CXX C
)

# Set default build type to Debug if not specified
if(NOT CMAKE_BUILD_TYPE AND NOT CMAKE_CONFIGURATION_TYPES)
    set(CMAKE_BUILD_TYPE Debug CACHE STRING "Choose the type of build" FORCE)
    set_property(CACHE CMAKE_BUILD_TYPE PROPERTY STRINGS "Debug" "Release" "MinSizeRel" "RelWithDebInfo")
endif()

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
    set(ENABLE_LTO OFF)
    set(ENABLE_COVERAGE OFF)
    
    message(STATUS "Windows build detected")
else()
    # Original Linux-specific settings
    set(UNIX ON)
    message(STATUS "Unix/Linux build detected")
    
    # Détection de CachyOS/Arch Linux
    if (EXISTS "/etc/arch-release")
        set(ARCH_LINUX TRUE)
        message(STATUS "Arch Linux/CachyOS detected")
        add_definitions(-DARCH_LINUX)
    endif()
endif()

# Ajout du répertoire cmake au chemin de recherche des modules
list(APPEND CMAKE_MODULE_PATH ${CMAKE_SOURCE_DIR}/cmake)

# Configuration C++
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_POSITION_INDEPENDENT_CODE ON)

# Options de construction
option(ENABLE_LOGGING "Activer les logs de débogage" ON)
option(ENABLE_WARNINGS_AS_ERRORS "Traiter les avertissements comme des erreurs" OFF)  # Disabled by default on Windows
option(BUILD_TESTS "Construire les tests unitaires" OFF)
option(ENABLE_COVERAGE "Activer la couverture de code" OFF)
option(ENABLE_ASAN "Activer AddressSanitizer" OFF)
option(ENABLE_UNITY_BUILD "Activer les builds Unity pour une compilation plus rapide" ON)

# Compiler detection and settings
if(MSVC)
    # MSVC compiler settings
    add_compile_options(
        $<$<CONFIG:Debug>:/Od /Zi>  # Debug info and optimizations
        $<$<CONFIG:Release>:/O2>    # Optimize for speed
        /W4                        # Warning level 4
        /WX-                       # Don't treat warnings as errors by default
        /EHsc                      # C++ exceptions
        /MP                        # Multi-processor compilation
    )
    
    # Disable specific warnings that are too noisy
    add_compile_options(
        /wd4251  # class 'std::XXX' needs to have dll-interface
        /wd4275  # non dll-interface class used as base for dll-interface class
    )
    
    # Set Windows SDK version if not set
    if(NOT CMAKE_VS_WINDOWS_TARGET_PLATFORM_VERSION)
        set(CMAKE_VS_WINDOWS_TARGET_PLATFORM_VERSION "10.0.19041.0")
    endif()
    
    # Use Unicode character set
    add_definitions(-DUNICODE -D_UNICODE)
    
    # Disable min/max macros that conflict with std::min/max
    add_definitions(-DNOMINMAX)
    
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

if(ENABLE_UNITY_BUILD)
    set(CMAKE_UNITY_BUILD ON)
    message(STATUS "Builds Unity activés")
endif()

# Désactiver les avertissements de dépréciation pour GTKmm
add_definitions(-DGTKMM_DISABLE_DEPRECATED)
add_compile_options($<$<COMPILE_LANGUAGE:CXX>:-Wno-deprecated-declarations>)

# Configuration des bibliothèques requises
if(WIN32)
    # Windows-specific package finding
    find_package(OpenGL REQUIRED)
    
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
    set(GTKMM3_LIBRARIES ${GTKMM3_LIBRARY} ${GIOMM2_4_LIBRARY} ${GLIBMM2_4_LIBRARY} 
                         ${CAIROMM1_0_LIBRARY} ${SIGC2_LIBRARY} ${OPENGL_gl_LIBRARY})
    set(SDL2_LIBRARIES ${SDL2_LIBRARY} ${SDL2_IMAGE_LIBRARY} ${OPENGL_gl_LIBRARY})
    
    # Add Windows-specific libraries
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
else()
    # Linux/Unix package finding using pkg-config
    find_package(PkgConfig REQUIRED)
    
    # GTKmm et dépendances
    pkg_check_modules(GTKMM3 REQUIRED gtkmm-3.0)
    pkg_check_modules(GIOMM2_4 REQUIRED giomm-2.4)
    pkg_check_modules(GLIBMM2_4 REQUIRED glibmm-2.4)
    pkg_check_modules(CAIROMM1_0 REQUIRED cairomm-1.0)
    pkg_check_modules(SIGC2 REQUIRED sigc++-2.0)
    
    # SDL2 et SDL2_image via pkg-config
    pkg_check_modules(SDL2 REQUIRED sdl2)
    pkg_check_modules(SDL2_IMAGE REQUIRED SDL2_image)
    
    # Combine all libraries
    set(ALL_LIBS 
        ${GTKMM3_LIBRARIES} 
        ${GIOMM2_4_LIBRARIES} 
        ${GLIBMM2_4_LIBRARIES} 
        ${CAIROMM1_0_LIBRARIES} 
        ${SIGC2_LIBRARIES} 
        ${SDL2_LIBRARIES} 
        ${SDL2_IMAGE_LIBRARIES}
    )
    
    # Add X11 and other Unix-specific libraries
    if(UNIX AND NOT APPLE)
        find_package(X11 REQUIRED)
        if(X11_FOUND)
            include_directories(${X11_INCLUDE_DIR})
            set(ALL_LIBS ${ALL_LIBS} ${X11_LIBRARIES} X11 Xrandr Xcursor Xinerama Xi)
        endif()
    endif()
    
    message(STATUS "Using Unix/Linux pkg-config for libraries")
endif()

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
