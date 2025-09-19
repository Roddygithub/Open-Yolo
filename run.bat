@echo off
setlocal enabledelayedexpansion

:: Script pour gérer facilement le conteneur Open-Yolo sous Windows

:: Couleurs
set "GREEN=<NUL set /p=^[[32m"
set "YELLOW=<NUL set /p=^[[33m"
set "NC=<NUL set /p=^[[0m"

:: Fonction pour afficher l'aide
:show_help
echo Usage: %~nx0 [commande]
echo Commandes disponibles:
echo   build         - Construit l'image Docker
echo   run           - Lance l'application (mode production)
echo   debug         - Lance un shell dans le conteneur pour le débogage
echo   logs          - Affiche les logs de l'application
echo   clean         - Nettoie les conteneurs et images inutilisés
echo   rebuild       - Reconstruit complètement l'application
echo   help          - Affiche cette aide
goto :eof

:: Vérifier si Docker est installé
where docker >nul 2>&1
if %ERRORLEVEL% NEQ 0 (
    echo Erreur: Docker n'est pas installé. Veuillez installer Docker d'abord.
    exit /b 1
)

:: Vérifier si Docker Compose est disponible
set DOCKER_COMPOSE_CMD=docker-compose
%DOCKER_COMPOSE_CMD% version >nul 2>&1
if %ERRORLEVEL% NEQ 0 (
    set DOCKER_COMPOSE_CMD="docker compose"
    %DOCKER_COMPOSE_CMD% version >nul 2>&1
    if %ERRORLEVEL% NEQ 0 (
        echo Erreur: Docker Compose n'est pas installé. Veuillez l'installer d'abord.
        exit /b 1
    )
)

:: Vérifier les arguments
if "%~1"=="" (
    call :show_help
    exit /b 1
)

:: Traitement des commandes
if "%~1"=="build" (
    echo Construction de l'image Docker...
    %DOCKER_COMPOSE_CMD% build
    goto :eof
)

if "%~1"=="run" (
    echo Lancement de l'application...
    %DOCKER_COMPOSE_CMD% up open-yolo
    goto :eof
)

if "%~1"=="debug" (
    echo Lancement du mode débogage...
    echo Utilisez 'make' pour recompiler et './bin/OpenYolo' pour lancer l'application
    %DOCKER_COMPOSE_CMD% run --service-ports open-yolo-debug //bin/bash
    goto :eof
)

if "%~1"=="logs" (
    %DOCKER_COMPOSE_CMD% logs -f open-yolo
    goto :eof
)

if "%~1"=="clean" (
    echo Nettoyage des conteneurs et images inutilisés...
    docker system prune -f
    goto :eof
)

if "%~1"=="rebuild" (
    echo Reconstruction complète de l'application...
    %DOCKER_COMPOSE_CMD% build --no-cache
    %DOCKER_COMPOSE_CMD% up -d --force-recreate
    goto :eof
)

if "%~1"=="help" (
    call :show_help
    goto :eof
)

echo Commande inconnue: %~1
call :show_help
exit /b 1
