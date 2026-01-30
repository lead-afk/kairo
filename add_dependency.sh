#!/usr/bin/env bash
set -e

if [ -z "$1" ]; then
  echo "Usage: $0 <package_name> [version]"
  echo "Example: $0 fmt"
  echo "Example: $0 boost-system 1.82.0"
  exit 1
fi

PACKAGE="$1"
VERSION="${2:-}"

echo "Adding dependency: $PACKAGE"

# Add to CMakeLists.txt if not already present
if ! grep -q "find_package($PACKAGE" CMakeLists.txt; then
  # Add find_package before add_executable
  sed -i "/add_executable/i find_package($PACKAGE CONFIG REQUIRED)" CMakeLists.txt

  # Add target_link_libraries after add_executable if not present
  if ! grep -q "target_link_libraries" CMakeLists.txt; then
    echo "" >> CMakeLists.txt
    echo "target_link_libraries(\${PROJECT_NAME} PRIVATE $PACKAGE::$PACKAGE)" >> CMakeLists.txt
  else
    # Append to existing target_link_libraries
    sed -i "s/target_link_libraries(\${PROJECT_NAME} PRIVATE/target_link_libraries(\${PROJECT_NAME} PRIVATE $PACKAGE::$PACKAGE/" CMakeLists.txt
  fi

  echo "Added $PACKAGE to CMakeLists.txt"
else
  echo "$PACKAGE already in CMakeLists.txt"
fi

# Install with vcpkg
echo "Installing $PACKAGE with vcpkg..."
if [ -n "$VERSION" ]; then
  vcpkg install "$PACKAGE:$VERSION"
else
  vcpkg install "$PACKAGE"
fi

echo ""
echo "Dependency added successfully!"
echo "Rebuild your project with: ./build_debug.sh or ./build_release.sh"
