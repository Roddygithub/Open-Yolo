@echo off
setlocal enabledelayedexpansion

:: Vérifier si vcpkg est installé
set VCPKG_ROOT=%USERPROFILE%\vcpkg
set VCPKG_EXE=%VCPKG_ROOT%\vcpkg.exe

if not exist "%VCPKG_EXE%" (
    echo Installation de vcpkg...
    cd /d "%USERPROFILE%"
    git clone https://github.com/Microsoft/vcpkg.git
    cd vcpkg
    call bootstrap-vcpkg.bat
    call vcpkg.exe integrate install
    cd /d "%~dp0"
)

:: Installer les dépendances
echo Installation des dépendances...
call "%VCPKG_EXE%" install @vcpkg.json --triplet x64-windows

:: Créer le répertoire de build
set BUILD_DIR=build
if not exist "%BUILD_DIR%" (
    mkdir "%BUILD_DIR%"
)

:: Configurer avec CMake
echo Configuration du projet avec CMake...
cd "%BUILD_DIR%"
call cmake .. ^
    -DCMAKE_TOOLCHAIN_FILE="%VCPKG_ROOT%\scripts\buildsystems\vcpkg.cmake" ^
    -DCMAKE_BUILD_TYPE=Release ^
    -DVCPKG_TARGET_TRIPLET=x64-windows

if %ERRORLEVEL% neq 0 (
    echo Erreur lors de la configuration CMake
    pause
    exit /b %ERRORLEVEL%
)

:: Compiler le projet
echo Compilation du projet...
call cmake --build . --config Release

if %ERRORLEVEL% neq 0 (
    echo Erreur lors de la compilation
    pause
    exit /b %ERRORLEVEL%
)

:: Copier les DLL requises
echo Copie des DLL requises...
if not exist "Release" mkdir "Release"
copy /Y "%VCPKG_ROOT%\installed\x64-windows\bin\*.dll" "Release\" >nul 2>&1

echo.
echo =========================================
echo Compilation terminee avec succes!
echo Vous pouvez executer le programme avec :
echo cd %BUILD_DIR%\Release
echo OpenYolo.exe
echo =========================================

pause
