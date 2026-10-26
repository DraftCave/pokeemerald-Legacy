#include "global.h"
#include "daycare.h"
#include "event_data.h"
#include "legacy_system.h"
#include "pokemon.h"

bool8 LegacySystem_IsDaycareSlotEligible(u8 slot)
{
    struct BoxPokemon *boxMon;

    if (slot >= DAYCARE_MON_COUNT)
        return FALSE;

    boxMon = &gSaveBlock1Ptr->daycare.mons[slot].mon;
    if (GetBoxMonData(boxMon, MON_DATA_SPECIES) == SPECIES_NONE)
        return FALSE;

    return LegacySystem_IsBoxMonEligible(boxMon);
}

u16 LegacyPrototype_CanTakeDaycareMon(void)
{
    return LegacySystem_IsDaycareSlotEligible(gSpecialVar_0x8004);
}
