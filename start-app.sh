#!/bin/bash

# Enable debug mode
set -euo pipefail

# Log file for debugging
LOG_FILE="/tmp/open-yolo-$(date +%s).log"
echo "Starting OpenYolo at $(date)" > "$LOG_FILE"

# Function to log messages
log() {
    echo "[$(date '+%Y-%m-%d %H:%M:%S')] $1" | tee -a "$LOG_FILE"
}

log "=== Starting OpenYolo Application ==="

# Set display from environment or use default
export DISPLAY=${DISPLAY:-:99}
log "Using DISPLAY=$DISPLAY"

# Set XAUTHORITY if not set
if [ -z "${XAUTHORITY:-}" ]; then
    export XAUTHORITY="/tmp/.Xauthority"
    log "Setting XAUTHORITY to $XAUTHORITY"
fi

# Create necessary directories
log "Setting up X11 environment..."
mkdir -p /tmp/.X11-unix
chmod 1777 /tmp/.X11-unix 2>/dev/null || {
    log "Warning: Could not set permissions on /tmp/.X11-unix"
}

# Set up XDG runtime directory
export XDG_RUNTIME_DIR="/tmp/runtime-$(id -u)"
mkdir -p "$XDG_RUNTIME_DIR"
chmod 700 "$XDG_RUNTIME_DIR"

# Set up GTK environment
log "Configuring GTK environment..."

# GTK basic configuration
export GTK_THEME="Adwaita"
export GTK_THEME_VARIANT="light"
export GDK_BACKEND="x11"
export NO_AT_BRIDGE=1

# Disable overlay scrolling and client-side decorations
export GTK_OVERLAY_SCROLLING=0
export GTK_CSD=0

# Memory management settings
export G_SLICE=always-malloc
export G_DEBUG=gc-friendly

# Disable specific warnings and debug output
export G_ENABLE_DIAGNOSTIC=0

# Disable graphical effects to improve stability
export CLUTTER_VBLANK=none
export CLUTTER_PAINT=disable-clipped-redraws,disable-culling

# Configure debug settings (enable only if needed)
# export GTK_DEBUG=1
# export GDK_DEBUG=1
# export G_MESSAGES_DEBUG=all
# export G_DEBUG=fatal-warnings

# Create GTK settings directory
log "Creating GTK settings..."
mkdir -p "$HOME/.config/gtk-3.0"
cat > "$HOME/.config/gtk-3.0/settings.ini" << 'EOL'
[Settings]
gtk-theme-name=Adwaita
gtk-icon-theme-name=Adwaita
gtk-font-name=Sans 10
gtk-cursor-theme-name=Adwaita
gtk-enable-animations=0
gtk-application-prefer-dark-theme=false
EOL

# Set environment for GTK
export GTK_DATA_PREFIX=/usr

# Log environment for debugging
log "=== Environment Variables ==="
env | grep -E 'GTK|GDK|G_|DISPLAY|XAUTHORITY|XDG|LD_LIBRARY_PATH' | tee -a "$LOG_FILE"
log "============================"

# Check X server connection
log "Checking X server connection..."
if ! xdpyinfo >/dev/null 2>&1; then
    log "Error: Cannot connect to X server on $DISPLAY"
    log "Troubleshooting steps:"
    log "1. Make sure Xvfb is running in the xvfb container"
    log "2. Check that the display $DISPLAY is accessible"
    log "3. Verify that /tmp/.X11-unix is mounted correctly"
    log "4. Check Xauthority file permissions"
    exit 1
else
    log "Successfully connected to X server on $DISPLAY"
    log "X server info:"
    xdpyinfo | grep -E '^name of display|^version|^vendor string|^default screen' | tee -a "$LOG_FILE"
fi

# Check for required GTK components
log "Checking for required GTK components..."
if ! command -v gtk3-demo &>/dev/null; then
    log "Warning: gtk3-demo not found. Installing required packages..."
    if command -v apt-get &>/dev/null; then
        sudo apt-get update && sudo apt-get install -y gtk-3-examples || {
            log "Warning: Failed to install gtk-3-examples"
        }
    fi
fi

# Change to application directory
APP_DIR="/workspace/build/bin"
log "Changing to application directory: $APP_DIR"
cd "$APP_DIR" || {
    log "Error: Could not change to directory: $APP_DIR"
    log "Available directories in $(dirname "$APP_DIR"):"
    ls -la "$(dirname "$APP_DIR")" | tee -a "$LOG_FILE"
    exit 1
}

# Check if the binary exists
BINARY_NAME="OpenYolo"
BINARY_PATH="./$BINARY_NAME"

# Check multiple possible locations for the binary
if [ ! -f "$BINARY_PATH" ]; then
    # Try in the build directory
    if [ -f "/workspace/build/$BINARY_NAME" ]; then
        BINARY_PATH="/workspace/build/$BINARY_NAME"
    # Try in the bin directory
    elif [ -f "/workspace/build/bin/$BINARY_NAME" ]; then
        BINARY_PATH="/workspace/build/bin/$BINARY_NAME"
    # Try in the source directory
    elif [ -f "/workspace/src/$BINARY_NAME" ]; then
        BINARY_PATH="/workspace/src/$BINARY_NAME"
    # Try in /usr/local/bin
    elif [ -f "/usr/local/bin/$BINARY_NAME" ]; then
        BINARY_PATH="/usr/local/bin/$BINARY_NAME"
    else
        log "Error: $BINARY_NAME binary not found in any standard location"
        log "Searched in:"
        log "  - $(pwd)/$BINARY_NAME"
        log "  - /workspace/build/$BINARY_NAME"
        log "  - /workspace/build/bin/$BINARY_NAME"
        log "  - /workspace/src/$BINARY_NAME"
        log "  - /usr/local/bin/$BINARY_NAME"
        log "\nAvailable files in /workspace:"
        find /workspace -type f -name "$BINARY_NAME" -o -name "$BINARY_NAME*" | tee -a "$LOG_FILE"
        log "\nPlease build the application first"
        exit 1
    fi
fi

log "Found $BINARY_NAME at: $BINARY_PATH"

# Set the library path if needed
export LD_LIBRARY_PATH="/workspace/build/lib:${LD_LIBRARY_PATH:-}"

# Check for required libraries
log "Checking for required libraries in $BINARY_PATH..."
if ! ldd "$BINARY_PATH" >/dev/null 2>&1; then
    log "Error: Failed to check shared library dependencies for $BINARY_PATH"
    log "Trying to run ldd with more verbose output..."
    ldd -v "$BINARY_PATH" 2>&1 | tee -a "$LOG_FILE"
    
    # Show library search paths
    log "\nLibrary search paths:"
    ldconfig -v 2>/dev/null | grep -v "^\s*$" | tee -a "$LOG_FILE"
    
    # Show environment variables
    log "\nEnvironment variables:"
    env | grep -E 'LD_LIBRARY_PATH|PATH' | tee -a "$LOG_FILE"
    
    exit 1
fi

# Run the application
log "Starting $BINARY_NAME from $BINARY_PATH..."
log "Command: $BINARY_PATH $*"
log "Current directory: $(pwd)"
log "LD_LIBRARY_PATH=$LD_LIBRARY_PATH"

# Run the application with error handling
set +e
if [ "${DEBUG:-0}" = "1" ]; then
    log "Running in debug mode with strace..."
    strace -o "/tmp/open-yolo-strace-$(date +%s).log" -f -s 1000 \
        "$BINARY_PATH" "$@" 2>&1 | tee -a "$LOG_FILE"
else
    log "Executing: $BINARY_PATH $*"
    "$BINARY_PATH" "$@" 2>&1 | tee -a "$LOG_FILE"
fi

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
