# Pokémon Legacy — Continuity Handoff

This file is the first thing a new ChatGPT session should read before continuing Pokémon Legacy work.

## Repository / active work

- Repository: `DraftCave/pokeemerald-Legacy`
- Base branch: `master` — preserve it; do not implement unfinished work directly here.
- Active development branch: `legacy-system-prototype`
- Active draft PR: `#1 — Prototype the three-generation Legacy System`
- Primary architecture doc: `docs/LEGACY_SYSTEM.md`
- Manual acceptance checklist: `docs/PROTOTYPE_PLAYTEST.md`
- Engine integration patch: `patches/legacy_core.patch`
- Safe-transition installer: `tools/apply_legacy_transition_guard.py`
- Deterministic playable-content installer: `tools/apply_legacy_prototype_content.py`
- Prototype CI workflow: `.github/workflows/legacy-prototype.yml`

Current repository state wins over this handoff if newer commits exist.

## Product goal

Pokémon Legacy is a GBA ROM hack built on `pokeemerald-expansion` with one continuous save spanning three campaigns:

1. Kanto — Gen 1 species pool, fresh Lv. 5 Kanto starter.
2. Johto — Gen 1–2 pool, previous Kanto Pokémon temporarily locked during regional progression; Kanto veterans return at the Johto League checkpoint.
3. Hoenn — Gen 1–3 pool, Kanto + Johto veterans temporarily locked; all previous veterans return at the Hoenn League checkpoint.
4. Postgame — all regional restrictions removed.

The defining mechanic is the **Legacy System**: eligibility follows the individual Pokémon's campaign of acquisition, not its species generation.

Example: a Pikachu caught in Johto is Johto-origin even though Pikachu is a Gen 1 species.

The current task is still the three-region Gen 1–3 base game. Future ideas such as Gen 4–9, a Heart & Soul-style rules/settings menu, day/night, and map-aware battle backgrounds are parked until the base is stable.

## Critical Pokémon-origin representation

The current prototype uses two otherwise-unused top-level `BoxPokemon` bits so the Pokémon/save structure does not grow.

Encoding:

- `0` = UNSET / compatibility fallback
- `1` = KANTO
- `2` = JOHTO
- `3` = HOENN

Do **not** change zero to mean Kanto. Zeroed/legacy data naturally starts at zero, so zero must remain UNSET.

Origin belongs to the individual Pokémon and must survive party/PC movement, evolution, leveling, healing, moves, held items, and save/reload.

## Implemented campaign state

`include/constants/legacy_system.h`, `include/legacy_system.h`, and `src/legacy_system.c` implement persistent campaign state using vanilla-unused persistent variables rather than expanding save blocks.

Implemented state includes:

- Legacy system version
- current campaign region
- regional badge counters
- regional champion bitmask
- legacy-access bitmask
- region transitions
- League unlock state
- postgame state
- per-Pokémon origin getter/setter and eligibility API
- safe veteran-vaulting framework

Core rule:

- Kanto campaign: Kanto-origin usable.
- Johto pre-League: Johto-origin usable; Kanto-origin locked.
- Johto League phase: Kanto + Johto usable.
- Hoenn pre-League: Hoenn usable; Kanto + Johto locked.
- Hoenn League/postgame: prior veterans restored as appropriate.

## Implemented/staged engine integration

`patches/legacy_core.patch` currently integrates the Legacy engine into upstream expansion files.

### Acquisition/origin assignment

Normal prototype acquisition paths assign the current campaign origin, including:

- wild captures
- scripted/gift Pokémon
- regional starters via `ScriptGiveMon`
- eggs/daycare path used by the prototype

The starter/gift path was mechanically checked: regional starters flow through the scripted-give path that assigns current-region origin rather than remaining UNSET.

### Regional starters

Starter selection is derived from current Legacy region:

- Kanto: Bulbasaur / Charmander / Squirtle
- Johto: Chikorita / Cyndaquil / Totodile
- Hoenn: Treecko / Torchic / Mudkip

### PC enforcement

The PC integration blocks ineligible Pokémon from entering the active party while preserving them in storage. The intended player message is:

`This POKéMON is Legacy-locked until the League.`

Summary viewing may remain available; locking is an eligibility restriction, not deletion or mutation.

### Safe transition guard

`src/legacy_transition_guard.c` plus `tools/apply_legacy_transition_guard.py` add the current safety preflight for Kanto → Johto prototype transition.

The transition checks that enough PC storage exists for the veterans that must be vaulted **before** the League/transition sequence can proceed. Insufficient capacity blocks the transition instead of risking deletion, partial movement, or an ineligible veteran being stranded in the party.

This must still be manually emulator-tested; compilation alone does not certify runtime UI/save behavior.

## Playable vertical-slice integration

The stale hand-authored `patches/legacy_prototype_content.patch` is no longer authoritative for the build. It drifted against upstream files and was separated from the build gate.

Playable content is now installed deterministically by:

`tools/apply_legacy_prototype_content.py`

The prototype deliberately repurposes existing Emerald maps/NPCs/trainers as mechanical scaffolding. Do not spend time recreating full regions until the loop is manually proven.

Current intended play sequence:

1. new save in prototype Kanto
2. Lv. 5 Bulbasaur / Charmander / Squirtle choice
3. Gen-1 prototype Route 101 encounters
4. compact Kanto League boss (RED)
5. capacity preflight
6. mark Kanto Champion
7. safely vault Kanto veterans
8. begin Johto and give Lv. 5 Chikorita / Cyndaquil / Totodile
9. Kanto veterans remain stored but PC withdrawal is blocked
10. Gen-1/2 prototype Route 102 encounters/trainers
11. Petalburg prototype checkpoint simulates Johto eighth badge / League access
12. Kanto veterans unlock
13. mixed Kanto + Johto party becomes possible
14. stronger Johto League opponent (LANCE)
15. record Johto Champion

`docs/PROTOTYPE_PLAYTEST.md` is the authoritative manual acceptance checklist for this slice.

## Exact verified build/test status at this checkpoint

Do not overstate this section.

### Verified

- Core Legacy patch applies in GitHub Actions.
- Transition-guard installer applies in GitHub Actions.
- Deterministic playable-content installer applies in GitHub Actions.
- The integrated playable vertical slice has completed the **Emerald compile successfully** in the focused `Legacy Prototype` workflow.
- An earlier engine-only checkpoint completed the expansion suite successfully.

### In progress / not yet certified

- The current full playable-slice workflow run had reached `make check` / **Run expansion tests** and was still in progress at the time this handoff was updated. Do not call the current full-content suite green until a later workflow result explicitly says success.
- Actual emulator playthrough has **not** yet certified the whole sequence.
- Save/reload across Kanto → Johto, PC UI behavior, transition blocking, Legacy unlock, and mixed-party behavior still require manual emulator acceptance.
- Link/trade compatibility of the two origin bits is not certified.

## CI integration path

From a clean checkout of `legacy-system-prototype`, the focused workflow performs the equivalent of:

```sh
git apply --recount patches/legacy_core.patch
python3 tools/apply_legacy_transition_guard.py
python3 tools/apply_legacy_prototype_content.py
make -j$(nproc) -O all
make -j$(nproc) check
```

The workflow does not publish or commit a copyrighted commercial ROM.

## Immediate next step — DO THIS FIRST

1. Inspect the newest `Legacy Prototype` GitHub Actions run on the current branch head.
2. If expansion tests are red, fetch the failing job log and fix the concrete failure before adding features.
3. If expansion tests are green, record the exact green SHA/run in this handoff.
4. Then move to **manual emulator acceptance** using `docs/PROTOTYPE_PLAYTEST.md`.
5. Fix any runtime/save/UI issue found by the acceptance playthrough.
6. Only after the vertical slice passes manual save/reload acceptance should full Kanto map/story/content production begin.

Do not start day/night, battle-background polish, settings/randomizer work, Gen 4–9 expansion, or full Hoenn production before this gate.

## Manual vertical-slice pass criteria

The first playable milestone is accepted only when an emulator completes:

**Kanto starter → Kanto catches → save/reload → Kanto boss → safe vault → Johto Lv. 5 starter → save/reload → Kanto withdrawal blocked → Johto catches → League checkpoint → Kanto veterans restored → mixed party → Johto finale → final save/reload with roster intact.**

At every transition verify:

- no Pokémon disappears
- no Pokémon is duplicated
- origin remains stable
- level/moves/item/nickname/OT/stats remain intact
- insufficient PC capacity blocks safely
- current region/champion/badge/access state survives save/reload

## Future wishlist — parked, not current work

Once the three-region base is stable, user wants to explore:

- Heart & Soul-style Recommended/Custom game-rules menu
- settings that distinguish change-anytime vs save-locked rules
- day/night cycle with RTC/fake-RTC support
- battle backgrounds reflecting the actual route/city/area, later with day/night variants
- eventual Gen 4–9 species expansion and potentially broader campaign content

These are not current blockers and must not derail base-game completion.

## Resume instruction for a future ChatGPT session

Use:

> Continue Pokémon Legacy from `POKEMON_LEGACY_HANDOFF.md` in `DraftCave/pokeemerald-Legacy`. Inspect the current `legacy-system-prototype` branch, draft PR #1, and newest `Legacy Prototype` workflow run first. Current repository state wins over the handoff. Preserve `master`, do not restart the architecture, and do not claim runtime playability until the emulator acceptance checklist has passed.

## Updating this handoff

After every meaningful implementation milestone, update this file with:

- newest verified branch/PR state
- what is actually implemented
- exact build/test status
- known failures/blockers
- next concrete step

Never let this file claim more than the repository, CI, and manual tests prove.
