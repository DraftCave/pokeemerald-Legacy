# Pokémon Legacy — Continuity Handoff

This file is the first thing a new ChatGPT session should read before continuing Pokémon Legacy work.

## Repository / active work

- Repository: `DraftCave/pokeemerald-Legacy`
- Base branch: `master` — preserve it; do not implement unfinished work directly here.
- Active development branch: `legacy-system-prototype`
- Active draft PR: `#1 — Prototype the three-generation Legacy System`
- Primary architecture doc: `docs/LEGACY_SYSTEM.md`
- Engine integration patch: `patches/legacy_core.patch`
- Prototype CI workflow: `.github/workflows/legacy-prototype.yml`

## Product goal

Pokémon Legacy is a GBA ROM hack built on `pokeemerald-expansion` with one continuous save spanning three campaigns:

1. Kanto — Gen 1 species pool, fresh Lv. 5 Kanto starter.
2. Johto — Gen 1–2 pool, previous Kanto Pokémon temporarily locked during regional progression; Kanto veterans return at the Johto League checkpoint.
3. Hoenn — Gen 1–3 pool, Kanto + Johto veterans temporarily locked; all previous veterans return at the Hoenn League checkpoint.
4. Postgame — all regional restrictions removed.

The defining mechanic is the **Legacy System**: Pokémon are locked/unlocked by the individual Pokémon's campaign of origin, not by species generation.

Example: a Pikachu caught in Johto is Johto-origin even though Pikachu is a Gen 1 species.

## Critical data rules

Per-Pokémon origin encoding is intentionally designed around two otherwise-unused top-level `BoxPokemon` bits so the Pokémon/save structure size does not increase.

Use this encoding:

- `0` = UNSET / compatibility fallback
- `1` = KANTO
- `2` = JOHTO
- `3` = HOENN

Do **not** change zero to mean Kanto. Zeroed/legacy data naturally starts at zero, so zero must remain UNSET.

Unknown/unset Pokémon must be handled conservatively and deterministically; they must not silently bypass regional restrictions.

## What is already implemented on the branch

The branch contains persistent campaign-state infrastructure in:

- `include/constants/legacy_system.h`
- `include/legacy_system.h`
- `src/legacy_system.c`

Implemented state includes:

- Legacy system version
- current campaign region
- regional badge counters
- regional champion bitmask
- legacy-access bitmask
- region transitions
- League unlock state
- postgame state

The campaign state uses vanilla-unused persistent variables rather than expanding save-block structures.

The current Legacy code also contains the per-Pokémon origin API and safe transition/vaulting framework. Region transitions are intended to move now-ineligible party Pokémon into storage only after a successful PC copy; Pokémon must never be deleted merely because a new region begins.

## Engine integration currently staged

`patches/legacy_core.patch` contains the current integration work against upstream engine files.

It currently stages:

### Pokémon acquisition

- `src/pokemon.c`
  - assigns the current campaign origin to wild captures before they are given to the player
  - assigns the current campaign origin to scripted/gift Pokémon

### Eggs / breeding

- `src/daycare.c`
  - assigns the current campaign origin when a daycare egg is created

### PC enforcement

- `src/pokemon_storage_system.c`
  - adds a clear Legacy-lock message
  - blocks direct withdrawal of an ineligible boxed Pokémon
  - blocks placement/shift actions that would move an ineligible held Pokémon into the party

The desired player-facing message is currently:

`This POKéMON is Legacy-locked until the League.`

### Regional starters

- `src/starter_choose.c`
  - Kanto: Bulbasaur / Charmander / Squirtle
  - Johto: Chikorita / Cyndaquil / Totodile
  - Hoenn: Treecko / Torchic / Mudkip
  - starter choice is derived from the current Legacy campaign region

## CI / build state

A dedicated workflow exists at `.github/workflows/legacy-prototype.yml`.

Its intended sequence is:

1. checkout the branch
2. install the ARM/GBA build toolchain
3. `git apply --check patches/legacy_core.patch`
4. apply the patch
5. build Emerald with `make`
6. run the expansion test suite with `make check`

Important: as of this handoff, do **not** claim that the prototype is compiled, tested, or playable unless a later commit/workflow result proves it. The engine patch has been staged, but a verified green build/test result has not yet been recorded in this handoff.

The workflow intentionally does not publish a compiled commercial ROM artifact.

## Current verified design decisions

- Origin follows the individual Pokémon, not species generation.
- Previous-region Pokémon are preserved, never deleted or level-reset.
- Fresh regional progression starts with a Lv. 5 regional starter.
- Old veterans become usable again at the next region's League checkpoint.
- Hoenn is designed to end with a much harder League and dedicated high-level training content later.
- Finished normal gameplay should use only Gen 1–3 species (#001–386), even though expansion internals may contain newer species/features.
- Use a consistent Gen-III-compatible/modernized battle engine rather than recreating Gen I bugs.
- Save integrity outranks map/UI polish.
- Full-size Kanto/Johto/Hoenn map production must wait until the Legacy loop compiles and is proven.

## Immediate next step — DO THIS FIRST

Do not start drawing full regions.

Finish and verify the engine hooks in this order:

1. Validate the origin getter/setter implementation against the exact `BoxPokemon` layout and all relevant copy/zero operations.
2. Ensure every normal acquisition path relevant to the prototype receives an origin: wild capture, scripted/gift Pokémon, starters, daycare eggs/hatching.
3. Confirm evolution, leveling, healing, moves, held items, party↔PC movement, and daycare storage do not rewrite the origin bits.
4. Complete safe region-transition vaulting so an old-region party is deposited without loss before a fresh regional starter is given.
5. Complete PC enforcement for every path that can place an ineligible Pokémon into the party; summary viewing may remain allowed.
6. Run `git apply --check` for `patches/legacy_core.patch`.
7. Run the Emerald build and test suite through the prototype workflow.
8. Fix all compiler/test failures before adding substantial map content.

Only after the build is green should the vertical slice be scripted.

## First playable vertical-slice target

Use existing Emerald maps/assets as temporary prototype geography rather than spending time drawing full regions.

The first playable acceptance sequence is:

1. start in prototype Kanto
2. choose Bulbasaur / Charmander / Squirtle at Lv. 5
3. obtain Kanto-origin Pokémon
4. defeat a compact Kanto League / Champion sequence
5. mark Kanto Champion
6. transition to Johto
7. safely vault Kanto party members and lock them
8. choose Chikorita / Cyndaquil / Totodile at Lv. 5
9. obtain Johto-origin Pokémon
10. confirm Kanto Pokémon still exist but cannot enter the active party
11. reach a simulated Johto final-badge / League checkpoint
12. restore Kanto Legacy access
13. build a mixed Kanto + Johto party
14. fight a stronger Johto League encounter
15. save/reload at the major transition points and confirm persistence

## Definition of first milestone success

Do not call the vertical slice done merely because source code exists.

It is successful only after the branch has a verified build where:

- origin assignment works
- save persistence works
- Kanto→Johto transition works
- veterans are preserved but locked
- Johto begins with a fresh Lv. 5 starter
- the League checkpoint restores veterans
- Kanto + Johto mixed-party play works afterward
- save/reload preserves the state

## Resume instruction for a future ChatGPT session

Use this instruction:

> Continue Pokémon Legacy from `POKEMON_LEGACY_HANDOFF.md` in `DraftCave/pokeemerald-Legacy`. Inspect the current `legacy-system-prototype` branch and draft PR #1 first. Current repository state wins over this handoff if newer commits exist. Do not restart the architecture. Continue from the immediate next step, preserve `master`, and do not claim compilation/playability until verified by the actual build/tests.

## Updating this handoff

After every meaningful implementation milestone, update this file with:

- newest verified branch/PR state
- what is actually implemented
- exact build/test status
- known failures/blockers
- next concrete step

Never let this file claim more than the repository and tests prove.
