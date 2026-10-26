#!/usr/bin/env bash
set -euo pipefail

ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
cd "$ROOT"

usage() {
    cat <<'EOF'
Usage: bash tools/build_legacy_prototype.sh [--prepare|--check]

  no option   Prepare generated Legacy content and build pokeemerald.gba.
  --prepare   Apply/validate generated Legacy content only; do not compile.
  --check     Prepare, build, then run the focused legacy_system.c tests.
EOF
}

MODE="build"
if (( $# > 1 )); then
    usage >&2
    exit 2
elif (( $# == 1 )); then
    case "$1" in
        --prepare)
            MODE="prepare"
            ;;
        --check)
            MODE="check"
            ;;
        -h|--help)
            usage
            exit 0
            ;;
        *)
            echo "ERROR: unknown option: $1" >&2
            usage >&2
            exit 2
            ;;
    esac
fi

PATCH="patches/legacy_core.patch"

if git apply --recount --check "$PATCH" >/dev/null 2>&1; then
    echo "Applying Pokémon Legacy core integration..."
    git apply --recount "$PATCH"
elif git apply --recount --reverse --check "$PATCH" >/dev/null 2>&1; then
    echo "Pokémon Legacy core integration is already applied; continuing."
else
    echo "ERROR: $PATCH cannot be applied cleanly and does not appear to be already applied." >&2
    echo "Start from a clean checkout of legacy-system-prototype and try again." >&2
    exit 1
fi

python3 tools/apply_legacy_prototype_content.py
python3 tools/apply_legacy_transition_guard.py
python3 tools/apply_legacy_hoenn_prototype.py
python3 tools/apply_legacy_terminal_routing.py
python3 tools/apply_legacy_daycare_guard.py
python3 tools/validate_legacy_prototype.py

if ! git diff --check; then
    echo "ERROR: generated prototype edits failed git diff --check." >&2
    exit 1
fi

if [[ "$MODE" == "prepare" ]]; then
    echo "Pokémon Legacy prototype source prepared and semantically validated."
    exit 0
fi

JOBS="${JOBS:-$(getconf _NPROCESSORS_ONLN 2>/dev/null || echo 2)}"

echo "Building Pokémon Legacy with $JOBS parallel jobs..."
make -j"$JOBS" -O all

if [[ "$MODE" == "check" ]]; then
    echo "Running focused Pokémon Legacy tests..."
    GAME_VERSION=EMERALD TEST=1 TESTS=legacy_system.c make -j"$JOBS" check
fi

echo
if [[ "$MODE" == "check" ]]; then
    echo "Build and focused Legacy tests complete."
else
    echo "Build complete."
fi
echo "Open $ROOT/pokeemerald.gba in your GBA emulator for local testing."
echo "See docs/PROTOTYPE_PLAYTEST.md for the acceptance playthrough."
