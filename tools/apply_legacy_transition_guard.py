#!/usr/bin/env python3
from pathlib import Path

ROOT = Path(__file__).resolve().parents[1]


def replace_once(text: str, old: str, new: str, label: str) -> str:
    count = text.count(old)
    if count != 1:
        raise RuntimeError(f"{label}: expected exactly one match, found {count}")
    return text.replace(old, new, 1)


def main() -> None:
    specials_path = ROOT / "data/specials.inc"
    specials = specials_path.read_text(encoding="utf-8")
    marker = "\tdef_special LegacyPrototype_BeginJohto\n"
    addition = "\tdef_special LegacyPrototype_CanBeginJohto\n"
    if addition not in specials:
        specials = replace_once(specials, marker, marker + addition, "transition guard special")
    specials_path.write_text(specials, encoding="utf-8")

    oldale_path = ROOT / "data/maps/OldaleTown/scripts.inc"
    oldale = oldale_path.read_text(encoding="utf-8")

    battle_marker = (
        "\tgoto_if_eq VAR_UNUSED_0x40F8, 1, OldaleTown_EventScript_JohtoActive\n"
        "\ttrainerbattle_single TRAINER_SAWYER_1, OldaleTown_Text_LegacyKantoChallenge, OldaleTown_Text_LegacyKantoDefeated, OldaleTown_EventScript_LegacyBeginJohto\n"
    )
    guarded_battle = (
        "\tgoto_if_eq VAR_UNUSED_0x40F8, 1, OldaleTown_EventScript_JohtoActive\n"
        "\tspecialvar VAR_RESULT, LegacyPrototype_CanBeginJohto\n"
        "\tgoto_if_eq VAR_RESULT, FALSE, OldaleTown_EventScript_LegacyStorageFull\n"
        "\ttrainerbattle_single TRAINER_SAWYER_1, OldaleTown_Text_LegacyKantoChallenge, OldaleTown_Text_LegacyKantoDefeated, OldaleTown_EventScript_LegacyBeginJohto\n"
    )
    if "LegacyPrototype_CanBeginJohto" not in oldale:
        oldale = replace_once(oldale, battle_marker, guarded_battle, "Oldale transition preflight")

    label_marker = "OldaleTown_EventScript_LegacyBeginJohto::\n"
    storage_label = (
        "OldaleTown_EventScript_LegacyStorageFull::\n"
        "\tmsgbox OldaleTown_Text_LegacyStorageFull, MSGBOX_NPC\n"
        "\tend\n\n"
    )
    if "OldaleTown_EventScript_LegacyStorageFull::" not in oldale:
        oldale = replace_once(oldale, label_marker, storage_label + label_marker, "Oldale storage-full branch")

    text_marker = "OldaleTown_Text_LegacyKantoChallenge:\n"
    storage_text = (
        "OldaleTown_Text_LegacyStorageFull:\n"
        "\t.string \"Your next journey needs room to preserve your current team.\\p\"\n"
        "\t.string \"Free enough PC storage for your party, then return for the KANTO LEAGUE test.$\"\n\n"
    )
    if "OldaleTown_Text_LegacyStorageFull:" not in oldale:
        oldale = replace_once(oldale, text_marker, storage_text + text_marker, "Oldale storage-full text")

    oldale_path.write_text(oldale, encoding="utf-8")
    print("Pokémon Legacy transition storage guard installed successfully.")


if __name__ == "__main__":
    main()
