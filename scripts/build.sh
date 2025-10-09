#!/bin/bash

# Configuration
BUILD_DIR="build"
BUILD_TYPE="Release"

# Création du répertoire de build
mkdir -p "${BUILD_DIR}"

# Configuration avec CMake
cmake -S . -B "${BUILD_DIR}" \
  -G "Ninja" \
  -DCMAKE_BUILD_TYPE="${BUILD_TYPE}" \
  -DCMAKE_EXPORT_COMPILE_COMMANDS=ON

# Construction
cmake --build "${BUILD_DIR}" --config "${BUILD_TYPE}"
