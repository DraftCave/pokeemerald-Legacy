# Pokémon Legacy — Three-Region Prototype Playtest

This checklist is for the temporary Kanto → Johto → Hoenn mechanical prototype on `legacy-system-prototype`.

The prototype deliberately reuses Emerald geography. It is not representative of the final region maps, story, trainer balance, or art direction. Its purpose is to prove the complete Legacy loop in one real save before full-region content production begins.

## Build

From a checkout of `legacy-system-prototype`, the preferred local command is:

```sh
bash tools/build_legacy_prototype.sh
```

The helper safely detects whether the core patch is already applied, installs the deterministic Kanto/Johto content, transition-capacity guard, Hoenn prototype content, and terminal-state routing, then builds `pokeemerald.gba`.

Equivalent manual sequence:

```sh
git apply --recount patches/legacy_core.patch
python3 tools/apply_legacy_prototype_content.py
python3 tools/apply_legacy_transition_guard.py
python3 tools/apply_legacy_hoenn_prototype.py
python3 tools/apply_legacy_terminal_routing.py
make -j$(nproc)
```

The GitHub Actions workflow `.github/workflows/legacy-prototype.yml` performs the same integration path, builds Emerald, and runs the focused `legacy_system.c` automated tests. A complete expansion regression suite has already passed for the Legacy core; periodic full-suite regression remains appropriate after meaningful engine changes.

Do not treat a successful CI build as a substitute for emulator playtesting of save/reload, PC UI behavior, and the actual scripted journey.

## Acceptance playthrough

### 1. New game / prototype Kanto

1. Start a new save.
2. Reach Birch's starter selection as normal.
3. Confirm the three choices are **Bulbasaur / Charmander / Squirtle**.
4. Choose one and confirm it is Lv. 5.
5. Save and reload once.
6. Catch at least one Pokémon on Route 101.
7. Confirm Route 101 is using the temporary Gen-1 encounter pool.
8. Visit a PC and verify normal party ↔ PC movement works for currently eligible Kanto Pokémon.

Expected result: new Pokémon obtained during this phase are Kanto-origin and usable.

### 2. Compact Kanto League / transition

1. In Oldale Town, speak to the girl NPC used by the prototype.
2. Defeat the temporary Kanto League boss (**RED**).
3. Confirm the game announces Kanto completion.
4. Confirm the previous active Kanto team is moved safely into PC storage.
5. Confirm the Johto starter selector opens.
6. Confirm the choices are **Chikorita / Cyndaquil / Totodile**.
7. Choose one and confirm it is Lv. 5.
8. Save immediately, reset/reload, and continue.

Expected result: the Kanto Pokémon still exist unchanged in storage and the active journey is now Johto. Their stored move PP must match the pre-transition values; Legacy vaulting must not heal or otherwise rewrite them.

### 3. Kanto Legacy lock verification

1. Open a PC before the Johto League checkpoint.
2. View a stored Kanto Pokémon's summary if desired.
3. Attempt to withdraw that Kanto Pokémon.
4. Confirm withdrawal is rejected with the Legacy-lock message.
5. Confirm the Pokémon remains safely in storage.
6. Confirm the Johto starter and Pokémon obtained during Johto remain usable.

Expected message:

> This POKéMON is Legacy-locked until the League.

### 4. Prototype Johto journey

1. Travel west through Route 102.
2. Confirm the temporary encounter pool contains Gen-1/2 Pokémon rather than Hoenn-route defaults.
3. Battle the temporary Johto trainer replacements.
4. Catch at least one Johto-journey Pokémon.
5. Save/reload and confirm it remains usable.

### 5. Johto League checkpoint / Kanto return

1. Reach Petalburg City.
2. Speak to the boy NPC used by the prototype.
3. On the first Johto-phase interaction, confirm the prototype simulates the eighth Johto badge and restores Kanto Legacy access.
4. Return to a PC.
5. Withdraw at least one Kanto veteran.
6. Build a mixed Kanto + Johto party.
7. Save/reload with the mixed party and confirm both origins remain usable.

Expected result: completed Kanto veterans are available again without any change to level, moves, move PP, held item, nickname, OT data, or identity.

### 6. Johto finale / transition to Hoenn

1. Speak to the Petalburg prototype NPC again.
2. Defeat the stronger temporary Johto League opponent (**LANCE**).
3. Confirm Johto Champion completion is recorded.
4. Confirm the game begins the Hoenn transition only when enough PC slots exist to preserve the active mixed party.
5. Confirm the Kanto + Johto party is safely vaulted and both prior origins become Legacy-locked again.
6. Confirm the starter selector now offers **Treecko / Torchic / Mudkip**.
7. Choose one and confirm it is Lv. 5.
8. Save/reload immediately.

Expected result: the active campaign is Hoenn, old veterans still exist in storage unchanged, and only Hoenn-origin Pokémon are currently eligible.

### 7. Hoenn Legacy lock verification

1. Open a PC before the Hoenn League checkpoint.
2. Attempt to withdraw one Kanto-origin Pokémon and one Johto-origin Pokémon.
3. Confirm both withdrawals are rejected.
4. Confirm the Hoenn starter remains usable.
5. Obtain at least one additional Pokémon during the Hoenn phase and confirm it remains usable after save/reload.

### 8. Hoenn League checkpoint / full Legacy return

1. Travel north through the temporary Emerald geography toward Rustboro City.
2. Speak to the fat-man NPC repurposed by the prototype.
3. Confirm the first Hoenn-phase interaction simulates eight Hoenn badges and restores completed Kanto + Johto Legacy access.
4. Visit a PC.
5. Build a party containing at least one Kanto-origin, one Johto-origin, and one Hoenn-origin Pokémon.
6. Save/reload with the three-origin party.
7. Confirm every member remains eligible.

Expected result: all three campaign origins can coexist for the Hoenn League phase.

### 9. Hoenn finale / postgame

1. Speak to the Rustboro prototype NPC again.
2. Defeat the temporary final opponent (**WALLACE**).
3. Confirm Hoenn Champion completion is recorded.
4. Confirm the campaign enters postgame.
5. Save/reload.
6. Confirm all regional restrictions remain removed and the entire collection is available.
7. Return to the earlier prototype NPCs in Oldale and Petalburg and confirm they show terminal/postgame dialogue rather than reopening old League transitions.

## PC-capacity transition test

Perform this test at least once before accepting the prototype:

1. Before a region-ending boss/transition, fill PC storage until fewer free slots remain than the number of active party Pokémon that must be vaulted.
2. Attempt the transition.
3. Confirm the game refuses to begin the transition and tells the player to free PC storage.
4. Confirm no Pokémon is moved, deleted, duplicated, healed, or otherwise changed.
5. Free enough PC slots.
6. Retry and confirm the transition proceeds normally.

This guard is currently wired for both Kanto → Johto and Johto → Hoenn. The guard counts all free PC slots, matching the storage search behavior used by the Legacy vault.

## Destructive-safety checks

At every transition, specifically verify:

- no Pokémon disappears;
- no Pokémon is duplicated;
- origin eligibility does not change after evolution;
- level, moves, move PP, held item, nickname, OT data, and persistent stats remain intact through locking/unlocking;
- save/reload does not reset current region, Champion state, badge state, or Legacy access;
- PC movement does not rewrite origin;
- a failed storage-capacity preflight does not partially transition campaign state;
- Legacy vaulting does not use the normal `CopyMonToPC()` PP-healing behavior.

## Known prototype limitations

- Emerald geography is temporary scaffolding, not Kanto/Johto/Hoenn map recreation.
- Route 102 is not yet physically gated behind the Kanto boss, so a player deliberately walking there early can see Johto-style prototype content before the intended transition. Full regions must enforce progression spatially/story-wise.
- The prototype currently uses existing trainer IDs and NPCs as disposable test hooks.
- The Hoenn shell intentionally reuses the normal Emerald path from Petalburg toward Rustboro instead of implementing Hoenn's final region design.
- Link/trade compatibility of the two origin bits is not yet certified.
- CI can prove compilation and automated state-machine tests, but emulator playtesting is still required for UI flow and actual save/reload behavior.
- The current 2-bit origin encoding is intentionally scoped to Kanto/Johto/Hoenn. It must be redesigned before a future Gen-4–9 region-origin expansion is declared save-format stable.

## Pass criteria

The prototype is accepted only when the complete sequence works in an emulator:

**Kanto Lv.5 starter → Kanto catches → RED → safe vault → Johto Lv.5 starter → Kanto locked → Johto catches → Johto checkpoint → Kanto veterans restored → mixed team → LANCE → safe vault → Hoenn Lv.5 starter → Kanto+Johto locked → Hoenn catches → Hoenn checkpoint → all veterans restored → three-origin team → WALLACE → postgame → terminal routing → save/reload remains correct.**
