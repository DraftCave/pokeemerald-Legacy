#include "global.h"
#include "event_data.h"
#include "legacy_system.h"
#include "main.h"
#include "overworld.h"
#include "party_menu.h"
#include "pokemon.h"
#include "script_pokemon_util.h"
#include "starter_choose.h"
#include "constants/items.h"
#include "constants/legacy_system.h"

static void CB2_GiveLegacyStarter(void);

static u16 LegacySystem_GetRegionBit(enum LegacyCampaignRegion region)
{
    switch (region)
    {
    case LEGACY_REGION_KANTO:
        return LEGACY_REGION_MASK_KANTO;
    case LEGACY_REGION_JOHTO:
        return LEGACY_REGION_MASK_JOHTO;
    case LEGACY_REGION_HOENN:
        return LEGACY_REGION_MASK_HOENN;
    default:
        return 0;
    }
}

static u16 LegacySystem_GetBadgeVar(enum LegacyCampaignRegion region)
{
    switch (region)
    {
    case LEGACY_REGION_KANTO:
        return VAR_LEGACY_KANTO_BADGES;
    case LEGACY_REGION_JOHTO:
        return VAR_LEGACY_JOHTO_BADGES;
    case LEGACY_REGION_HOENN:
        return VAR_LEGACY_HOENN_BADGES;
    default:
        return 0;
    }
}

static enum LegacyPokemonOrigin LegacySystem_GetOriginForRegion(enum LegacyCampaignRegion region)
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

void LegacySystem_Reset(void)
{
    VarSet(VAR_LEGACY_STATE_VERSION, LEGACY_SYSTEM_STATE_VERSION);
    VarSet(VAR_LEGACY_CURRENT_REGION, LEGACY_REGION_KANTO);
    VarSet(VAR_LEGACY_ACCESS_MASK, 0);
    VarSet(VAR_LEGACY_CHAMPION_MASK, 0);
    VarSet(VAR_LEGACY_KANTO_BADGES, 0);
    VarSet(VAR_LEGACY_JOHTO_BADGES, 0);
    VarSet(VAR_LEGACY_HOENN_BADGES, 0);
    VarSet(VAR_LEGACY_RESERVED_1, 0);
    VarSet(VAR_LEGACY_RESERVED_2, 0);
}

void LegacySystem_EnsureInitialized(void)
{
    if (VarGet(VAR_LEGACY_STATE_VERSION) != LEGACY_SYSTEM_STATE_VERSION)
        LegacySystem_Reset();
}

enum LegacyCampaignRegion LegacySystem_GetCurrentRegion(void)
{
    LegacySystem_EnsureInitialized();
    return (enum LegacyCampaignRegion)VarGet(VAR_LEGACY_CURRENT_REGION);
}

void LegacySystem_BeginRegion(enum LegacyCampaignRegion region)
{
    u16 badgeVar;

    LegacySystem_EnsureInitialized();

    if (region > LEGACY_REGION_HOENN)
        return;

    VarSet(VAR_LEGACY_CURRENT_REGION, region);

    // A new regional journey begins with legacy rosters locked. They are
    // restored only when the player reaches that region's League checkpoint.
    VarSet(VAR_LEGACY_ACCESS_MASK, 0);

    badgeVar = LegacySystem_GetBadgeVar(region);
    if (badgeVar != 0)
        VarSet(badgeVar, 0);

    // Preserve ownership and identity while removing newly ineligible veterans
    // from the active party. A mon is erased from the party only after it was
    // successfully copied to PC storage.
    LegacySystem_StoreLockedPartyMons();
}

void LegacySystem_EnterPostgame(void)
{
    LegacySystem_EnsureInitialized();
    VarSet(VAR_LEGACY_CURRENT_REGION, LEGACY_REGION_POSTGAME);
    VarSet(VAR_LEGACY_ACCESS_MASK, LEGACY_REGION_MASK_ALL);
}

void LegacySystem_SetBadgeCount(enum LegacyCampaignRegion region, u16 badgeCount)
{
    u16 badgeVar;

    LegacySystem_EnsureInitialized();

    if (badgeCount > 8)
        badgeCount = 8;

    badgeVar = LegacySystem_GetBadgeVar(region);
    if (badgeVar != 0)
        VarSet(badgeVar, badgeCount);
}

u16 LegacySystem_GetBadgeCount(enum LegacyCampaignRegion region)
{
    u16 badgeVar;

    LegacySystem_EnsureInitialized();

    badgeVar = LegacySystem_GetBadgeVar(region);
    if (badgeVar == 0)
        return 0;

    return VarGet(badgeVar);
}

void LegacySystem_MarkRegionChampion(enum LegacyCampaignRegion region)
{
    u16 regionBit;

    LegacySystem_EnsureInitialized();

    regionBit = LegacySystem_GetRegionBit(region);
    if (regionBit != 0)
        VarSet(VAR_LEGACY_CHAMPION_MASK, VarGet(VAR_LEGACY_CHAMPION_MASK) | regionBit);
}

bool8 LegacySystem_IsRegionChampion(enum LegacyCampaignRegion region)
{
    u16 regionBit;

    LegacySystem_EnsureInitialized();

    regionBit = LegacySystem_GetRegionBit(region);
    if (regionBit == 0)
        return FALSE;

    return (VarGet(VAR_LEGACY_CHAMPION_MASK) & regionBit) != 0;
}

void LegacySystem_UnlockChampionsForCurrentLeague(void)
{
    enum LegacyCampaignRegion currentRegion;
    u16 eligibleMask = 0;

    LegacySystem_EnsureInitialized();
    currentRegion = LegacySystem_GetCurrentRegion();

    switch (currentRegion)
    {
    case LEGACY_REGION_JOHTO:
        eligibleMask = LEGACY_REGION_MASK_KANTO;
        break;
    case LEGACY_REGION_HOENN:
        eligibleMask = LEGACY_REGION_MASK_KANTO | LEGACY_REGION_MASK_JOHTO;
        break;
    case LEGACY_REGION_POSTGAME:
        eligibleMask = LEGACY_REGION_MASK_ALL;
        break;
    case LEGACY_REGION_KANTO:
    default:
        eligibleMask = 0;
        break;
    }

    // Only completed regional championship rosters are eligible to return.
    VarSet(VAR_LEGACY_ACCESS_MASK, VarGet(VAR_LEGACY_CHAMPION_MASK) & eligibleMask);
}

bool8 LegacySystem_IsRegionAccessible(enum LegacyCampaignRegion region)
{
    enum LegacyCampaignRegion currentRegion;
    u16 regionBit;

    LegacySystem_EnsureInitialized();
    currentRegion = LegacySystem_GetCurrentRegion();

    if (currentRegion == LEGACY_REGION_POSTGAME)
        return region <= LEGACY_REGION_HOENN;

    if (region == currentRegion)
        return TRUE;

    regionBit = LegacySystem_GetRegionBit(region);
    if (regionBit == 0)
        return FALSE;

    return (VarGet(VAR_LEGACY_ACCESS_MASK) & regionBit) != 0;
}

u16 LegacySystem_GetLegacyAccessMask(void)
{
    LegacySystem_EnsureInitialized();
    return VarGet(VAR_LEGACY_ACCESS_MASK);
}

u16 LegacySystem_GetChampionMask(void)
{
    LegacySystem_EnsureInitialized();
    return VarGet(VAR_LEGACY_CHAMPION_MASK);
}

enum LegacyPokemonOrigin LegacySystem_GetBoxMonOrigin(const struct BoxPokemon *boxMon)
{
    // These are two top-level BoxPokemon bits that are unused by the current
    // expansion. Keeping the origin outside the encrypted substruct preserves
    // checksum behavior and sizeof(struct BoxPokemon).
    return (enum LegacyPokemonOrigin)((boxMon->unused_13 ? 1 : 0)
        | (boxMon->unused_1E ? 2 : 0));
}

enum LegacyPokemonOrigin LegacySystem_GetMonOrigin(const struct Pokemon *mon)
{
    return LegacySystem_GetBoxMonOrigin(&mon->box);
}

void LegacySystem_SetBoxMonOrigin(struct BoxPokemon *boxMon, enum LegacyPokemonOrigin origin)
{
    if (origin > LEGACY_ORIGIN_HOENN)
        origin = LEGACY_ORIGIN_UNSET;

    boxMon->unused_13 = origin & 1;
    boxMon->unused_1E = (origin >> 1) & 1;
}

void LegacySystem_SetMonOrigin(struct Pokemon *mon, enum LegacyPokemonOrigin origin)
{
    LegacySystem_SetBoxMonOrigin(&mon->box, origin);
}

void LegacySystem_AssignCurrentRegionOrigin(struct Pokemon *mon)
{
    enum LegacyPokemonOrigin origin;

    if (LegacySystem_GetMonOrigin(mon) != LEGACY_ORIGIN_UNSET)
        return;

    origin = LegacySystem_GetOriginForRegion(LegacySystem_GetCurrentRegion());
    if (origin != LEGACY_ORIGIN_UNSET)
        LegacySystem_SetMonOrigin(mon, origin);
}

bool8 LegacySystem_IsBoxMonEligible(const struct BoxPokemon *boxMon)
{
    enum LegacyPokemonOrigin origin;

    if (GetBoxMonData((struct BoxPokemon *)boxMon, MON_DATA_SPECIES) == SPECIES_NONE)
        return TRUE;

    if (LegacySystem_GetCurrentRegion() == LEGACY_REGION_POSTGAME)
        return TRUE;

    origin = LegacySystem_GetBoxMonOrigin(boxMon);
    switch (origin)
    {
    case LEGACY_ORIGIN_KANTO:
        return LegacySystem_IsRegionAccessible(LEGACY_REGION_KANTO);
    case LEGACY_ORIGIN_JOHTO:
        return LegacySystem_IsRegionAccessible(LEGACY_REGION_JOHTO);
    case LEGACY_ORIGIN_HOENN:
        return LegacySystem_IsRegionAccessible(LEGACY_REGION_HOENN);
    case LEGACY_ORIGIN_UNSET:
    default:
        // Fail closed. Missing tagging must never silently bypass a regional
        // lock; this also makes acquisition-path bugs obvious during testing.
        return FALSE;
    }
}

bool8 LegacySystem_IsMonEligible(const struct Pokemon *mon)
{
    return LegacySystem_IsBoxMonEligible(&mon->box);
}

void LegacySystem_StoreLockedPartyMons(void)
{
    s32 i;

    for (i = PARTY_SIZE - 1; i >= 0; i--)
    {
        struct Pokemon *mon = &gParties[B_TRAINER_PLAYER][i];

        if (GetMonData(mon, MON_DATA_SPECIES) == SPECIES_NONE)
            continue;
        if (LegacySystem_IsMonEligible(mon))
            continue;

        // Never destroy a veteran unless the PC copy succeeds.
        if (CopyMonToPC(mon) != MON_CANT_GIVE)
            ZeroMonData(mon);
    }

    CompactPartySlots();
    CalculatePlayerPartyCount();
}

// -----------------------------------------------------------------------------
// Script-facing prototype helpers
// -----------------------------------------------------------------------------

void ChooseLegacyStarter(void)
{
    SetMainCallback2(CB2_ChooseStarter);
    gMain.savedCallback = CB2_GiveLegacyStarter;
}

static void CB2_GiveLegacyStarter(void)
{
    enum Species starterMon = GetStarterPokemon(gSpecialVar_Result);

    ScriptGiveMon(starterMon, 5, ITEM_NONE);
    SetMainCallback2(CB2_ReturnToFieldContinueScript);
}

u16 LegacyPrototype_BeginJohto(void)
{
    LegacySystem_MarkRegionChampion(LEGACY_REGION_KANTO);
    LegacySystem_SetBadgeCount(LEGACY_REGION_KANTO, 8);
    LegacySystem_BeginRegion(LEGACY_REGION_JOHTO);
    return LegacySystem_GetChampionMask();
}

u16 LegacyPrototype_UnlockJohtoLeague(void)
{
    LegacySystem_SetBadgeCount(LEGACY_REGION_JOHTO, 8);
    LegacySystem_UnlockChampionsForCurrentLeague();
    return LegacySystem_GetLegacyAccessMask();
}

u16 LegacyPrototype_FinishJohto(void)
{
    LegacySystem_MarkRegionChampion(LEGACY_REGION_JOHTO);
    return LegacySystem_GetChampionMask();
}
