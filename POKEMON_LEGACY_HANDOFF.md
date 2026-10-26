# Pokémon Legacy — Continuity Handoff

Read this file first in any new ChatGPT session before continuing Pokémon Legacy.

## Repository / active work

- Repository: `DraftCave/pokeemerald-Legacy`
- Base branch: `master` — preserve it.
- Active development branch: `legacy-system-prototype`
- Draft PR: `#1 — Prototype the three-generation Legacy System`
- Three-region promotion merge: `44a2bd436dff18c1dcc9e30ea99608a8a4ec15a5`
- Architecture: `docs/LEGACY_SYSTEM.md`
- Manual acceptance checklist: `docs/PROTOTYPE_PLAYTEST.md`
- Engine integration: `patches/legacy_core.patch`
- Kanto/Johto content installer: `tools/apply_legacy_prototype_content.py`
- Transition-capacity installer: `tools/apply_legacy_transition_guard.py`
- Hoenn prototype installer: `tools/apply_legacy_hoenn_prototype.py`
- Terminal-state routing installer: `tools/apply_legacy_terminal_routing.py`
- One-command local builder: `tools/build_legacy_prototype.sh`
- CI: `.github/workflows/legacy-prototype.yml`

Current repository state always wins over this handoff if newer commits exist.

## Product goal

Pokémon Legacy is one continuous GBA save spanning Kanto → Johto → Hoenn.

- Kanto: fresh Lv.5 Bulbasaur / Charmander / Squirtle; Gen-1 pool.
- Johto: Kanto veterans temporarily locked; fresh Lv.5 Chikorita / Cyndaquil / Totodile; Gen-1/2 pool; Kanto returns at the Johto League checkpoint.
- Hoenn: Kanto + Johto veterans temporarily locked again; fresh Lv.5 Treecko / Torchic / Mudkip; Gen-1/2/3 pool; previous veterans return at the Hoenn League checkpoint.
- Postgame: all regional eligibility restrictions removed.

The defining rule is individual-Pokémon origin. Species generation does not determine eligibility. A Pikachu caught in Johto is Johto-origin.

Future wishlist is parked until the base is stable: day/night, contextual battle backgrounds, Heart & Soul-style settings, randomizer/challenge modes, and eventual Gen 4–9 support.

## Persistent representation

Campaign state uses unused saved event variables rather than expanding save blocks. Current state includes current region, badge counts, Champion mask, Legacy access mask, and state version.

Per-Pokémon origin uses two currently-unused top-level `BoxPokemon` bits:

- `0` = UNSET / UNKNOWN
- `1` = KANTO
- `2` = JOHTO
- `3` = HOENN

Zero must remain UNSET. Unknown origin fails closed during regional campaigns.

This two-bit representation is intentionally scoped to the three-region base. Redesign it before any future Gen-4–9 region-origin expansion is declared save-format stable.

## Core engine implemented

`include/legacy_system.h`, `include/constants/legacy_system.h`, `src/legacy_system.c`, `src/legacy_transition_guard.c`, and `patches/legacy_core.patch` implement or integrate:

- campaign initialization/reset;
- region transitions;
- badge, Champion, and access masks;
- postgame state;
- per-Pokémon origin getter/setter;
- automatic current-region origin assignment for prototype acquisition paths;
- central eligibility API;
- PC withdrawal / placement blocking for ineligible Pokémon;
- safe veteran vaulting on region change;
- regional starter selection;
- Kanto → Johto → Hoenn prototype script helpers;
- PC-capacity preflight so a transition refuses to begin if veterans cannot be safely stored.

Locked Pokémon remain owned and preserved. They are not deleted, level-reset, or mutated. Summary viewing may remain available.

The normal starter/gift path flows through `ScriptGiveMon`, which is patched to assign current-region origin.

`gPokemonStoragePtr` was traced in `src/load_save.c`: normal `SetSaveBlocksPointers()` initialization assigns it. The capacity guard is therefore not dependent on opening the PC UI first.

## Current temporary playable geography

This is mechanical scaffolding only, not final Kanto/Johto/Hoenn map design.

### Kanto shell

- Emerald opening / Route 101 reused.
- Starter choices: Bulbasaur / Charmander / Squirtle.
- Route 101 temporarily uses a Gen-1 encounter set.
- Oldale girl NPC is the compact Kanto League hook.
- Temporary boss: RED — Rattata 6 / Pidgey 7 / Pikachu 8.

After RED, storage capacity is preflighted, Kanto Champion is recorded, veterans are safely vaulted, Johto begins, and the player chooses a fresh Lv.5 Johto starter.

### Johto shell

- Route 102 is temporary Johto progression.
- Gen-1/2 encounter set and disposable Johto trainer replacements are installed.
- Petalburg boy NPC is the League checkpoint/finale hook.
- First Johto checkpoint restores Kanto veterans.
- Temporary finale: LANCE — Pidgeotto 11 / Flaaffy 12 / Quagsire 12 / Heracross 13 / Espeon 13 / Typhlosion 14.

After LANCE, Johto Champion is recorded. If enough PC space exists, the mixed Kanto+Johto active party is safely vaulted and Hoenn begins with Treecko / Torchic / Mudkip at Lv.5. If storage is insufficient, the transition is blocked and can be retried after freeing room.

### Hoenn shell

- Existing Emerald geography north from Petalburg is reused because it is already Gen-1–3-compatible scaffolding.
- During fresh Hoenn progression, Kanto + Johto veterans are Legacy-locked.
- Rustboro fat-man NPC is the temporary Hoenn League checkpoint/finale.
- First Hoenn checkpoint simulates eight badges and restores completed Kanto + Johto access.
- Player can then build a three-origin Kanto + Johto + Hoenn party.
- Temporary final boss: WALLACE using `TRAINER_MARCEL` — Swellow 14 / Breloom 15 / Manectric 15 / Flygon 16 / Milotic 17 / Metagross 18.
- Defeating WALLACE records Hoenn Champion and enters postgame, removing regional restrictions.

Terminal-state routing was added so completed Hoenn/postgame saves cannot accidentally reopen the early RED prototype fight.

## Automated tests

`test/legacy_system.c` covers:

- origin assignment and copy persistence;
- unknown-origin fail-closed behavior;
- Kanto locking on entry to Johto;
- Johto-origin eligibility;
- Kanto return at Johto League access;
- Kanto + Johto locking in Hoenn;
- Kanto + Johto restoration at Hoenn League access;
- Hoenn completion entering postgame and restoring all three origins.

The test runner supports exact filename filtering. Current prototype CI uses `TESTS=legacy_system.c` for fast iteration after a complete Emerald build. Periodic full expansion regression should still be used after meaningful engine changes.

## Verified CI history

Do not overstate runtime playability.

### Fully verified green checkpoints

1. Core engine checkpoint — run `40269983441`, job `121400488525`:
   - core patch applies ✅
   - Emerald build ✅
   - full expansion test suite ✅

2. Integrated Kanto→Johto content checkpoint — run `40271518222`, job `121405156368`:
   - core patch ✅
   - deterministic playable content installer ✅
   - Emerald build ✅
   - full expansion test suite ✅

### Current three-region checkpoint

The full Kanto→Johto→Hoenn implementation was promoted to `legacy-system-prototype` at merge commit `44a2bd436dff18c1dcc9e30ea99608a8a4ec15a5` and is awaiting/running its dedicated focused validation workflow.

At the moment this handoff was written, do **not** claim that the promoted three-region commit is green until the newest `Legacy Prototype` workflow explicitly completes successfully.

A superseded safety-guard run (`40272064482`) may still be consuming the GitHub runner with the older full expansion suite. The connected GitHub tool did not expose a cancel action. Let current repository/workflow status win.

## Build locally

From `legacy-system-prototype`:

```sh
bash tools/build_legacy_prototype.sh
```

The helper:

1. applies the core integration if needed;
2. installs Kanto/Johto content;
3. installs transition guards;
4. installs Hoenn content;
5. installs terminal-state routing;
6. runs `git diff --check`;
7. builds Emerald.

Expected local output is `pokeemerald.gba`. Do not commit or distribute copyrighted commercial ROM files.

## Manual emulator acceptance — required next milestone

CI compilation/tests do not certify the actual save/UI journey. Use `docs/PROTOTYPE_PLAYTEST.md`.

The prototype is not accepted until an emulator successfully proves:

**Kanto Lv.5 starter → Kanto catches → RED → safe vault → Johto Lv.5 starter → Kanto withdrawal blocked → Johto catches → Johto checkpoint → Kanto restored → mixed team → LANCE → safe vault → Hoenn Lv.5 starter → Kanto+Johto blocked → Hoenn catch → Hoenn checkpoint → all veterans restored → three-origin team → WALLACE → postgame → save/reload remains correct.**

Also explicitly test insufficient PC capacity at both regional transitions. A failed preflight must not partially move Pokémon or alter campaign state.

## Known prototype limitations

- Geography, NPCs, trainer IDs, dialogue, and level curves are disposable scaffolding.
- Route 102 is not physically gated like final regional progression will be.
- Existing Emerald story/map gates can still interfere with the temporary travel shell; manual playtest will expose those issues.
- Link/trade compatibility of the two origin bits is not certified.
- Contextual battle backgrounds, day/night, settings, and final region art/story are not implemented yet.
- This is a mechanical proof, not the finished ROM.

## Immediate resume instruction

> Continue Pokémon Legacy from `POKEMON_LEGACY_HANDOFF.md` in `DraftCave/pokeemerald-Legacy`. Inspect `legacy-system-prototype`, draft PR #1, and the newest `Legacy Prototype` GitHub Actions run first. Current repository state wins. If the three-region workflow is red, fix that exact failure. If it is green, record the exact SHA/run here and move to manual emulator acceptance from `docs/PROTOTYPE_PLAYTEST.md`. Preserve `master` and do not start day/night, battle-background polish, settings, or Gen 4–9 work before the three-region base passes runtime acceptance.
