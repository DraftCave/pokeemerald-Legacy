#!/usr/bin/env python3
from pathlib import Path

ROOT = Path(__file__).resolve().parents[1]


def replace_once(text: str, old: str, new: str, label: str) -> str:
    count = text.count(old)
    if count != 1:
        raise RuntimeError(f"{label}: expected exactly one match, found {count}")
    return text.replace(old, new, 1)


def main() -> None:
    path = ROOT / "data/maps/OldaleTown/scripts.inc"
    text = path.read_text(encoding="utf-8")

    old_head = (
        "OldaleTown_EventScript_Girl::\n"
        "\tgoto_if_eq VAR_UNUSED_0x40FA, 3, OldaleTown_EventScript_LegacyComplete\n"
        "\tgoto_if_eq VAR_UNUSED_0x40F8, 1, OldaleTown_EventScript_JohtoActive\n"
        "\tspecialvar VAR_RESULT, LegacyPrototype_CanBeginJohto\n"
    )
    new_head = (
        "OldaleTown_EventScript_Girl::\n"
        "\tgoto_if_eq VAR_UNUSED_0x40F8, 3, OldaleTown_EventScript_AllRegionsComplete\n"
        "\tgoto_if_eq VAR_UNUSED_0x40F8, 2, OldaleTown_EventScript_LegacyComplete\n"
        "\tgoto_if_eq VAR_UNUSED_0x40FA, 3, OldaleTown_EventScript_LegacyComplete\n"
        "\tgoto_if_eq VAR_UNUSED_0x40F8, 1, OldaleTown_EventScript_JohtoActive\n"
        "\tspecialvar VAR_RESULT, LegacyPrototype_CanBeginJohto\n"
    )
    if "OldaleTown_EventScript_AllRegionsComplete" not in text:
        text = replace_once(text, old_head, new_head, "Oldale terminal routing")

        label_marker = (
            "OldaleTown_EventScript_LegacyComplete::\n"
            "\tmsgbox OldaleTown_Text_LegacyComplete, MSGBOX_NPC\n"
            "\tend\n"
        )
        label_replacement = label_marker + (
            "\nOldaleTown_EventScript_AllRegionsComplete::\n"
            "\tmsgbox OldaleTown_Text_AllRegionsComplete, MSGBOX_NPC\n"
            "\tend\n"
        )
        text = replace_once(text, label_marker, label_replacement, "Oldale postgame label")

        text_marker = (
            "OldaleTown_Text_LegacyComplete:\n"
            "\t.string \"KANTO and JOHTO are both complete.\\p\"\n"
            "\t.string \"The Legacy System vertical slice is complete!$\"\n"
        )
        text_replacement = text_marker + (
            "\nOldaleTown_Text_AllRegionsComplete:\n"
            "\t.string \"KANTO, JOHTO, and HOENN are all complete.\\p\"\n"
            "\t.string \"Your full Legacy roster is available in the postgame.$\"\n"
        )
        text = replace_once(text, text_marker, text_replacement, "Oldale postgame text")

    path.write_text(text, encoding="utf-8")
    print("Pokémon Legacy terminal-state routing installed successfully.")


if __name__ == "__main__":
    main()
