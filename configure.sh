#!/bin/bash

set -e

TOOL_NAME="live-vim"          # change this to your actual binary name
BUILD_DIR="build"               # change if your binary is placed in a subdirectory
INSTALL_PATH="/usr/local/bin"

echo "🔧 Running make..."
make

# Check if the binary exists
if [ ! -f "$BUILD_DIR/$TOOL_NAME" ]; then
  echo "❌ Build failed: $TOOL_NAME not found in $BUILD_DIR"
  exit 1
fi

echo "📦 Installing $TOOL_NAME to $INSTALL_PATH (may require sudo)..."
sudo cp "$BUILD_DIR/$TOOL_NAME" "$INSTALL_PATH/"

echo "✅ $TOOL_NAME installed to $INSTALL_PATH"
echo "💡 Run it from anywhere using: $TOOL_NAME"
