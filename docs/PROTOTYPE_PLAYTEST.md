# Pokémon Legacy — Vertical Slice Playtest

This checklist is for the temporary Kanto → Johto mechanical prototype on `legacy-system-prototype`.

The prototype deliberately reuses Emerald geography. It is not representative of the final region maps, story, trainer balance, or art direction. Its purpose is to prove that the Legacy System works in a real save.

## Build

From a clean checkout of `legacy-system-prototype`:

```sh
git apply --recount patches/legacy_core.patch
python3 tools/apply_legacy_transition_guard.py
python3 tools/apply_legacy_prototype_content.py
make -j$(nproc) -O all
make -j$(nproc) check
```

The GitHub Actions workflow `.github/workflows/legacy-prototype.yml` runs this same integration path.

Do not treat a successful CI build as a substitute for emulator playtesting of save/reload, PC UI behavior, and map/script progression.

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
2. If PC storage does not have enough free slots for the active Kanto veterans that must be vaulted, confirm the transition is blocked before the League battle begins and no Pokémon is moved or deleted.
3. With enough PC capacity, defeat the temporary Kanto League boss (RED).
4. Confirm the game announces Kanto completion.
5. Confirm the previous active Kanto team is moved safely into PC storage.
6. Confirm the Johto starter selector opens.
7. Confirm the choices are **Chikorita / Cyndaquil / Totodile**.
8. Choose one and confirm it is Lv. 5.
9. Save immediately, reset/reload, and continue.

Expected result: the Kanto Pokémon still exist unchanged in storage and the active journey is now Johto.

### 3. Legacy lock verification

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

### 5. Johto League checkpoint / Legacy return

1. Reach Petalburg City.
2. Speak to the boy NPC used by the prototype.
3. On the first Johto-phase interaction, confirm the prototype simulates the eighth Johto badge and restores Legacy access.
4. Return to a PC.
5. Withdraw at least one Kanto veteran.
6. Build a mixed Kanto + Johto party.
7. Save/reload with the mixed party and confirm both origins remain usable.

Expected result: completed Kanto veterans are available again without any change to their level, moves, held item, or identity.

### 6. Johto finale

1. Speak to the Petalburg prototype NPC again.
2. Defeat the stronger temporary Johto League opponent (LANCE).
3. Confirm Johto Champion completion is recorded.
4. Save/reload.
5. Confirm the completed-state dialogue appears and the mixed roster remains intact.

## Destructive-safety checks

At every transition, specifically verify:

- no Pokémon disappears;
- no Pokémon is duplicated;
- insufficient PC capacity blocks the transition before destructive movement begins;
- origin eligibility does not change after evolution;
- level, moves, held item, nickname, OT data, and stats remain intact through locking/unlocking;
- save/reload does not reset current region, Champion state, badge state, or Legacy access;
- PC movement does not rewrite origin.

## Known prototype limitations

- Emerald geography is temporary scaffolding, not Kanto/Johto map recreation.
- Route 102 is not yet physically gated behind the Kanto boss, so a player deliberately walking there early can see Johto-style prototype content before the intended transition. Final regions must enforce progression spatially/story-wise.
- The prototype currently uses existing trainer IDs and NPCs as disposable test hooks.
- Transition capacity is safety-guarded, but the final game still needs a polished player-facing Legacy Vault/transition UX rather than prototype blocking behavior.
- Link/trade compatibility of the two origin bits is not yet certified.
- CI can prove compilation and automated tests, but emulator playtesting is still required for UI flow and actual save/reload behavior.

## Pass criteria

The vertical slice is accepted only when the complete sequence works in an emulator:

**Kanto starter → Kanto catches → Kanto boss → safe vault → Johto Lv. 5 starter → Kanto withdrawal blocked → Johto catches → League checkpoint → Kanto veterans restored → mixed party → Johto finale → save/reload remains correct.**
