// ee_map.h

// Copyright 2012
// Graco Inc., Minneapolis, MN
// All Rights Reserved

// Getter - Setter functions for things in EEPROM.

#ifndef EE_MAP_H
#define EE_MAP_H

#include "dvinterface_17G721.h"
#include "dvseg_17G721_usage.h"

// *****************************************************************************
// MACROS
// *****************************************************************************

#define EE_MAP_VERSION (32) //increment when changing EEPROM addressing

#define EE_BASE (0x2000) // 8192
#define EE_MAX  (0x8000)

#define EE_gUsage_BASE (EE_BASE)
#define EE_gUsage_LEN (GUSAGE_DATASTORE_TOTAL_SIZE)
#define EE_gUsage_COUNT (1)

// Storage for setup variables
#define EE_gSetup_BASE (EE_gUsage_BASE + EE_gUsage_LEN * EE_gUsage_COUNT)
#define EE_gSetup_LEN  (sizeof(DVS17G721_gSetup_t))

#define EE_gLevelChart_BASE (EE_gSetup_BASE + EE_gSetup_LEN)
#define EE_gLevelChart_LEN (sizeof(DVS17G721_gLevelChart_t))

#define EE_LAST_USED (EE_gLevelChart_BASE + EE_gLevelChart_LEN - 1)

#endif /*EEPROMVARS_OFFSETS_H*/

