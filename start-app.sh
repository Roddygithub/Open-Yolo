#!/bin/bash

# Enable debug mode and exit on error
set -euo pipefail

# Configuration
LOG_DIR="/var/log/open-yolo"
LOG_FILE="$LOG_DIR/open-yolo-$(date +%Y%m%d-%H%M%S).log"
CONFIG_DIR="${XDG_CONFIG_HOME:-$HOME/.config}/open-yolo"
CACHE_DIR="${XDG_CACHE_HOME:-$HOME/.cache}/open-yolo"
DATA_DIR="${XDG_DATA_HOME:-$HOME/.local/share}/open-yolo"

# Create necessary directories
mkdir -p "$LOG_DIR" "$CONFIG_DIR" "$CACHE_DIR" "$DATA_DIR"
chmod 755 "$LOG_DIR" "$CONFIG_DIR" "$CACHE_DIR" "$DATA_DIR"

# Function to log messages
log() {
    local level="${1:-INFO}"
    local message="${2:-}"
    local timestamp=$(date '+%Y-%m-%d %H:%M:%S')
    echo "[$timestamp] [$level] $message" | tee -a "$LOG_FILE"
}

# Initialize logging
{
    log "INFO" "=== Starting OpenYolo Application ==="
    log "INFO" "Version: $(cat /workspace/VERSION 2>/dev/null || echo 'unknown')"
    log "INFO" "Hostname: $(hostname)"
    log "INFO" "User: $(whoami) (UID: $(id -u), GID: $(id -g))"
} >> "$LOG_FILE" 2>&1

# Function to check and install dependencies
check_dependencies() {
    local deps=("$@")
    local missing=()
    
    for dep in "${deps[@]}"; do
        if ! command -v "$dep" >/dev/null 2>&1; then
            missing+=("$dep")
        fi
    done
    
    if [ ${#missing[@]} -gt 0 ]; then
        log "WARNING" "Missing dependencies: ${missing[*]}"
        
        if command -v apt-get >/dev/null 2>&1; then
            log "INFO" "Attempting to install missing dependencies..."
            apt-get update && apt-get install -y "${missing[@]}" || {
                log "ERROR" "Failed to install dependencies"
                return 1
            }
        else
            log "ERROR" "Please install the following packages: ${missing[*]}"
            return 1
        fi
    fi
    
    return 0
}

# Set up environment variables
setup_environment() {
    log "INFO" "Setting up environment..."
    
    # Set display from environment or use default
    export DISPLAY=${DISPLAY:-:99}
    export XAUTHORITY=${XAUTHORITY:-/tmp/.Xauthority}
    
    # Set up XDG runtime directory
    export XDG_RUNTIME_DIR="/tmp/runtime-$(id -u)"
    mkdir -p "$XDG_RUNTIME_DIR"
    chmod 700 "$XDG_RUNTIME_DIR"
    
    # GTK environment
    export GTK_THEME="Adwaita"
    export GDK_BACKEND="x11"
    export NO_AT_BRIDGE=1
    export CLUTTER_BACKEND="x11"
    
    # Performance optimizations
    export G_SLICE="always-malloc"
    export G_DEBUG="gc-friendly"
    export G_ENABLE_DIAGNOSTIC=0
    
    # Disable client-side decorations and animations
    export GTK_CSD=0
    export GTK_OVERLAY_SCROLLING=0
    
    # Log environment for debugging
    log "DEBUG" "Environment variables:"
    env | grep -E '^DISPLAY|^XAUTHORITY|^XDG_|^GTK_|^GDK_|^G_|^CLUTTER_|^LD_LIBRARY_PATH' | sort | while read -r line; do
        log "DEBUG" "  $line"
    done
    
    return 0
}

# Check X server connection
check_xserver() {
    log "INFO" "Checking X server connection..."
    
    if ! command -v xdpyinfo >/dev/null 2>&1; then
        log "ERROR" "xdpyinfo not found. Please install x11-utils package."
        return 1
    fi
    
    if ! xdpyinfo >/dev/null 2>&1; then
        log "ERROR" "Cannot connect to X server on $DISPLAY"
        log "TROUBLESHOOTING" "1. Make sure Xvfb is running"
        log "TROUBLESHOOTING" "2. Check display $DISPLAY is accessible"
        log "TROUBLESHOOTING" "3. Verify /tmp/.X11-unix is mounted"
        log "TROUBLESHOOTING" "4. Check Xauthority file permissions"
        return 1
    fi
    
    log "INFO" "X server connection successful"
    log "DEBUG" "X server info:"
    xdpyinfo | grep -E '^name of display|^version|^vendor string|^default screen' | while read -r line; do
        log "DEBUG" "  $line"
    done
    
    return 0
}

# Find the application binary
find_binary() {
    local binary_name="OpenYolo"
    local search_paths=(
        "/workspace/build/bin/$binary_name"
        "/workspace/build/$binary_name"
        "/workspace/src/$binary_name"
        "/usr/local/bin/$binary_name"
        "/usr/bin/$binary_name"
        "./$binary_name"
    )
    
    for path in "${search_paths[@]}"; do
        if [ -f "$path" ] && [ -x "$path" ]; then
            echo "$path"
            return 0
        fi
    done
    
    log "ERROR" "Could not find $binary_name executable in any of:"
    for path in "${search_paths[@]}"; do
        log "ERROR" "  - $path"
    done
    
    log "DEBUG" "Available files in /workspace:"
    find /workspace -type f -executable -name "$binary_name*" -o -name "*.so*" | sort | while read -r file; do
        log "DEBUG" "  $file"
    done
    
    return 1
}

# Check library dependencies
check_libraries() {
    local binary_path="$1"
    
    log "INFO" "Checking library dependencies for $binary_path"
    
    if ! command -v ldd >/dev/null 2>&1; then
        log "WARNING" "ldd not found, cannot check library dependencies"
        return 0
    fi
    
    if ! ldd "$binary_path" >/dev/null 2>&1; then
        log "ERROR" "Failed to check shared library dependencies"
        log "DEBUG" "ldd output:"
        ldd -v "$binary_path" 2>&1 | while read -r line; do
            log "DEBUG" "  $line"
        done
        
        log "DEBUG" "Library search paths:"
        ldconfig -v 2>/dev/null | grep -v "^\s*$" | while read -r line; do
            log "DEBUG" "  $line"
        done
        
        return 1
    fi
    
    log "INFO" "All library dependencies resolved successfully"
    return 0
}

# Main execution
main() {
    # Check for required dependencies
    local required_deps=(
        "gtk3-demo" "glxinfo" "xrandr" "xdpyinfo"
    )
    
    if ! check_dependencies "${required_deps[@]}"; then
        log "ERROR" "Failed to resolve dependencies"
        exit 1
    fi
    
    # Set up environment
    setup_environment || {
        log "ERROR" "Failed to set up environment"
        exit 1
    }
    
    # Check X server connection
    check_xserver || {
        log "ERROR" "X server connection check failed"
        exit 1
    }
    
    # Find the application binary
    local binary_path
    binary_path=$(find_binary) || {
        log "ERROR" "Could not find OpenYolo executable"
        exit 1
    }
    
    log "INFO" "Found OpenYolo at: $binary_path"
    
    # Check library dependencies
    check_libraries "$binary_path" || {
        log "ERROR" "Library dependency check failed"
        exit 1
    }
    
    # Set library path
    export LD_LIBRARY_PATH="/workspace/build/lib:${LD_LIBRARY_PATH:-}"
    
    # Change to the binary directory
    local binary_dir=$(dirname "$binary_path")
    cd "$binary_dir" || {
        log "ERROR" "Could not change to directory: $binary_dir"
        exit 1
    }
    
    log "INFO" "Starting OpenYolo from $(pwd)"
    log "DEBUG" "Command: $binary_path $*"
    log "DEBUG" "Working directory: $(pwd)"
    log "DEBUG" "LD_LIBRARY_PATH=$LD_LIBRARY_PATH"
    
    # Run the application
    local exit_code=0
    
    if [ "${DEBUG:-0}" = "1" ]; then
        log "INFO" "Running in debug mode with strace..."
        strace -o "$LOG_DIR/strace-$(date +%s).log" -f -s 1000 \
            "$binary_path" "$@" 2>&1 | tee -a "$LOG_FILE"
        exit_code=${PIPESTATUS[0]}
    else
        log "INFO" "Starting OpenYolo..."
        "$binary_path" "$@" 2>&1 | tee -a "$LOG_FILE"
        exit_code=${PIPESTATUS[0]}
    fi
    
    log "INFO" "OpenYolo exited with code $exit_code"
    exit $exit_code
}

# Execute main function
main "$@"

# Capture the exit code
APP_EXIT_CODE=${PIPESTATUS[0]}
set -e

# Log the exit status
if [ $APP_EXIT_CODE -eq 0 ]; then
    log "$BINARY_NAME exited successfully with code $APP_EXIT_CODE"
else
    log "ERROR: $BINARY_NAME exited with code $APP_EXIT_CODE"
    log "Please check the logs at $LOG_FILE for more details"
    
    # If we have a core dump, log its location
    if [ -f "core" ] || [ -f "core.*" ]; then
        log "Core dump detected!"
        log "To debug, run: gdb $BINARY_NAME core*"
    fi
fi

exit $APP_EXIT_CODE

# Show the last 50 lines of the log for debugging
echo "=== Last 50 lines of log ===" | tee -a "$LOG_FILE"
tail -n 50 "$LOG_FILE" | tee -a "$LOG_FILE"
echo "============================" | tee -a "$LOG_FILE"

# If there's a core dump, log it
if [ -f "/tmp/core" ]; then
    log "Core dump found at /tmp/core"
    gdb -batch -ex "bt" ./OpenYolo /tmp/core 2>&1 | tee -a "$LOG_FILE"
fi

# Clean up
if [ -n "$XVFB_PID" ]; then
    log "Stopping Xvfb (PID: $XVFB_PID)"
    kill -TERM "$XVFB_PID" 2>/dev/null || true
fi

exit $APP_EXIT_CODE
    kill $XVFB_PID 2>/dev/null
    exit 1
fi

# Run with error handling
if ! ./OpenYolo "$@"; then
    echo "Application crashed with exit code $?"
    kill $XVFB_PID 2>/dev/null
    exit 1
fi

# Cleanup
kill $XVFB_PID 2>/dev/null
exit 0
