#include "global.h"
#include "event_data.h"
#include "legacy_system.h"
#include "main.h"
#include "overworld.h"
#include "pokemon.h"
#include "pokemon_storage_system.h"
#include "script_pokemon_util.h"
#include "starter_choose.h"
#include "constants/items.h"
#include "constants/legacy_system.h"

static void CB2_GiveLegacyStarter(void);
static void LegacySystem_CompactPlayerParty(void);
static bool8 LegacySystem_CopyBoxMonToStoragePreservingState(struct BoxPokemon *boxMon);

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
    VarSet(VAR_LEGACY_ACCESS_MASK, 0);

    badgeVar = LegacySystem_GetBadgeVar(region);
    if (badgeVar != 0)
        VarSet(badgeVar, 0);

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
        return FALSE;
    }
}

bool8 LegacySystem_IsMonEligible(const struct Pokemon *mon)
{
    return LegacySystem_IsBoxMonEligible(&mon->box);
}

static void LegacySystem_CompactPlayerParty(void)
{
    s32 readSlot;
    s32 writeSlot = 0;

    for (readSlot = 0; readSlot < PARTY_SIZE; readSlot++)
    {
        if (GetMonData(&gParties[B_TRAINER_PLAYER][readSlot], MON_DATA_SPECIES) == SPECIES_NONE)
            continue;

        if (readSlot != writeSlot)
        {
            CopyMon(&gParties[B_TRAINER_PLAYER][writeSlot],
                    &gParties[B_TRAINER_PLAYER][readSlot],
                    sizeof(struct Pokemon));
            ZeroMonData(&gParties[B_TRAINER_PLAYER][readSlot]);
        }
        writeSlot++;
    }

    for (; writeSlot < PARTY_SIZE; writeSlot++)
        ZeroMonData(&gParties[B_TRAINER_PLAYER][writeSlot]);

    CalculatePlayerPartyCount();
}

static bool8 LegacySystem_CopyBoxMonToStoragePreservingState(struct BoxPokemon *boxMon)
{
    u8 startBox;
    u8 box;

    if (gPokemonStoragePtr == NULL)
        return FALSE;

    startBox = StorageGetCurrentBox();
    box = startBox;

    do
    {
        s16 boxPos = GetFirstFreeBoxSpot(box);

        if (boxPos >= 0)
        {
            // Normal CopyMonToPC() restores PP before storing. A Legacy vault
            // must preserve the persistent BoxPokemon payload exactly instead.
            CopyMon(GetBoxedMonPtr(box, boxPos), boxMon, sizeof(*boxMon));
            return TRUE;
        }

        box++;
        if (box == TOTAL_BOXES_COUNT)
            box = 0;
    } while (box != startBox);

    return FALSE;
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

        if (LegacySystem_CopyBoxMonToStoragePreservingState(&mon->box))
            ZeroMonData(mon);
    }

    LegacySystem_CompactPlayerParty();
}

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

u16 LegacyPrototype_BeginHoenn(void)
{
    LegacySystem_MarkRegionChampion(LEGACY_REGION_JOHTO);
    LegacySystem_SetBadgeCount(LEGACY_REGION_JOHTO, 8);
    LegacySystem_BeginRegion(LEGACY_REGION_HOENN);
    return LegacySystem_GetChampionMask();
}

u16 LegacyPrototype_UnlockHoennLeague(void)
{
    LegacySystem_SetBadgeCount(LEGACY_REGION_HOENN, 8);
    LegacySystem_UnlockChampionsForCurrentLeague();
    return LegacySystem_GetLegacyAccessMask();
}

u16 LegacyPrototype_FinishHoenn(void)
{
    LegacySystem_MarkRegionChampion(LEGACY_REGION_HOENN);
    LegacySystem_SetBadgeCount(LEGACY_REGION_HOENN, 8);
    LegacySystem_EnterPostgame();
    return LegacySystem_GetChampionMask();
}
