# Pokémon Legacy — Campaign / Legacy System

## Core loop

Pokémon Legacy is one continuous save spanning three regional campaigns:

1. **Kanto** — Gen 1 species pool. Start with a level 5 Kanto starter and build a fresh team.
2. **Johto** — Gen 1–2 species pool. Kanto Pokémon are locked during the eight-badge journey. At the Johto League checkpoint, completed Kanto veterans become usable again.
3. **Hoenn** — Gen 1–3 species pool. Kanto and Johto Pokémon are locked during the eight-badge journey. At the Hoenn League checkpoint, completed Kanto and Johto veterans become usable again.
4. **Postgame** — all regional restrictions are removed.

The player never loses old Pokémon. A region transition changes eligibility, not ownership, level, moves, stats, or identity.

## Intended difficulty curve

The regional gym journeys use conventional progression and begin from a fresh level 5 starter. The Leagues escalate because previous-region veterans return before the League.

Long-term target ranges are design goals, not hard-coded values yet:

- Kanto League: approximately level 55–62.
- Johto League: approximately level 60–75.
- Hoenn League finale: approximately level 80–100, with dedicated late-game training content between Badge 8 and the League.

The Hoenn finale should require meaningful preparation rather than ordinary route grinding. Planned training content includes high-level areas, repeatable trainer divisions, and rematches.

## Persistent campaign state

The prototype uses vanilla-unused persistent event vars instead of changing save-block layouts:

- state version
- current campaign region
- legacy access bitmask
- champion-region bitmask
- Kanto badge count
- Johto badge count
- Hoenn badge count

`LegacySystem_Reset()` initializes a new save to Kanto. `NewGameInitData()` is patched to call it immediately after event data is initialized. Existing saves with an unknown Legacy state version are initialized conservatively the first time the Legacy API is accessed.

## Individual Pokémon origin

The lock follows the **individual Pokémon's campaign of origin**, not the species generation.

Example: a Pikachu caught during Johto is Johto-origin even though Pikachu is a Gen 1 species.

The prototype stores origin in the two top-level `BoxPokemon` bits currently named `unused_13` and `unused_1E`. Repository inspection found no current expansion references to either bit outside the struct definition. Using these two bits does not change `sizeof(struct BoxPokemon)` and does not modify the encrypted secure substructures or their checksum.

Implemented encoding:

- `0` — UNSET / UNKNOWN
- `1` — KANTO
- `2` — JOHTO
- `3` — HOENN

Zero intentionally means **UNSET**. Old saves and freshly zeroed Pokémon naturally contain zero in these bits, so encoding Kanto as zero would silently misclassify unknown Pokémon. Unknown-origin Pokémon fail closed during regional campaigns.

Normal acquisition hooks currently staged by `patches/legacy_core.patch` assign the current campaign origin to:

- wild captures
- scripted gifts, including starters
- daycare eggs when the egg is created

Evolution, leveling, healing, held-item changes, move changes, and party/PC movement preserve origin automatically because the bits travel inside the individual `BoxPokemon` data.

Trading/link compatibility is not yet certified and must be tested before the two bits are considered permanently reserved.

## Eligibility rules

The authoritative eligibility API lives in `src/legacy_system.c`.

### Kanto

- Kanto-origin Pokémon usable.
- Unknown-origin Pokémon fail closed.

### Johto before League unlock

- Johto-origin Pokémon usable.
- Kanto-origin Pokémon locked.

### Johto League phase

- Johto-origin Pokémon usable.
- Kanto-origin Pokémon usable if Kanto was completed.

### Hoenn before League unlock

- Hoenn-origin Pokémon usable.
- Kanto- and Johto-origin Pokémon locked.

### Hoenn League phase

- Hoenn-origin Pokémon usable.
- completed Kanto and Johto regions become accessible again.

### Postgame

- all origins usable.

## Safe transition behavior

`LegacySystem_BeginRegion()` clears previous-region access and calls `LegacySystem_StoreLockedPartyMons()`.

For every now-ineligible party member:

1. attempt to copy the Pokémon to PC storage;
2. erase the party copy **only if the PC copy succeeds**;
3. compact the remaining party using exported Pokémon APIs.

This prevents a region transition from deleting a veteran when storage fails.

Known edge case for the prototype: if PC storage is completely full, an ineligible veteran remains in the active party rather than being destroyed. A full-game implementation must add a transition blocker or dedicated vault capacity before shipping.

## PC enforcement

`patches/legacy_core.patch` integrates the central eligibility check into the Pokémon Storage System.

A locked Pokémon:

- remains visible in storage;
- can still be inspected in the PC;
- cannot be withdrawn into the party;
- cannot be placed or shifted into a party slot while being moved;
- receives a clear `Legacy-locked until the League` message.

No lock operation changes the Pokémon itself beyond its persistent origin tag.

## Regional starters

The normal starter selector is made campaign-aware:

- Kanto: Bulbasaur / Charmander / Squirtle
- Johto: Chikorita / Cyndaquil / Totodile
- Hoenn: Treecko / Torchic / Mudkip

The initial Emerald starter event therefore becomes the Kanto starter event. The prototype also provides `ChooseLegacyStarter`, which reuses the same selector after the Kanto-to-Johto transition and returns to the overworld without replaying Emerald's forced first battle.

## Playable vertical-slice shell

`patches/legacy_content.patch` intentionally reuses existing Emerald geography for the first mechanical proof instead of spending the first build on custom maps.

Temporary prototype mapping:

- **Littleroot / Route 101** — Kanto opening shell.
- **Route 101** — Gen 1 wild encounter table.
- **Oldale Town girl NPC** — compact Kanto League boss and Kanto → Johto transition.
- **Route 102** — Johto route shell with Gen 1–2 encounters and four rebuilt Johto trainers.
- **Petalburg boy NPC** — Johto League checkpoint and final Johto Champion battle.

The Kanto boss uses a short Gen 1 roster. After victory, Kanto completion is recorded, Kanto party members are vaulted, and the player chooses a fresh level 5 Johto starter.

The first Petalburg checkpoint interaction simulates the final Johto badge for this compressed slice and restores Kanto Legacy access. The player can then visit a PC, build a mixed Kanto + Johto party, and return for a stronger six-Pokémon Johto finale.

This geography is disposable. The Legacy engine is intended to survive replacement with full Kanto, Johto, and Hoenn maps later.

## Tests

`test/legacy_system.c` covers:

- origin assignment and copy persistence;
- unknown-origin fail-closed behavior;
- Kanto lock on entering Johto;
- Kanto unlock at the Johto League checkpoint;
- Johto-origin eligibility during Johto;
- Kanto + Johto unlock at the Hoenn League checkpoint.

The dedicated workflow `.github/workflows/legacy-prototype.yml` applies both staged patches, builds Emerald, and runs the expansion test suite.

## Current verification status

Source implementation is committed on `legacy-system-prototype` and Draft PR #1.

A successful compile/test run has **not yet been observed**. GitHub currently reports zero Actions runs for the fork, consistent with Actions not yet being enabled for the newly forked repository. A separate local clone/build attempt from the assistant execution environment was blocked by that environment's outbound-network sandbox.

Therefore no document or PR should claim the prototype is compiled or play-tested until CI is enabled or a local build completes.

## Acceptance test after the first successful build

1. Start a new game and choose a Kanto starter.
2. Catch at least one Route 101 Pokémon.
3. Defeat the Oldale Kanto boss.
4. Confirm Kanto Pokémon are preserved in PC storage and cannot be withdrawn.
5. Choose a level 5 Johto starter.
6. Catch a Route 102 Pokémon and defeat the Johto route trainers.
7. Save/reload and confirm Kanto remains locked while Johto Pokémon remain usable.
8. Reach Petalburg and trigger the League checkpoint.
9. Confirm Kanto veterans can be withdrawn again with unchanged identity/data.
10. Build a mixed Kanto + Johto party and defeat the Johto finale.
11. Save/reload again and confirm both championship/progression state and Pokémon origins persist.

Only after this loop passes should full-region map production become the priority.
