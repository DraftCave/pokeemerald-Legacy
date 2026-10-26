#include "global.h"
#include "event_data.h"
#include "legacy_system.h"
#include "constants/legacy_system.h"

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
