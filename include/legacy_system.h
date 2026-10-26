#ifndef GUARD_LEGACY_SYSTEM_H
#define GUARD_LEGACY_SYSTEM_H

struct BoxPokemon;
struct Pokemon;

enum LegacyCampaignRegion
{
    LEGACY_REGION_KANTO = 0,
    LEGACY_REGION_JOHTO,
    LEGACY_REGION_HOENN,
    LEGACY_REGION_POSTGAME,
};

// Stored on each individual Pokemon. Zero is intentionally UNSET so old saves
// and freshly zeroed structs fail closed instead of being misclassified as Kanto.
enum LegacyPokemonOrigin
{
    LEGACY_ORIGIN_UNSET = 0,
    LEGACY_ORIGIN_KANTO,
    LEGACY_ORIGIN_JOHTO,
    LEGACY_ORIGIN_HOENN,
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

enum LegacyPokemonOrigin LegacySystem_GetBoxMonOrigin(const struct BoxPokemon *boxMon);
enum LegacyPokemonOrigin LegacySystem_GetMonOrigin(const struct Pokemon *mon);
void LegacySystem_SetBoxMonOrigin(struct BoxPokemon *boxMon, enum LegacyPokemonOrigin origin);
void LegacySystem_SetMonOrigin(struct Pokemon *mon, enum LegacyPokemonOrigin origin);
void LegacySystem_AssignCurrentRegionOrigin(struct Pokemon *mon);
bool8 LegacySystem_IsBoxMonEligible(const struct BoxPokemon *boxMon);
bool8 LegacySystem_IsMonEligible(const struct Pokemon *mon);
void LegacySystem_StoreLockedPartyMons(void);
bool8 LegacySystem_CanVaultPartyForRegion(enum LegacyCampaignRegion targetRegion);
bool8 LegacySystem_IsDaycareSlotEligible(u8 slot);

// Script-facing helpers used only by the vertical-slice prototype.
void ChooseLegacyStarter(void);
u16 LegacyPrototype_CanBeginJohto(void);
u16 LegacyPrototype_BeginJohto(void);
u16 LegacyPrototype_UnlockJohtoLeague(void);
u16 LegacyPrototype_FinishJohto(void);
u16 LegacyPrototype_CanBeginHoenn(void);
u16 LegacyPrototype_BeginHoenn(void);
u16 LegacyPrototype_UnlockHoennLeague(void);
u16 LegacyPrototype_FinishHoenn(void);
u16 LegacyPrototype_CanTakeDaycareMon(void);

#endif // GUARD_LEGACY_SYSTEM_H
