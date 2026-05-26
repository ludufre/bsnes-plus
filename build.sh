#!/usr/bin/env bash
set -euo pipefail

# Build script for bsnes-plus on macOS.
# Usage:
#   ./build.sh              # build
#   ./build.sh clean        # clean
#   ./build.sh rebuild      # clean + build
#   ./build.sh run          # build + open the resulting .app
#   ./build.sh install      # build + copy to /Applications

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
cd "$SCRIPT_DIR/bsnes"

# Locate Qt5 (Homebrew, Apple Silicon and Intel paths).
if [[ -z "${QTPATH:-}" ]]; then
  for candidate in /opt/homebrew/opt/qt@5 /usr/local/opt/qt@5; do
    if [[ -d "$candidate" ]]; then
      QTPATH="$candidate"
      break
    fi
  done
fi

if [[ -z "${QTPATH:-}" || ! -d "$QTPATH" ]]; then
  echo "error: Qt5 not found. Install it with: brew install qt@5" >&2
  echo "       Or set QTPATH=/path/to/qt5 before running this script." >&2
  exit 1
fi

JOBS="${JOBS:-$(sysctl -n hw.ncpu)}"
PROFILE="${PROFILE:-compatibility}"

MAKE_ARGS=(platform=osx qtpath="$QTPATH" profile="$PROFILE" -j"$JOBS")

action="${1:-build}"
case "$action" in
  build)
    make "${MAKE_ARGS[@]}"
    echo "built: $SCRIPT_DIR/bsnes+.app"
    ;;
  clean)
    make "${MAKE_ARGS[@]}" clean
    ;;
  rebuild)
    make "${MAKE_ARGS[@]}" clean
    make "${MAKE_ARGS[@]}"
    echo "built: $SCRIPT_DIR/bsnes+.app"
    ;;
  run)
    make "${MAKE_ARGS[@]}"
    open "$SCRIPT_DIR/bsnes+.app"
    ;;
  install)
    make "${MAKE_ARGS[@]}" install
    ;;
  *)
    echo "usage: $0 [build|clean|rebuild|run|install]" >&2
    exit 1
    ;;
esac
