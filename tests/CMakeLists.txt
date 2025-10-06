# Fichier CMake pour les tests

# Activer les tests si l'option est activée
if(BUILD_TESTS)
    # Trouver Google Test
    find_package(GTest REQUIRED)
    include(GoogleTest)

    # Définir les sources de test
    set(TEST_SOURCES
        TestCursorManager.cpp
        test_cursormanager.cpp
        test_cursor.cpp
        test_gui.cpp
        test_wayland_backend.cpp
        test_theme_manager.cpp
    )

    # Créer l'exécutable de test principal
    add_executable(${PROJECT_NAME}-tests ${TEST_SOURCES})

    # Inclure les répertoires nécessaires
    target_include_directories(${PROJECT_NAME}-tests
        PRIVATE
            ${CMAKE_SOURCE_DIR}/include
            ${CMAKE_SOURCE_DIR}/src
            ${GTKMM3_INCLUDE_DIRS}
            ${GIOMM2_4_INCLUDE_DIRS}
            ${SDL2_INCLUDE_DIRS}
            ${SDL2_IMAGE_INCLUDE_DIRS}
            ${GIF_INCLUDE_DIRS}
            ${OPENGL_INCLUDE_DIR}
    )

    # Ajout des dépendances pour Wayland si disponible
    if(WAYLAND_CLIENT_FOUND AND WAYLAND_EGL_FOUND AND WAYLAND_CURSOR_FOUND AND XKBCOMMON_FOUND)
        target_include_directories(${PROJECT_NAME}-tests
            PRIVATE
                ${WAYLAND_CLIENT_INCLUDE_DIRS}
                ${WAYLAND_EGL_INCLUDE_DIRS}
                ${WAYLAND_CURSOR_INCLUDE_DIRS}
                ${XKBCOMMON_INCLUDE_DIRS}
        )
        
        target_link_libraries(${PROJECT_NAME}-tests
            PRIVATE
                ${WAYLAND_CLIENT_LIBRARIES}
                ${WAYLAND_EGL_LIBRARIES}
                ${WAYLAND_CURSOR_LIBRARIES}
                ${XKBCOMMON_LIBRARIES}
        )
    endif()

    # Définitions de compilation
    target_compile_definitions(${PROJECT_NAME}-tests
        PRIVATE
            ${GTKMM_CFLAGS_OTHER}
    )

    # Lier les dépendances
    target_link_libraries(${PROJECT_NAME}-tests
        PRIVATE
            GTest::GTest
            GTest::Main
            config
            cursormanager
            displaymanager
            gui
            inputmanager
            ${GTKMM3_LIBRARIES}
            ${GIOMM2_4_LIBRARIES}
            ${GLIBMM2_4_LIBRARIES}
            ${GLIBMM2_4_LIBRARIES}-2.4
            ${SDL2_LIBRARIES}
            ${SDL2_IMAGE_LIBRARIES}
            ${GLEW_LIBRARIES}
            ${GIF_LIBRARIES}
            OpenGL::GL
            OpenGL::GLU
            ${CMAKE_THREAD_LIBS_INIT}
            dl
            gmodule-2.0
            gobject-2.0
            glib-2.0
    )
    
    # Créer un exécutable séparé pour le test Wayland
    if(WAYLAND_CLIENT_FOUND)
        add_executable(test_wayland_shortcuts 
            test_wayland_shortcuts.cpp
        )
        
        target_include_directories(test_wayland_shortcuts
            PRIVATE
                ${CMAKE_SOURCE_DIR}/include
                ${GTKMM3_INCLUDE_DIRS}
                ${GIOMM2_4_INCLUDE_DIRS}
                ${GLIBMM2_4_INCLUDE_DIRS}
                ${GLIB2_INCLUDE_DIRS}
        )
        
        target_link_libraries(test_wayland_shortcuts
            PRIVATE
                inputmanager
                displaymanager
                ${GTKMM3_LIBRARIES}
                ${GIOMM2_4_LIBRARIES}
                ${GLIBMM2_4_LIBRARIES}
                ${WAYLAND_CLIENT_LIBRARIES}
                ${WAYLAND_EGL_LIBRARIES}
                ${WAYLAND_CURSOR_LIBRARIES}
                ${XKBCOMMON_LIBRARIES}
                dl
                gtk-3
                gdk-3
                gio-2.0
                gobject-2.0
                glib-2.0
        )
        
        # Installer l'exécutable de test
        install(TARGETS test_wayland_shortcuts
            RUNTIME DESTINATION bin
        )
    endif()
    # Activer la couverture de code si demandé
    if(ENABLE_COVERAGE)
        target_compile_options(${PROJECT_NAME}-tests
            PRIVATE --coverage
        )
        target_link_libraries(${PROJECT_NAME}-tests
            PRIVATE --coverage
        )
    endif()

    # Activer les sanitizers si demandé
    if(ENABLE_ASAN)
        target_compile_options(${PROJECT_NAME}-tests
            PRIVATE -fsanitize=address -fno-omit-frame-pointer
        )
        target_link_libraries(${PROJECT_NAME}-tests
            PRIVATE -fsanitize=address
        )
    endif()

    # Créer le répertoire pour les résultats des tests
    file(MAKE_DIRECTORY ${CMAKE_BINARY_DIR}/test_results)

    # Copier les shaders vers le répertoire de build
    file(GLOB SHADER_FILES "${CMAKE_SOURCE_DIR}/src/cursormanager/shaders/*.glsl")
    file(MAKE_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/shaders)
    file(COPY ${SHADER_FILES} DESTINATION ${CMAKE_CURRENT_BINARY_DIR}/shaders)

    # Ajouter les tests à CTest
    gtest_discover_tests(${PROJECT_NAME}-tests
        EXTRA_ARGS --gtest_output=xml:${CMAKE_BINARY_DIR}/test_results/${PROJECT_NAME}-tests.xml
    )

    # Installation des binaires de test
    install(TARGETS ${PROJECT_NAME}-tests
        RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
    )

    # Message de configuration
    message(STATUS "Tests activés")
    message(STATUS "  - Exécuter les tests avec: ctest -V")
    if(ENABLE_COVERAGE)
        message(STATUS "  - Couverture de code activée")
    endif()
    if(ENABLE_ASAN)
        message(STATUS "  - AddressSanitizer activé")
    endif()
endif()
