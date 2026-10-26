#include "global.h"
#include "event_data.h"
#include "legacy_system.h"
#include "pokemon.h"
#include "test/test.h"
#include "constants/legacy_system.h"

static void ResetLegacyTestState(void)
{
    InitEventData();
    ZeroPlayerPartyMons();
    LegacySystem_Reset();
}

static void CreateTestMon(struct Pokemon *mon, enum Species species)
{
    ZeroMonData(mon);
    CreateMon(mon, species, 5, 0, OTID_STRUCT_PRESET(0));
}

TEST("Legacy origin is attached to the individual Pokemon and survives copying")
{
    struct Pokemon original;
    struct Pokemon copy;

    ResetLegacyTestState();
    CreateTestMon(&original, SPECIES_PIKACHU);

    EXPECT_EQ(LegacySystem_GetMonOrigin(&original), LEGACY_ORIGIN_UNSET);
    LegacySystem_AssignCurrentRegionOrigin(&original);
    EXPECT_EQ(LegacySystem_GetMonOrigin(&original), LEGACY_ORIGIN_KANTO);

    ZeroMonData(&copy);
    CopyMon(&copy, &original, sizeof(struct Pokemon));

    EXPECT_EQ(LegacySystem_GetMonOrigin(&copy), LEGACY_ORIGIN_KANTO);
    EXPECT_EQ(GetMonData(&copy, MON_DATA_SPECIES), SPECIES_PIKACHU);
}

TEST("Unknown-origin Pokemon fail closed during a regional campaign")
{
    struct Pokemon mon;

    ResetLegacyTestState();
    CreateTestMon(&mon, SPECIES_PIKACHU);

    EXPECT(!LegacySystem_IsMonEligible(&mon));
}

TEST("Entering Johto locks Kanto Pokemon until the League checkpoint")
{
    struct Pokemon kantoMon;
    struct Pokemon johtoMon;

    ResetLegacyTestState();
    CreateTestMon(&kantoMon, SPECIES_PIKACHU);
    LegacySystem_SetMonOrigin(&kantoMon, LEGACY_ORIGIN_KANTO);
    LegacySystem_MarkRegionChampion(LEGACY_REGION_KANTO);

    LegacySystem_BeginRegion(LEGACY_REGION_JOHTO);

    CreateTestMon(&johtoMon, SPECIES_MAREEP);
    LegacySystem_AssignCurrentRegionOrigin(&johtoMon);

    EXPECT_EQ(LegacySystem_GetMonOrigin(&johtoMon), LEGACY_ORIGIN_JOHTO);
    EXPECT(!LegacySystem_IsMonEligible(&kantoMon));
    EXPECT(LegacySystem_IsMonEligible(&johtoMon));

    LegacySystem_UnlockChampionsForCurrentLeague();

    EXPECT(LegacySystem_IsMonEligible(&kantoMon));
    EXPECT(LegacySystem_IsMonEligible(&johtoMon));
}

TEST("Hoenn League access restores both completed previous regions")
{
    struct Pokemon kantoMon;
    struct Pokemon johtoMon;
    struct Pokemon hoennMon;

    ResetLegacyTestState();
    CreateTestMon(&kantoMon, SPECIES_VENUSAUR);
    CreateTestMon(&johtoMon, SPECIES_TYPHLOSION);
    CreateTestMon(&hoennMon, SPECIES_SWAMPERT);
    LegacySystem_SetMonOrigin(&kantoMon, LEGACY_ORIGIN_KANTO);
    LegacySystem_SetMonOrigin(&johtoMon, LEGACY_ORIGIN_JOHTO);
    LegacySystem_SetMonOrigin(&hoennMon, LEGACY_ORIGIN_HOENN);

    LegacySystem_MarkRegionChampion(LEGACY_REGION_KANTO);
    LegacySystem_MarkRegionChampion(LEGACY_REGION_JOHTO);
    LegacySystem_BeginRegion(LEGACY_REGION_HOENN);

    EXPECT(!LegacySystem_IsMonEligible(&kantoMon));
    EXPECT(!LegacySystem_IsMonEligible(&johtoMon));
    EXPECT(LegacySystem_IsMonEligible(&hoennMon));

    LegacySystem_UnlockChampionsForCurrentLeague();

    EXPECT(LegacySystem_IsMonEligible(&kantoMon));
    EXPECT(LegacySystem_IsMonEligible(&johtoMon));
    EXPECT(LegacySystem_IsMonEligible(&hoennMon));
}

TEST("Hoenn League only restores previous regions that were actually completed")
{
    struct Pokemon kantoMon;
    struct Pokemon johtoMon;

    ResetLegacyTestState();
    CreateTestMon(&kantoMon, SPECIES_BLASTOISE);
    CreateTestMon(&johtoMon, SPECIES_MEGANIUM);
    LegacySystem_SetMonOrigin(&kantoMon, LEGACY_ORIGIN_KANTO);
    LegacySystem_SetMonOrigin(&johtoMon, LEGACY_ORIGIN_JOHTO);

    LegacySystem_MarkRegionChampion(LEGACY_REGION_KANTO);
    LegacySystem_BeginRegion(LEGACY_REGION_HOENN);
    LegacySystem_UnlockChampionsForCurrentLeague();

    EXPECT(LegacySystem_IsMonEligible(&kantoMon));
    EXPECT(!LegacySystem_IsMonEligible(&johtoMon));
    EXPECT_EQ(LegacySystem_GetLegacyAccessMask(), LEGACY_REGION_MASK_KANTO);
}

TEST("Prototype helpers advance Kanto through Johto and Hoenn into postgame")
{
    ResetLegacyTestState();

    EXPECT_EQ(LegacySystem_GetCurrentRegion(), LEGACY_REGION_KANTO);
    EXPECT_EQ(LegacySystem_GetChampionMask(), 0);
    EXPECT_EQ(LegacySystem_GetLegacyAccessMask(), 0);

    EXPECT_EQ(LegacyPrototype_BeginJohto(), LEGACY_REGION_MASK_KANTO);
    EXPECT_EQ(LegacySystem_GetCurrentRegion(), LEGACY_REGION_JOHTO);
    EXPECT_EQ(LegacySystem_GetBadgeCount(LEGACY_REGION_KANTO), 8);
    EXPECT_EQ(LegacySystem_GetLegacyAccessMask(), 0);

    EXPECT_EQ(LegacyPrototype_UnlockJohtoLeague(), LEGACY_REGION_MASK_KANTO);
    EXPECT_EQ(LegacySystem_GetBadgeCount(LEGACY_REGION_JOHTO), 8);
    EXPECT_EQ(LegacySystem_GetLegacyAccessMask(), LEGACY_REGION_MASK_KANTO);

    EXPECT_EQ(LegacyPrototype_FinishJohto(), LEGACY_REGION_MASK_KANTO | LEGACY_REGION_MASK_JOHTO);

    EXPECT_EQ(LegacyPrototype_BeginHoenn(), LEGACY_REGION_MASK_KANTO | LEGACY_REGION_MASK_JOHTO);
    EXPECT_EQ(LegacySystem_GetCurrentRegion(), LEGACY_REGION_HOENN);
    EXPECT_EQ(LegacySystem_GetBadgeCount(LEGACY_REGION_JOHTO), 8);
    EXPECT_EQ(LegacySystem_GetLegacyAccessMask(), 0);

    EXPECT_EQ(LegacyPrototype_UnlockHoennLeague(), LEGACY_REGION_MASK_KANTO | LEGACY_REGION_MASK_JOHTO);
    EXPECT_EQ(LegacySystem_GetBadgeCount(LEGACY_REGION_HOENN), 8);
    EXPECT_EQ(LegacySystem_GetLegacyAccessMask(), LEGACY_REGION_MASK_KANTO | LEGACY_REGION_MASK_JOHTO);

    EXPECT_EQ(LegacyPrototype_FinishHoenn(), LEGACY_REGION_MASK_ALL);
    EXPECT_EQ(LegacySystem_GetCurrentRegion(), LEGACY_REGION_POSTGAME);
    EXPECT_EQ(LegacySystem_GetChampionMask(), LEGACY_REGION_MASK_ALL);
    EXPECT_EQ(LegacySystem_GetLegacyAccessMask(), LEGACY_REGION_MASK_ALL);
}

TEST("Completing Hoenn enters postgame and removes regional eligibility restrictions")
{
    struct Pokemon kantoMon;
    struct Pokemon johtoMon;
    struct Pokemon hoennMon;

    ResetLegacyTestState();
    CreateTestMon(&kantoMon, SPECIES_CHARIZARD);
    CreateTestMon(&johtoMon, SPECIES_FERALIGATR);
    CreateTestMon(&hoennMon, SPECIES_SCEPTILE);
    LegacySystem_SetMonOrigin(&kantoMon, LEGACY_ORIGIN_KANTO);
    LegacySystem_SetMonOrigin(&johtoMon, LEGACY_ORIGIN_JOHTO);
    LegacySystem_SetMonOrigin(&hoennMon, LEGACY_ORIGIN_HOENN);

    LegacySystem_MarkRegionChampion(LEGACY_REGION_KANTO);
    LegacySystem_MarkRegionChampion(LEGACY_REGION_JOHTO);
    LegacySystem_BeginRegion(LEGACY_REGION_HOENN);
    LegacyPrototype_FinishHoenn();

    EXPECT_EQ(LegacySystem_GetCurrentRegion(), LEGACY_REGION_POSTGAME);
    EXPECT_EQ(LegacySystem_GetChampionMask(), LEGACY_REGION_MASK_ALL);
    EXPECT(LegacySystem_IsMonEligible(&kantoMon));
    EXPECT(LegacySystem_IsMonEligible(&johtoMon));
    EXPECT(LegacySystem_IsMonEligible(&hoennMon));
}
