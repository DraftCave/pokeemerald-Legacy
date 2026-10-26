#!/usr/bin/env bash
set -euo pipefail

ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
cd "$ROOT"

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

if ! git diff --check; then
    echo "ERROR: generated prototype edits failed git diff --check." >&2
    exit 1
fi

JOBS="${JOBS:-$(getconf _NPROCESSORS_ONLN 2>/dev/null || echo 2)}"

echo "Building Pokémon Legacy with $JOBS parallel jobs..."
make -j"$JOBS" -O all

echo
echo "Build complete. Open $ROOT/pokeemerald.gba in your GBA emulator for local testing."
echo "See docs/PROTOTYPE_PLAYTEST.md for the acceptance playthrough."
