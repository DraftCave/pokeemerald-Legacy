#ifndef GUARD_LEGACY_SYSTEM_H
#define GUARD_LEGACY_SYSTEM_H

enum LegacyCampaignRegion
{
    LEGACY_REGION_KANTO = 0,
    LEGACY_REGION_JOHTO,
    LEGACY_REGION_HOENN,
    LEGACY_REGION_POSTGAME,
};

void LegacySystem_Reset(void);
void LegacySystem_EnsureInitialized(void);
void LegacySystem_BeginRegion(enum LegacyCampaignRegion region);
void LegacySystem_EnterPostgame(void);

enum LegacyCampaignRegion LegacySystem_GetCurrentRegion(void);

void LegacySystem_SetBadgeCount(enum LegacyCampaignRegion region, u16 badgeCount);
u16 LegacySystem_GetBadgeCount(enum LegacyCampaignRegion region);

void LegacySystem_MarkRegionChampion(enum LegacyCampaignRegion region);
bool8 LegacySystem_IsRegionChampion(enum LegacyCampaignRegion region);

void LegacySystem_UnlockChampionsForCurrentLeague(void);
bool8 LegacySystem_IsRegionAccessible(enum LegacyCampaignRegion region);

u16 LegacySystem_GetLegacyAccessMask(void);
u16 LegacySystem_GetChampionMask(void);

#endif // GUARD_LEGACY_SYSTEM_H
