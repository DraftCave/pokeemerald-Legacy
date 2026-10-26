#include "global.h"
#include "legacy_system.h"
#include "pokemon.h"
#include "pokemon_storage_system.h"
#include "constants/legacy_system.h"

static enum LegacyPokemonOrigin GetTargetOrigin(enum LegacyCampaignRegion region)
{
    switch (region)
    {
    case LEGACY_REGION_KANTO:
        return LEGACY_ORIGIN_KANTO;
    case LEGACY_REGION_JOHTO:
        return LEGACY_ORIGIN_JOHTO;
    case LEGACY_REGION_HOENN:
        return LEGACY_ORIGIN_HOENN;
    default:
        return LEGACY_ORIGIN_UNSET;
    }
}

static u16 CountFreeStorageSlots(void)
{
    u16 freeSlots = 0;
    u8 box;

    if (gPokemonStoragePtr == NULL)
        return 0;

    for (box = 0; box < TOTAL_BOXES_COUNT; box++)
        freeSlots += IN_BOX_COUNT - CountMonsInBox(box);

    return freeSlots;
}

static u16 CountPartyMonsToVault(enum LegacyCampaignRegion targetRegion)
{
    enum LegacyPokemonOrigin targetOrigin = GetTargetOrigin(targetRegion);
    u16 count = 0;
    u8 i;

    if (targetOrigin == LEGACY_ORIGIN_UNSET)
        return PARTY_SIZE + 1;

    for (i = 0; i < PARTY_SIZE; i++)
    {
        struct Pokemon *mon = &gParties[B_TRAINER_PLAYER][i];

        if (GetMonData(mon, MON_DATA_SPECIES) == SPECIES_NONE)
            continue;

        // A fresh regional journey clears all previous-region access. Only a
        // Pokémon whose individual origin is the destination region could stay
        // active; every other owned party member must be safely vaulted first.
        if (LegacySystem_GetMonOrigin(mon) != targetOrigin)
            count++;
    }

    return count;
}

bool8 LegacySystem_CanVaultPartyForRegion(enum LegacyCampaignRegion targetRegion)
{
    u16 requiredSlots;

    if (targetRegion > LEGACY_REGION_HOENN)
        return FALSE;

    requiredSlots = CountPartyMonsToVault(targetRegion);
    return CountFreeStorageSlots() >= requiredSlots;
}

u16 LegacyPrototype_CanBeginJohto(void)
{
    return LegacySystem_CanVaultPartyForRegion(LEGACY_REGION_JOHTO);
}

u16 LegacyPrototype_CanBeginHoenn(void)
{
    return LegacySystem_CanVaultPartyForRegion(LEGACY_REGION_HOENN);
}
