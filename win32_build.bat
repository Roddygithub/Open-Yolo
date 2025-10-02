@echo off
setlocal enabledelayedexpansion

:: Check if Visual Studio environment is set up
if not defined VisualStudioVersion (
    echo Setting up Visual Studio environment...
    call "C:\Program Files\Microsoft Visual Studio\2022\Community\VC\Auxiliary\Build\vcvars64.bat"
    if errorlevel 1 (
        echo Failed to set up Visual Studio environment.
        echo Please install Visual Studio 2022 with C++ workload or update the path in this script.
        exit /b 1
    )
)

:: Create build directory
if not exist "build" mkdir build
cd build

:: Configure with CMake
echo Configuring project with CMake...
cmake .. -G "Visual Studio 17 2022" -A x64 ^
    -DCMAKE_BUILD_TYPE=Debug ^
    -DCMAKE_EXPORT_COMPILE_COMMANDS=ON ^
    -DENABLE_TESTS=ON ^
    -DGTK3_PATH=C:/gtkmm64 ^
    -DSDL2_PATH=C:/SDL2

if errorlevel 1 (
    echo CMake configuration failed.
    exit /b 1
)

echo.
echo Configuration successful! You can now open the project in Visual Studio or build from the command line:
echo   - Open OpenYolo.sln in Visual Studio

echo.
echo Or build from the command line with:
echo   cmake --build . --config Debug
echo   cmake --build . --config Release

endlocal
