#!/usr/bin/env python3
from pathlib import Path

ROOT = Path(__file__).resolve().parents[1]


def replace_once(path: Path, old: str, new: str) -> None:
    text = path.read_text(encoding="utf-8")
    if new in text:
        return
    if old not in text:
        raise RuntimeError(f"{path}: expected anchor not found")
    path.write_text(text.replace(old, new, 1), encoding="utf-8")


# Engine fail-closed guard: never move an ineligible Day Care veteran into party.
daycare_c = ROOT / "src/daycare.c"
replace_once(
    daycare_c,
    '#include "global.h"\n',
    '#include "global.h"\n#include "legacy_system.h"\n',
)
replace_once(
    daycare_c,
    "u16 TakePokemonFromDaycare(void)\n{\n    return TakeSelectedPokemonMonFromDaycareShiftSlots(&gSaveBlock1Ptr->daycare, gSpecialVar_0x8004);\n}\n",
    "u16 TakePokemonFromDaycare(void)\n{\n    if (!LegacySystem_IsDaycareSlotEligible(gSpecialVar_0x8004))\n        return SPECIES_NONE;\n\n    return TakeSelectedPokemonMonFromDaycareShiftSlots(&gSaveBlock1Ptr->daycare, gSpecialVar_0x8004);\n}\n",
)

# Register the script-facing precheck immediately beside the normal Day Care specials.
specials = ROOT / "data/specials.inc"
replace_once(
    specials,
    "\tdef_special GetDaycareCostAndPrepareString\n\tdef_special TakePokemonFromDaycare\n",
    "\tdef_special GetDaycareCostAndPrepareString\n\tdef_special LegacyPrototype_CanTakeDaycareMon\n\tdef_special TakePokemonFromDaycare\n",
)

# Block before cost calculation/payment so the player is not charged for a locked veteran.
scripts = ROOT / "data/scripts/day_care.inc"
replace_once(
    scripts,
    "Route117_PokemonDayCare_EventScript_CostPrompt::\n\tspecial GetDaycareCostAndPrepareString\n",
    "Route117_PokemonDayCare_EventScript_CostPrompt::\n\tspecialvar VAR_RESULT, LegacyPrototype_CanTakeDaycareMon\n\tgoto_if_eq VAR_RESULT, 0, Route117_PokemonDayCare_EventScript_LegacyLocked\n\tspecial GetDaycareCostAndPrepareString\n",
)
replace_once(
    scripts,
    "Route117_PokemonDayCare_EventScript_NoRoom::\n\tmsgbox Route117_PokemonDayCare_Text_YourTeamIsFull, MSGBOX_DEFAULT\n\trelease\n\tend\n",
    "Route117_PokemonDayCare_EventScript_NoRoom::\n\tmsgbox Route117_PokemonDayCare_Text_YourTeamIsFull, MSGBOX_DEFAULT\n\trelease\n\tend\n\nRoute117_PokemonDayCare_EventScript_LegacyLocked::\n\tmsgbox Route117_PokemonDayCare_Text_LegacyLocked, MSGBOX_DEFAULT\n\trelease\n\tend\n",
)
replace_once(
    scripts,
    "Route117_PokemonDayCare_Text_NotEnoughMoney:\n\t.string \"You don't have enough money…$\"\n",
    "Route117_PokemonDayCare_Text_NotEnoughMoney:\n\t.string \"You don't have enough money…$\"\n\nRoute117_PokemonDayCare_Text_LegacyLocked:\n\t.string \"That POKéMON is Legacy-locked right now.\\p\"\n\t.string \"We'll keep it safe until you regain\\n\"\n\t.string \"Legacy access at the League.$\"\n",
)

print("Installed Pokémon Legacy Day Care withdrawal guard.")
