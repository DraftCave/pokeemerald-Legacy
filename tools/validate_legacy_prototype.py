#!/usr/bin/env python3
from __future__ import annotations

import json
import re
from pathlib import Path

ROOT = Path(__file__).resolve().parents[1]

ROUTE101_SPECIES = [
    "SPECIES_RATTATA", "SPECIES_PIDGEY", "SPECIES_RATTATA", "SPECIES_PIDGEY",
    "SPECIES_CATERPIE", "SPECIES_WEEDLE", "SPECIES_NIDORAN_F", "SPECIES_NIDORAN_M",
    "SPECIES_SPEAROW", "SPECIES_SPEAROW", "SPECIES_PIKACHU", "SPECIES_PIKACHU",
]

ROUTE102_SPECIES = [
    "SPECIES_SENTRET", "SPECIES_HOOTHOOT", "SPECIES_SENTRET", "SPECIES_HOOTHOOT",
    "SPECIES_MAREEP", "SPECIES_MAREEP", "SPECIES_LEDYBA", "SPECIES_SPINARAK",
    "SPECIES_WOOPER", "SPECIES_BELLSPROUT", "SPECIES_HOPPIP", "SPECIES_DROWZEE",
]


def read(path: str) -> str:
    return (ROOT / path).read_text(encoding="utf-8")


def require(path: str, *needles: str) -> None:
    text = read(path)
    missing = [needle for needle in needles if needle not in text]
    if missing:
        joined = "\n  - ".join(missing)
        raise RuntimeError(f"{path}: missing required prototype wiring:\n  - {joined}")


def trainer_levels(text: str, trainer: str) -> list[int]:
    pattern = re.compile(
        rf"=== {re.escape(trainer)} ===\n(.*?)(?=\n=== |\Z)",
        flags=re.DOTALL,
    )
    match = pattern.search(text)
    if not match:
        raise RuntimeError(f"trainer block missing: {trainer}")
    levels = [int(value) for value in re.findall(r"^Level:\s*(\d+)\s*$", match.group(1), flags=re.MULTILINE)]
    if not levels:
        raise RuntimeError(f"trainer block has no levels: {trainer}")
    return levels


def find_map_entries(node, map_name: str):
    if isinstance(node, dict):
        if node.get("map") == map_name:
            yield node
        for value in node.values():
            yield from find_map_entries(value, map_name)
    elif isinstance(node, list):
        for value in node:
            yield from find_map_entries(value, map_name)


def encounter_species(data, map_name: str) -> list[str]:
    entries = list(find_map_entries(data, map_name))
    if len(entries) != 1:
        raise RuntimeError(f"{map_name}: expected one encounter entry, found {len(entries)}")
    land = entries[0].get("land_mons")
    if not land or "mons" not in land:
        raise RuntimeError(f"{map_name}: missing land encounter table")
    return [slot["species"] for slot in land["mons"]]


def validate_script_flow() -> None:
    require(
        "data/maps/OldaleTown/scripts.inc",
        "LegacyPrototype_CanBeginJohto",
        "LegacyPrototype_BeginJohto",
        "ChooseLegacyStarter",
        "OldaleTown_EventScript_AllRegionsComplete::",
    )
    require(
        "data/maps/PetalburgCity/scripts.inc",
        "LegacyPrototype_UnlockJohtoLeague",
        "LegacyPrototype_FinishJohto",
        "LegacyPrototype_CanBeginHoenn",
        "LegacyPrototype_BeginHoenn",
        "PetalburgCity_EventScript_HoennStorageFull::",
        "PetalburgCity_EventScript_AllRegionsComplete::",
    )
    require(
        "data/maps/RustboroCity/scripts.inc",
        "LegacyPrototype_UnlockHoennLeague",
        "LegacyPrototype_FinishHoenn",
        "RustboroCity_EventScript_LegacyPostgame::",
    )
    require(
        "data/specials.inc",
        "def_special ChooseLegacyStarter",
        "def_special LegacyPrototype_CanBeginJohto",
        "def_special LegacyPrototype_BeginJohto",
        "def_special LegacyPrototype_UnlockJohtoLeague",
        "def_special LegacyPrototype_FinishJohto",
        "def_special LegacyPrototype_CanBeginHoenn",
        "def_special LegacyPrototype_BeginHoenn",
        "def_special LegacyPrototype_UnlockHoennLeague",
        "def_special LegacyPrototype_FinishHoenn",
    )


def validate_encounters() -> None:
    data = json.loads(read("src/data/wild_encounters.json"))
    actual101 = encounter_species(data, "MAP_ROUTE101")
    actual102 = encounter_species(data, "MAP_ROUTE102")

    if actual101 != ROUTE101_SPECIES:
        raise RuntimeError(f"Route 101 Kanto encounter pool drifted: {actual101}")
    if actual102 != ROUTE102_SPECIES:
        raise RuntimeError(f"Route 102 Johto encounter pool drifted: {actual102}")


def validate_boss_progression() -> None:
    trainers = read("src/data/trainers.party")
    kanto = trainer_levels(trainers, "TRAINER_SAWYER_1")
    johto = trainer_levels(trainers, "TRAINER_GABRIELLE_1")
    hoenn = trainer_levels(trainers, "TRAINER_MARCEL")

    if not (max(kanto) < max(johto) < max(hoenn)):
        raise RuntimeError(
            "Prototype League difficulty must rise Kanto -> Johto -> Hoenn; "
            f"max levels were {max(kanto)}, {max(johto)}, {max(hoenn)}"
        )


def validate_engine_invariants() -> None:
    require(
        "src/legacy_system.c",
        "LegacySystem_CanVaultPartyForRegion(region)",
        "LegacySystem_CopyBoxMonToStoragePreservingState",
        "LegacySystem_EnterPostgame();",
    )
    require(
        "patches/legacy_core.patch",
        "LegacySystem_IsBoxMonEligible",
        "Legacy-locked until the League",
    )


def main() -> None:
    validate_script_flow()
    validate_encounters()
    validate_boss_progression()
    validate_engine_invariants()
    print("Pokémon Legacy three-region semantic validation passed.")


if __name__ == "__main__":
    main()
