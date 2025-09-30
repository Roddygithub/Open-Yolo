# Fichier CMake pour les tests

# Activer les tests si l'option est activée
if(BUILD_TESTS)
    # Trouver Google Test
    find_package(GTest REQUIRED)
    include(GoogleTest)

    # Définir les sources de test
    set(TEST_SOURCES
        test_cursormanager.cpp
        test_gui.cpp
        test_input.cpp
        test_displaymanager.cpp
    )

    # Créer l'exécutable de test
    add_executable(${PROJECT_NAME}-tests ${TEST_SOURCES})

    # Inclure les répertoires nécessaires
    target_include_directories(${PROJECT_NAME}-tests
        PRIVATE
            ${CMAKE_SOURCE_DIR}/include
            ${CMAKE_SOURCE_DIR}/src
            ${GTKMM_INCLUDE_DIRS}
            ${SDL2_INCLUDE_DIRS}
            ${SDL2_IMAGE_INCLUDE_DIRS}
            ${GLEW_INCLUDE_DIRS}
            ${GIF_INCLUDE_DIRS}
            ${OPENGL_INCLUDE_DIR}
    )

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
            ${CMAKE_THREAD_LIBS_INIT}
            config
            cursormanager
            displaymanager
            gui
            input
            ${GTKMM_LIBRARIES}
            ${SDL2_LIBRARIES}
            ${SDL2_IMAGE_LIBRARIES}
            ${GLEW_LIBRARIES}
            ${GIF_LIBRARIES}
            OpenGL::GL
    )

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
