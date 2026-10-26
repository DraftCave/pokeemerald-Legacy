#ifndef GUARD_CONSTANTS_LEGACY_SYSTEM_H
#define GUARD_CONSTANTS_LEGACY_SYSTEM_H

#include "constants/vars.h"

// Persistent campaign state. These aliases intentionally use vanilla-unused
// save variables so the Legacy System prototype does not change save-block
// layouts while the multi-region architecture is still being validated.
#define VAR_LEGACY_STATE_VERSION     VAR_UNUSED_0x40F7
#define VAR_LEGACY_CURRENT_REGION    VAR_UNUSED_0x40F8
#define VAR_LEGACY_ACCESS_MASK       VAR_UNUSED_0x40F9
#define VAR_LEGACY_CHAMPION_MASK     VAR_UNUSED_0x40FA
#define VAR_LEGACY_KANTO_BADGES      VAR_UNUSED_0x40FB
#define VAR_LEGACY_JOHTO_BADGES      VAR_UNUSED_0x40FC
#define VAR_LEGACY_HOENN_BADGES      VAR_UNUSED_0x40FD
#define VAR_LEGACY_RESERVED_1        VAR_UNUSED_0x40FE
#define VAR_LEGACY_RESERVED_2        VAR_UNUSED_0x40FF

#define LEGACY_SYSTEM_STATE_VERSION  1

#define LEGACY_REGION_MASK_KANTO     (1 << 0)
#define LEGACY_REGION_MASK_JOHTO     (1 << 1)
#define LEGACY_REGION_MASK_HOENN     (1 << 2)
#define LEGACY_REGION_MASK_ALL       (LEGACY_REGION_MASK_KANTO | LEGACY_REGION_MASK_JOHTO | LEGACY_REGION_MASK_HOENN)

#endif // GUARD_CONSTANTS_LEGACY_SYSTEM_H
