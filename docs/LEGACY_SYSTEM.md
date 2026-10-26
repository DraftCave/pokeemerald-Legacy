# Pokémon Legacy — Campaign / Legacy System

## Core loop

Pokémon Legacy is one continuous save spanning three regional campaigns:

1. **Kanto** — Gen 1 species pool. Start with a level 5 Kanto starter and build a fresh team.
2. **Johto** — Gen 1–2 species pool. Kanto Pokémon are locked during the eight-badge journey. After the Johto League checkpoint, prior Kanto champions become usable again.
3. **Hoenn** — Gen 1–3 species pool. Kanto and Johto Pokémon are locked during the eight-badge journey. After the Hoenn League checkpoint, prior Kanto and Johto champions become usable again.
4. **Postgame** — all regional restrictions are removed.

The player never loses old Pokémon. A region transition changes eligibility, not ownership or levels.

## Intended difficulty curve

The regional gym journeys use conventional progression and begin from a fresh level 5 starter. The Leagues escalate much harder than the routes and gyms because previous-region champions return before the League.

Target ranges are design goals, not hard-coded values yet:

- Kanto League: approximately level 55–62.
- Johto League: approximately level 60–75.
- Hoenn League finale: approximately level 80–100, with dedicated late-game training content between Badge 8 and the League.

The Hoenn finale should require meaningful preparation rather than ordinary route grinding. Planned training content includes high-level areas, repeatable trainer divisions, and rematches.

## Persistent state

The prototype uses vanilla-unused persistent vars instead of changing save-block layouts:

- state version
- current campaign region
- legacy access bitmask
- champion-region bitmask
- Kanto badge count
- Johto badge count
- Hoenn badge count

This is deliberately conservative while the architecture is being validated.

## Region transition rules

### New Kanto save

- current region = Kanto
- Kanto badges = 0
- no previous-region access exists

### Kanto Champion -> Johto

1. Mark Kanto as a completed champion region.
2. Begin Johto.
3. Clear legacy access.
4. Choose a level 5 Johto starter.
5. Kanto Pokémon remain owned but unavailable during Johto's badge journey.

### Johto League checkpoint

- Restore access to completed Kanto champion Pokémon.
- Player may build a League roster from new Johto Pokémon plus returning Kanto veterans.

### Johto Champion -> Hoenn

1. Mark Johto as a completed champion region.
2. Begin Hoenn.
3. Clear legacy access again.
4. Choose a level 5 Hoenn starter.
5. Kanto and Johto veterans remain locked during Hoenn's badge journey.

### Hoenn League checkpoint

- Restore access to completed Kanto and Johto champion regions.
- Begin the final training phase.
- Hoenn Elite Four / Champion is intended to be the hardest main-story challenge.

### Postgame

- All Kanto, Johto, and Hoenn Pokémon become usable.

## Individual Pokémon origin

The lock must follow the **individual Pokémon's campaign of origin**, not the species generation.

Example: a Pikachu caught during Johto is a Johto-origin Pokémon even though Pikachu is a Gen 1 species.

The current expansion's `BoxPokemon` structure contains two top-level bits marked unused (`unused_13` and `unused_1E`). Together they can represent four origin states without changing `sizeof(struct BoxPokemon)` or the encrypted secure substructures. Before adopting them permanently, the prototype must verify that expansion features and transfer/link functionality do not depend on either bit.

Planned origin values:

- `0` — Kanto
- `1` — Johto
- `2` — Hoenn
- `3` — unset / compatibility fallback

Origin assignment must eventually cover wild catches, gifts, eggs, starters, trades, scripted Pokémon, and any debug/test creation path relevant to normal gameplay.

## Prototype acceptance test

The first playable vertical slice is successful when a single save can complete this sequence:

1. Start Kanto test chapter.
2. Obtain and level several Kanto Pokémon.
3. Defeat a test Kanto Champion.
4. Transition to Johto.
5. Confirm Kanto Pokémon still exist but cannot be used.
6. Receive a level 5 Johto starter.
7. Catch a Johto-origin Pokémon.
8. Reach the Johto League checkpoint.
9. Confirm Kanto veterans become usable again while preserving level, moves, stats, held items, ribbons, and identity.
10. Save, reload, and confirm the same state persists.

Only after this loop is stable should full Kanto/Johto/Hoenn map and trainer production begin.
