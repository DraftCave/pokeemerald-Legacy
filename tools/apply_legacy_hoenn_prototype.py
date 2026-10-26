#!/usr/bin/env python3
from __future__ import annotations

import re
from pathlib import Path

ROOT = Path(__file__).resolve().parents[1]


def read(path: str) -> str:
    return (ROOT / path).read_text(encoding="utf-8")


def write(path: str, content: str) -> None:
    (ROOT / path).write_text(content, encoding="utf-8")


def replace_once(text: str, old: str, new: str, *, label: str) -> str:
    count = text.count(old)
    if count != 1:
        raise RuntimeError(f"{label}: expected exactly one match, found {count}")
    return text.replace(old, new, 1)


def replace_trainer_block(text: str, trainer: str, body: str) -> str:
    pattern = re.compile(
        rf"=== {re.escape(trainer)} ===\n.*?(?=\n=== )",
        flags=re.DOTALL,
    )
    replacement = f"=== {trainer} ===\n{body.rstrip()}\n"
    text, count = pattern.subn(replacement, text, count=1)
    if count != 1:
        raise RuntimeError(f"trainer {trainer}: expected one block, replaced {count}")
    return text


def install_specials() -> None:
    path = "data/specials.inc"
    text = read(path)
    marker = "\tdef_special LegacyPrototype_FinishJohto\n"
    additions = (
        "\tdef_special LegacyPrototype_CanBeginHoenn\n"
        "\tdef_special LegacyPrototype_BeginHoenn\n"
        "\tdef_special LegacyPrototype_UnlockHoennLeague\n"
        "\tdef_special LegacyPrototype_FinishHoenn\n"
    )
    if "def_special LegacyPrototype_BeginHoenn" not in text:
        text = replace_once(text, marker, marker + additions, label="Hoenn Legacy specials")
    write(path, text)


def install_petalburg_transition() -> None:
    path = "data/maps/PetalburgCity/scripts.inc"
    text = read(path)

    old_head = '''PetalburgCity_EventScript_Boy::
	goto_if_eq VAR_UNUSED_0x40FA, 3, PetalburgCity_EventScript_LegacyComplete
	goto_if_eq VAR_UNUSED_0x40F8, 1, PetalburgCity_EventScript_LegacyJohto
	msgbox PetalburgCity_Text_LegacyFinishKanto, MSGBOX_NPC
	end
'''
    new_head = '''PetalburgCity_EventScript_Boy::
	goto_if_eq VAR_UNUSED_0x40F8, 3, PetalburgCity_EventScript_AllRegionsComplete
	goto_if_eq VAR_UNUSED_0x40F8, 2, PetalburgCity_EventScript_HoennActive
	goto_if_eq VAR_UNUSED_0x40FA, 3, PetalburgCity_EventScript_HoennPending
	goto_if_eq VAR_UNUSED_0x40F8, 1, PetalburgCity_EventScript_LegacyJohto
	msgbox PetalburgCity_Text_LegacyFinishKanto, MSGBOX_NPC
	end
'''
    if "PetalburgCity_EventScript_HoennPending" not in text:
        text = replace_once(text, old_head, new_head, label="Petalburg Hoenn routing")

    old_finish = '''PetalburgCity_EventScript_FinishJohto::
	lock
	special LegacyPrototype_FinishJohto
	msgbox PetalburgCity_Text_LegacyJohtoChampion, MSGBOX_DEFAULT
	release
	end

PetalburgCity_EventScript_LegacyComplete::
	msgbox PetalburgCity_Text_LegacyComplete, MSGBOX_NPC
	end
'''
    new_finish = '''PetalburgCity_EventScript_FinishJohto::
	lock
	special LegacyPrototype_FinishJohto
	msgbox PetalburgCity_Text_LegacyJohtoChampion, MSGBOX_DEFAULT
	goto PetalburgCity_EventScript_BeginHoenn

PetalburgCity_EventScript_HoennPending::
	lock
	faceplayer
	msgbox PetalburgCity_Text_LegacyComplete, MSGBOX_DEFAULT
	goto PetalburgCity_EventScript_BeginHoenn

PetalburgCity_EventScript_BeginHoenn::
	specialvar VAR_RESULT, LegacyPrototype_CanBeginHoenn
	goto_if_eq VAR_RESULT, FALSE, PetalburgCity_EventScript_HoennStorageFull
	special LegacyPrototype_BeginHoenn
	msgbox PetalburgCity_Text_HoennTransition, MSGBOX_DEFAULT
	special ChooseLegacyStarter
	msgbox PetalburgCity_Text_HoennBegins, MSGBOX_DEFAULT
	release
	end

PetalburgCity_EventScript_HoennStorageFull::
	msgbox PetalburgCity_Text_HoennStorageFull, MSGBOX_DEFAULT
	release
	end

PetalburgCity_EventScript_HoennActive::
	msgbox PetalburgCity_Text_HoennActive, MSGBOX_NPC
	end

PetalburgCity_EventScript_AllRegionsComplete::
	msgbox PetalburgCity_Text_AllRegionsComplete, MSGBOX_NPC
	end
'''
    if "PetalburgCity_EventScript_BeginHoenn::" not in text:
        text = replace_once(text, old_finish, new_finish, label="Petalburg Johto-to-Hoenn transition")

    old_text = '''PetalburgCity_Text_LegacyJohtoChampion:
	.string "JOHTO CHAMPION recorded!\\p"
	.string "The vertical slice is complete: fresh journey, locked veterans, League unlock, mixed-team finale.$"

PetalburgCity_Text_LegacyComplete:
	.string "Two generations. Two championships.\\p"
	.string "Next comes HOENN.$"
'''
    new_text = '''PetalburgCity_Text_LegacyJohtoChampion:
	.string "JOHTO CHAMPION recorded!\\p"
	.string "Your second journey is complete. HOENN is next.$"

PetalburgCity_Text_LegacyComplete:
	.string "Two generations. Two championships.\\p"
	.string "HOENN is ready when your veterans can be safely stored.$"

PetalburgCity_Text_HoennStorageFull:
	.string "Your next journey needs room to preserve your current team.\\p"
	.string "Free enough PC storage, then speak to me again.$"

PetalburgCity_Text_HoennTransition:
	.string "JOHTO is complete.\\p"
	.string "Your KANTO and JOHTO veterans are safe in the PC and Legacy-locked again.\\p"
	.string "HOENN begins with a fresh partner.$"

PetalburgCity_Text_HoennBegins:
	.string "Take your new Lv. 5 partner north toward RUSTBORO.\\p"
	.string "Your previous champions return at the HOENN League checkpoint.$"

PetalburgCity_Text_HoennActive:
	.string "This is your HOENN journey now.\\p"
	.string "Head north through the woods and find the League checkpoint in RUSTBORO.$"

PetalburgCity_Text_AllRegionsComplete:
	.string "KANTO, JOHTO, and HOENN are complete.\\p"
	.string "All Legacy restrictions are gone. Welcome to the postgame.$"
'''
    if "PetalburgCity_Text_HoennTransition:" not in text:
        text = replace_once(text, old_text, new_text, label="Petalburg Hoenn text")

    write(path, text)


def install_rustboro_checkpoint() -> None:
    path = "data/maps/RustboroCity/scripts.inc"
    text = read(path)
    old = '''RustboroCity_EventScript_FatMan::
	lock
	faceplayer
	goto_if_set FLAG_DEVON_GOODS_STOLEN, RustboroCity_EventScript_FatManSawGrunt
	msgbox RustboroCity_Text_WeShortenItToDevon, MSGBOX_DEFAULT
	release
	end

RustboroCity_EventScript_FatManSawGrunt::
	msgbox RustboroCity_Text_SneakyLookingManWentAroundCorner, MSGBOX_DEFAULT
	release
	end
'''
    new = r'''RustboroCity_EventScript_FatMan::
	goto_if_eq VAR_UNUSED_0x40F8, 3, RustboroCity_EventScript_LegacyPostgame
	goto_if_eq VAR_UNUSED_0x40F8, 2, RustboroCity_EventScript_LegacyHoenn
	msgbox RustboroCity_Text_WeShortenItToDevon, MSGBOX_NPC
	end

RustboroCity_EventScript_LegacyHoenn::
	goto_if_eq VAR_UNUSED_0x40F9, 0, RustboroCity_EventScript_UnlockHoennLegacy
	trainerbattle_single TRAINER_MARCEL, RustboroCity_Text_LegacyHoennChallenge, RustboroCity_Text_LegacyHoennDefeated, RustboroCity_EventScript_FinishHoenn
	msgbox RustboroCity_Text_LegacyHoennReady, MSGBOX_NPC
	end

RustboroCity_EventScript_UnlockHoennLegacy::
	lock
	faceplayer
	special LegacyPrototype_UnlockHoennLeague
	msgbox RustboroCity_Text_LegacyHoennUnlocked, MSGBOX_DEFAULT
	release
	end

RustboroCity_EventScript_FinishHoenn::
	lock
	special LegacyPrototype_FinishHoenn
	msgbox RustboroCity_Text_LegacyHoennChampion, MSGBOX_DEFAULT
	release
	end

RustboroCity_EventScript_LegacyPostgame::
	msgbox RustboroCity_Text_LegacyPostgame, MSGBOX_NPC
	end

RustboroCity_Text_LegacyHoennUnlocked:
	.string "Eight HOENN badges are simulated for this prototype.\p"
	.string "FULL LEGACY ACCESS RESTORED!\p"
	.string "KANTO and JOHTO veterans can join your HOENN team. Prepare, then speak to me again.$"

RustboroCity_Text_LegacyHoennChallenge:
	.string "Three journeys meet here.\p"
	.string "Show me the strongest team your LEGACY can build!$"

RustboroCity_Text_LegacyHoennDefeated:
	.string "You conquered the final prototype League!$"

RustboroCity_Text_LegacyHoennReady:
	.string "Full Legacy access is active. Build your best three-region team.$"

RustboroCity_Text_LegacyHoennChampion:
	.string "HOENN CHAMPION recorded!\p"
	.string "The three-region Legacy loop is complete. All regional restrictions are now removed.$"

RustboroCity_Text_LegacyPostgame:
	.string "Three regions. Three championships.\p"
	.string "Your entire collection is available now.$"
'''
    if "RustboroCity_EventScript_LegacyHoenn::" not in text:
        text = replace_once(text, old, new, label="Rustboro Hoenn checkpoint")
    write(path, text)


def install_hoenn_champion() -> None:
    path = "src/data/trainers.party"
    text = read(path)
    text = replace_trainer_block(text, "TRAINER_MARCEL", '''Name: WALLACE
Class: Champion
Pic: Steven
Gender: Male
Music: Cool
Double Battle: No
AI: Basic Trainer

Swellow
Level: 14

Breloom
Level: 15

Manectric
Level: 15

Flygon @ Oran Berry
Level: 16

Milotic @ Oran Berry
Level: 17

Metagross @ Sitrus Berry
Level: 18''')
    write(path, text)


def main() -> None:
    install_specials()
    install_petalburg_transition()
    install_rustboro_checkpoint()
    install_hoenn_champion()
    print("Pokémon Legacy Hoenn prototype phase installed successfully.")


if __name__ == "__main__":
    main()
