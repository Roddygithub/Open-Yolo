# Configuration for tests
set(TEST_SOURCES
    test_cursormanager.cpp
    test_gui.cpp
)

# Find required packages
find_package(GTest REQUIRED)
find_package(OpenGL REQUIRED)
find_package(SDL2 REQUIRED)
find_package(PkgConfig REQUIRED)
pkg_check_modules(GTKMM REQUIRED gtkmm-3.0>=3.22.0)

# Find SDL2_image using pkg-config
pkg_check_modules(SDL2IMAGE REQUIRED SDL2_image>=2.0.5)
include(GoogleTest)

# Add test executable
add_executable(open-yolo-tests ${TEST_SOURCES})

# Link libraries
target_link_libraries(open-yolo-tests
    PRIVATE
    GTest::GTest
    GTest::Main
    ${CMAKE_THREAD_LIBS_INIT}
    OpenGL::GL
    ${SDL2_LIBRARIES}
    ${SDL2IMAGE_LIBRARIES}
    ${GTKMM_LIBRARIES}
    OpenYoloLib
)

# Include directories
target_include_directories(open-yolo-tests
    PRIVATE
    ${CMAKE_SOURCE_DIR}/include
    ${GTKMM_INCLUDE_DIRS}
    ${SDL2_INCLUDE_DIRS}
    ${SDL2IMAGE_INCLUDE_DIRS}
)

# Compile definitions
target_compile_definitions(open-yolo-tests
    PRIVATE
    ${GTKMM_CFLAGS_OTHER}
)

# Copy shaders to build directory
file(GLOB SHADER_FILES "${CMAKE_SOURCE_DIR}/src/cursormanager/shaders/*.glsl")
file(COPY ${SHADER_FILES} DESTINATION ${CMAKE_CURRENT_BINARY_DIR}/shaders)

# Add tests
gtest_discover_tests(open-yolo-tests)
