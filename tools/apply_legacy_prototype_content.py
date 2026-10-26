#!/usr/bin/env python3
from __future__ import annotations

import json
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
    marker = "\tdef_special ChooseStarter, waitstate=1\n"
    additions = (
        "\tdef_special ChooseLegacyStarter, waitstate=1\n"
        "\tdef_special LegacyPrototype_BeginJohto\n"
        "\tdef_special LegacyPrototype_UnlockJohtoLeague\n"
        "\tdef_special LegacyPrototype_FinishJohto\n"
    )
    if "def_special ChooseLegacyStarter" not in text:
        text = replace_once(text, marker, marker + additions, label="Legacy specials")
    write(path, text)


def install_oldale() -> None:
    path = "data/maps/OldaleTown/scripts.inc"
    text = read(path)
    old = (
        "OldaleTown_EventScript_Girl::\n"
        "\tmsgbox OldaleTown_Text_SavingMyProgress, MSGBOX_NPC\n"
        "\tend\n"
    )
    new = r'''OldaleTown_EventScript_Girl::
	goto_if_eq VAR_UNUSED_0x40FA, 3, OldaleTown_EventScript_LegacyComplete
	goto_if_eq VAR_UNUSED_0x40F8, 1, OldaleTown_EventScript_JohtoActive
	trainerbattle_single TRAINER_SAWYER_1, OldaleTown_Text_LegacyKantoChallenge, OldaleTown_Text_LegacyKantoDefeated, OldaleTown_EventScript_LegacyBeginJohto
	msgbox OldaleTown_Text_LegacyKantoReady, MSGBOX_NPC
	end

OldaleTown_EventScript_LegacyBeginJohto::
	lock
	special LegacyPrototype_BeginJohto
	msgbox OldaleTown_Text_LegacyKantoChampion, MSGBOX_DEFAULT
	special ChooseLegacyStarter
	msgbox OldaleTown_Text_LegacyJohtoBegins, MSGBOX_DEFAULT
	release
	end

OldaleTown_EventScript_JohtoActive::
	goto_if_eq VAR_UNUSED_0x40F9, 1, OldaleTown_EventScript_JohtoLeagueOpen
	msgbox OldaleTown_Text_LegacyJohtoJourney, MSGBOX_NPC
	end

OldaleTown_EventScript_JohtoLeagueOpen::
	msgbox OldaleTown_Text_LegacyVeteransReturned, MSGBOX_NPC
	end

OldaleTown_EventScript_LegacyComplete::
	msgbox OldaleTown_Text_LegacyComplete, MSGBOX_NPC
	end

OldaleTown_Text_LegacyKantoChallenge:
	.string "This is the KANTO LEAGUE test.\p"
	.string "Show me the team you built on your first journey!$"

OldaleTown_Text_LegacyKantoDefeated:
	.string "You earned the title of KANTO CHAMPION!$"

OldaleTown_Text_LegacyKantoReady:
	.string "Your KANTO League title is already recorded.\p"
	.string "Your next journey is waiting.$"

OldaleTown_Text_LegacyKantoChampion:
	.string "KANTO is complete.\p"
	.string "Your veteran POKéMON are safe in the PC, but they are now Legacy-locked.\p"
	.string "JOHTO begins with a fresh partner.$"

OldaleTown_Text_LegacyJohtoBegins:
	.string "Take your new Lv. 5 partner west through ROUTE 102.\p"
	.string "Build a new team. Your KANTO veterans return at the JOHTO League.$"

OldaleTown_Text_LegacyJohtoJourney:
	.string "This is your JOHTO journey now.\p"
	.string "Your KANTO champions remain safely locked until the League checkpoint in PETALBURG.$"

OldaleTown_Text_LegacyVeteransReturned:
	.string "Legacy access is active.\p"
	.string "Your KANTO veterans may be withdrawn from the PC again.$"

OldaleTown_Text_LegacyComplete:
	.string "KANTO and JOHTO are both complete.\p"
	.string "The Legacy System vertical slice is complete!$"
'''
    if "OldaleTown_EventScript_LegacyBeginJohto::" not in text:
        text = replace_once(text, old, new, label="Oldale prototype NPC")
    write(path, text)


def install_petalburg() -> None:
    path = "data/maps/PetalburgCity/scripts.inc"
    text = read(path)
    old = (
        "PetalburgCity_EventScript_Boy::\n"
        "\tlock\n"
        "\tfaceplayer\n"
        "\tmsgbox PetalburgCity_Text_WaterReflection, MSGBOX_DEFAULT\n"
        "\tclosemessage\n"
        "\tapplymovement LOCALID_PETALBURG_BOY, Common_Movement_FaceOriginalDirection\n"
        "\twaitmovement 0\n"
        "\trelease\n"
        "\tend\n"
    )
    new = r'''PetalburgCity_EventScript_Boy::
	goto_if_eq VAR_UNUSED_0x40FA, 3, PetalburgCity_EventScript_LegacyComplete
	goto_if_eq VAR_UNUSED_0x40F8, 1, PetalburgCity_EventScript_LegacyJohto
	msgbox PetalburgCity_Text_LegacyFinishKanto, MSGBOX_NPC
	end

PetalburgCity_EventScript_LegacyJohto::
	goto_if_eq VAR_UNUSED_0x40F9, 0, PetalburgCity_EventScript_UnlockLegacy
	trainerbattle_single TRAINER_GABRIELLE_1, PetalburgCity_Text_LegacyJohtoChallenge, PetalburgCity_Text_LegacyJohtoDefeated, PetalburgCity_EventScript_FinishJohto
	msgbox PetalburgCity_Text_LegacyLeagueReady, MSGBOX_NPC
	end

PetalburgCity_EventScript_UnlockLegacy::
	lock
	special LegacyPrototype_UnlockJohtoLeague
	msgbox PetalburgCity_Text_LegacyUnlocked, MSGBOX_DEFAULT
	release
	end

PetalburgCity_EventScript_FinishJohto::
	lock
	special LegacyPrototype_FinishJohto
	msgbox PetalburgCity_Text_LegacyJohtoChampion, MSGBOX_DEFAULT
	release
	end

PetalburgCity_EventScript_LegacyComplete::
	msgbox PetalburgCity_Text_LegacyComplete, MSGBOX_NPC
	end

PetalburgCity_Text_LegacyFinishKanto:
	.string "The JOHTO checkpoint is not active yet.\p"
	.string "Finish the KANTO test League in OLDALE first.$"

PetalburgCity_Text_LegacyUnlocked:
	.string "Eight JOHTO badges are simulated for this vertical slice.\p"
	.string "LEGACY ACCESS RESTORED!\p"
	.string "Your KANTO veterans can be withdrawn from the PC. Build your mixed team, then speak to me again.$"

PetalburgCity_Text_LegacyJohtoChallenge:
	.string "Your new team got you here. Your old champions can stand beside them now.\p"
	.string "This JOHTO League is stronger than KANTO. Show me your Legacy team!$"

PetalburgCity_Text_LegacyJohtoDefeated:
	.string "You conquered the second League!$"

PetalburgCity_Text_LegacyLeagueReady:
	.string "Legacy access is active. Prepare your best mixed KANTO-JOHTO team.$"

PetalburgCity_Text_LegacyJohtoChampion:
	.string "JOHTO CHAMPION recorded!\p"
	.string "The vertical slice is complete: fresh journey, locked veterans, League unlock, mixed-team finale.$"

PetalburgCity_Text_LegacyComplete:
	.string "Two generations. Two championships.\p"
	.string "Next comes HOENN.$"
'''
    if "PetalburgCity_EventScript_LegacyJohto::" not in text:
        text = replace_once(text, old, new, label="Petalburg prototype checkpoint")
    write(path, text)


def install_route102_trainers() -> None:
    path = "data/maps/Route102/scripts.inc"
    text = read(path)
    old_calvin = '''Route102_EventScript_Calvin::
	trainerbattle_single TRAINER_CALVIN_1, Route102_Text_CalvinIntro, Route102_Text_CalvinDefeated, Route102_EventScript_CalvinRegisterMatchCallAfterBattle
	specialvar VAR_RESULT, ShouldTryRematchBattle
	goto_if_eq VAR_RESULT, TRUE, Route102_EventScript_CalvinRematch
	setvar VAR_0x8004, TRAINER_CALVIN_1
	specialvar VAR_RESULT, IsTrainerRegistered
	goto_if_eq VAR_RESULT, FALSE, Route102_EventScript_CalvinTryRegister
	msgbox Route102_Text_CalvinPostBattle, MSGBOX_DEFAULT
	release
	end
'''
    new_calvin = '''Route102_EventScript_Calvin::
	trainerbattle_single TRAINER_GRUNT_AQUA_HIDEOUT_1, Route102_Text_CalvinIntro, Route102_Text_CalvinDefeated
	msgbox Route102_Text_CalvinPostBattle, MSGBOX_DEFAULT
	end
'''
    if "TRAINER_GRUNT_AQUA_HIDEOUT_1, Route102_Text_CalvinIntro" not in text:
        text = replace_once(text, old_calvin, new_calvin, label="Route102 Calvin")
    replacements = {
        "\ttrainerbattle_single TRAINER_RICK, Route102_Text_RickIntro, Route102_Text_RickDefeated\n":
            "\ttrainerbattle_single TRAINER_GRUNT_AQUA_HIDEOUT_2, Route102_Text_RickIntro, Route102_Text_RickDefeated\n",
        "\ttrainerbattle_single TRAINER_TIANA, Route102_Text_TianaIntro, Route102_Text_TianaDefeated\n":
            "\ttrainerbattle_single TRAINER_GRUNT_AQUA_HIDEOUT_3, Route102_Text_TianaIntro, Route102_Text_TianaDefeated\n",
        "\ttrainerbattle_single TRAINER_ALLEN, Route102_Text_AllenIntro, Route102_Text_AllenDefeated\n":
            "\ttrainerbattle_single TRAINER_GRUNT_AQUA_HIDEOUT_4, Route102_Text_AllenIntro, Route102_Text_AllenDefeated\n",
    }
    for old, new in replacements.items():
        if new not in text:
            text = replace_once(text, old, new, label="Route102 trainer swap")
    write(path, text)


def install_trainer_parties() -> None:
    path = "src/data/trainers.party"
    text = read(path)
    blocks = {
        "TRAINER_SAWYER_1": '''Name: RED
Class: Cooltrainer
Pic: Cooltrainer M
Gender: Male
Music: Cool
Double Battle: No
AI: Basic Trainer

Rattata
Level: 6

Pidgey
Level: 7

Pikachu @ Oran Berry
Level: 8''',
        "TRAINER_GRUNT_AQUA_HIDEOUT_1": '''Name: JOEY
Class: Youngster
Pic: Youngster
Gender: Male
Music: Male
Double Battle: No
AI: Basic Trainer

Sentret
Level: 6''',
        "TRAINER_GRUNT_AQUA_HIDEOUT_2": '''Name: ETHAN
Class: Youngster
Pic: Youngster
Gender: Male
Music: Male
Double Battle: No
AI: Basic Trainer

Hoothoot
Level: 7''',
        "TRAINER_GRUNT_AQUA_HIDEOUT_3": '''Name: KRIS
Class: Lass
Pic: Lass
Gender: Female
Music: Female
Double Battle: No
AI: Basic Trainer

Mareep
Level: 8''',
        "TRAINER_GRUNT_AQUA_HIDEOUT_4": '''Name: SILVER
Class: Cooltrainer
Pic: Cooltrainer M
Gender: Male
Music: Cool
Double Battle: No
AI: Basic Trainer

Wooper @ Oran Berry
Level: 9''',
        "TRAINER_GABRIELLE_1": '''Name: LANCE
Class: Champion
Pic: Steven
Gender: Male
Music: Cool
Double Battle: No
AI: Basic Trainer

Pidgeotto
Level: 11

Flaaffy
Level: 12

Quagsire
Level: 12

Heracross
Level: 13

Espeon @ Oran Berry
Level: 13

Typhlosion @ Sitrus Berry
Level: 14''',
    }
    for trainer, body in blocks.items():
        text = replace_trainer_block(text, trainer, body)
    write(path, text)


def find_map_entries(node, map_name: str):
    if isinstance(node, dict):
        if node.get("map") == map_name:
            yield node
        for value in node.values():
            yield from find_map_entries(value, map_name)
    elif isinstance(node, list):
        for value in node:
            yield from find_map_entries(value, map_name)


def set_land_species(data, map_name: str, species: list[str]) -> None:
    entries = list(find_map_entries(data, map_name))
    if len(entries) != 1:
        raise RuntimeError(f"{map_name}: expected one encounter entry, found {len(entries)}")
    land = entries[0].get("land_mons")
    if not land or "mons" not in land:
        raise RuntimeError(f"{map_name}: no land encounter table")
    mons = land["mons"]
    if len(mons) != len(species):
        raise RuntimeError(f"{map_name}: expected {len(species)} land slots, found {len(mons)}")
    for slot, mon_species in zip(mons, species):
        slot["species"] = mon_species


def install_encounters() -> None:
    path = ROOT / "src/data/wild_encounters.json"
    data = json.loads(path.read_text(encoding="utf-8"))
    set_land_species(data, "MAP_ROUTE101", [
        "SPECIES_RATTATA", "SPECIES_PIDGEY", "SPECIES_RATTATA", "SPECIES_PIDGEY",
        "SPECIES_CATERPIE", "SPECIES_WEEDLE", "SPECIES_NIDORAN_F", "SPECIES_NIDORAN_M",
        "SPECIES_SPEAROW", "SPECIES_SPEAROW", "SPECIES_PIKACHU", "SPECIES_PIKACHU",
    ])
    set_land_species(data, "MAP_ROUTE102", [
        "SPECIES_SENTRET", "SPECIES_HOOTHOOT", "SPECIES_SENTRET", "SPECIES_HOOTHOOT",
        "SPECIES_MAREEP", "SPECIES_MAREEP", "SPECIES_LEDYBA", "SPECIES_SPINARAK",
        "SPECIES_WOOPER", "SPECIES_BELLSPROUT", "SPECIES_HOPPIP", "SPECIES_DROWZEE",
    ])
    path.write_text(json.dumps(data, indent=2, ensure_ascii=False) + "\n", encoding="utf-8")


def main() -> None:
    install_specials()
    install_oldale()
    install_petalburg()
    install_route102_trainers()
    install_trainer_parties()
    install_encounters()
    print("Pokémon Legacy prototype content installed successfully.")


if __name__ == "__main__":
    main()
