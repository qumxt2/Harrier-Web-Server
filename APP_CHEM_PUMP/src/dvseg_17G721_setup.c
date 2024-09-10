// dvseg_17G721_setup.c

// Copyright 2015 - 2017
// Graco Inc., Minneapolis, MN
// All Rights Reserved

// This file contains the setup dvars used to store system setup information

// *****************************************************************************
// * HEADER FILES
// *****************************************************************************

#include "dvseg_17G721_setup.h"
#include "eeprom.h"
#include "rtos.h"
#include "assert.h"
#include "define_app.h"         //For APP_MAGIC_NUMBER
#include "dvcb_helper.h"
#include "ee_map.h"
#include "dvinterface_17G721.h"
#include "debug.h"
#include "securekey.h"
#include "alarms.h"
#include "io_typedef.h"
#include "in_pressure.h"
#include "NetworkScreen.h"
#include "AdvancedScreen.h"
#include "TankScreen.h"
#include "pressure_15M669.h"
#include "pressure_420420.h"
#include "debug_app.h"
#include "d2a.h"
#include "AlarmScreenControl.h"
#include "TankScreenCustom.h"
#include "stdio.h"
#include "CountDigit.h"
#include "limits.h"
#include "stdio.h"
#include "utilities.h"
#include "dvseg_17G721_run.h"
#include "TankScreen.h"
#include "FlowScreen.h"
#include "volumeTask.h"

// *****************************************************************************
// * MACROS
// *****************************************************************************

#define MAGIC_NUMBER_AND_VERSION                (APP_MAGIC_NUMBER)
#define IDX(element)                            (DVA17G721_SS(gSetup,element) - DVA17G721_gSetup_BASE)
#define PUBLICATION_PERIOD_DEFAULT              (15*60)      // 15 minutes
#define FLUID_DENSITY_MIN                       1            // 00.01
#define FLUID_DENSITY_MAX                       9999         // 99.99
#define FLUID_DENSITY_DEF                       833          // 8.33
#define OFFSET_MIN                              0            // mV
#define OFFSET_MAX                              0x7D00000    // 2000 mV s16d16

// *****************************************************************************
// * TYPEDEFS & STRUCTURES
// *****************************************************************************

// *****************************************************************************
// * PUBLIC VARIABLES
// *****************************************************************************

DVS17G721_gSetup_t gSetup;

// *****************************************************************************
// * PRIVATE FUNCTION PROTOTYPES
// *****************************************************************************

static DistVarType Callback( DistVarID id, DistVarType oldVal, DistVarType newVal );
static DistVarType systemPressure_1_OffsetCallback( DistVarID id, DistVarType oldVal, DistVarType newVal, DistVarType defVal );
static DistVarType systemPressure_1_SlopeCallback( DistVarID id, DistVarType oldVal, DistVarType newVal, DistVarType defVal );
static DistVarType systemPressure_2_OffsetCallback( DistVarID id, DistVarType oldVal, DistVarType newVal, DistVarType defVal );
static DistVarType systemPressure_2_SlopeCallback( DistVarID id, DistVarType oldVal, DistVarType newVal, DistVarType defVal );
static IOErrorCode_t IOHARDWARE_PressureSensorCal( IOHARDWARE_PRESSURESENSOR_t port, s16d16_t zero_cal_mVpV_s16d16, s16d16_t sensitivity_cal_mVpV_s16d16 );
static IOrtn_mV_to_psi_int_t GetReferencePSIAveraged(PressureSensor_Port_t port);

// *****************************************************************************
// * PRIVATE VARIABLES
// *****************************************************************************

//DVCB_Helper callback definition arrays; indexed by offset within segment
static const DVCB_definition_t sDvcbSetup[sizeof (DVS17G721_gSetup_t) / sizeof(DistVarType)] = {
    [IDX( magicNumber_u32 )] =
    { NULL, DVCB_CHECK_DEF, 0, 0, MAGIC_NUMBER_AND_VERSION },

    [IDX( MeteringMode )] =
    { NULL, DVCB_CHECK_REJECT_DEF, 0, 3, 0 },

    [IDX( OnTime )] =
    { NULL, DVCB_CHECK_REJECT_DEF, 1, 359999, 10 },

    [IDX( OffTime )] =
    { NULL, DVCB_CHECK_REJECT_DEF, 0, 359999, 10 },

    [IDX( OnCycles )] =
    { NULL, DVCB_CHECK_REJECT_DEF, 0, 99, 5 },

    [IDX( OnTimeout )] =
    { NULL, DVCB_CHECK_REJECT_DEF, 1, 359999, 300},

    [IDX( PowerSaveMode )] =
    { NULL, DVCB_CHECK_REJECT_DEF, 0, 4, 3},

    [IDX( Units )] =
    { NULL, DVCB_CHECK_REJECT_DEF, 0, 1, 0},

    [IDX( Pressure_1_Offset )] =
    { systemPressure_1_OffsetCallback, DVCB_CHECK_COERCE, 0, 0x7FFFFFFF, 0x0000CCCC},

    [IDX( Pressure_1_Slope )] =
    { systemPressure_1_SlopeCallback, DVCB_CHECK_COERCE, 0, 0x7FFFFFFF, 0x00230000},

    [IDX( KFactor )] =
    { NULL, DVCB_CHECK_REJECT_DEF, 1, 9999, 106 },

    [IDX( DesiredFlowRate )] =
    { NULL, DVCB_CHECK_REJECT_DEF, MIN_FLOW_RATE_GAL, MAX_FLOW_RATE_GAL, 500 },

    [IDX( TotalBackup )] =
    { NULL, DVCB_CHECK_REJECT_DEF, 0, 0xFFFFFFFF, 0 },

    [IDX( GrandTotalBackup )] =
    { NULL, DVCB_CHECK_REJECT_DEF, 0, 0xFFFFFFFF, 0 },

    [IDX( Location )] =
    { NULL, DVCB_CHECK_REJECT_DEF, 0, 0xFFFFFFFF, 0 },

    [IDX( SoftwareVersion )] =
    { NULL, DVCB_CHECK_REJECT_DEF, 0, 0xFFFFFFFF, 0 },
    
    [IDX( NetworkMode )] =
    { NULL, DVCB_CHECK_REJECT_DEF, 0, 2, NETWORK_MODE_NO_NETWORK},

    [IDX( ScreenPin )] =
    { NULL, DVCB_CHECK_REJECT_DEF, 0, 9999, 0 },

    [IDX( ScreenPinEnable )] =
    { NULL, DVCB_CHECK_REJECT_DEF, 0, 1, 0 },

    [IDX( HighPressureTrigger )] =
    { NULL, DVCB_CHECK_COERCE, 0, 30000, 7500 },

    [IDX( LowPressureTrigger )] =
    { NULL, DVCB_CHECK_COERCE, 0, 30000, 0 },

    [IDX( BatteryWarningTrigger )] =
    { NULL, DVCB_CHECK_REJECT_DEF, 0, 99999, 11500 },
    
    [IDX( BatteryShutoffTrigger )] =
    { NULL, DVCB_CHECK_REJECT_DEF, 0, 99999, 11000 },

    [IDX( Alarm1Trigger )] =
    { NULL, DVCB_CHECK_REJECT_DEF, 0, NUMBER_LOGIC_LEVELS-1, LOGIC_DISABLED },

    [IDX( Alarm2Trigger )] =
    { NULL, DVCB_CHECK_REJECT_DEF, 0, NUMBER_LOGIC_LEVELS-1, LOGIC_DISABLED },

    [IDX( RemoteOffTrigger )] =
    { NULL, DVCB_CHECK_REJECT_DEF, 0, NUMBER_LOGIC_LEVELS-1, LOGIC_DISABLED },

    [IDX( SystemPublicationPeriod )] =
    { NULL, DVCB_CHECK_REJECT_DEF, 10, 99999, PUBLICATION_PERIOD_DEFAULT }, //seconds

    [IDX( ModbusSlaveID )] =
    { NULL, DVCB_CHECK_REJECT_DEF, 0, 0x7FFFFFFF, 0x1 },

    [IDX( ModbusBaudrateIndex )] =
    { NULL, DVCB_CHECK_COERCE, 0, 3, 3 },       // Test fixture requires 115,200 default
    
    [IDX( VolumeModeInterval )] =
    { NULL, DVCB_CHECK_REJECT_DEF, 0, 2, 0},    

    [IDX( AlarmAction )] =
    { NULL, DVCB_CHECK_REJECT_DEF, 0, NUMBER_ALARM_ACTIONS-1, ACTION_ALARM },

    [IDX( TankType )] =
    { NULL, DVCB_CHECK_COERCE, 0, 2, 0 },

    [IDX( TankLevelNotifyTrigger )] =
    { NULL, DVCB_CHECK_COERCE, 0, MAX_VOLUME_G*10, 0 },

    [IDX( TankLevelShutoffTrigger )] =
    { NULL, DVCB_CHECK_COERCE, 0, MAX_VOLUME_G*10, 0 },

    [IDX( FluidDensity )] =
    { NULL, DVCB_CHECK_REJECT_DEF, FLUID_DENSITY_MIN, FLUID_DENSITY_MAX, FLUID_DENSITY_DEF },
    
    [IDX( FlowVerifyEnable )] =
    { NULL, DVCB_CHECK_REJECT_DEF, 0, 1, 0 },
    
    [IDX( FlowVerifyPercentage )] =
    { NULL, DVCB_CHECK_COERCE, 0, 200, 0 },    
    
    [IDX( Pressure_2_Offset )] =
    { systemPressure_2_OffsetCallback, DVCB_CHECK_COERCE, 0, 0x7FFFFFFF, 0x01900000},

    [IDX( Pressure_2_Slope )] =
    { systemPressure_2_SlopeCallback, DVCB_CHECK_COERCE, 0, 0x7FFFFFFF, 0x00003728},
    
    [IDX( TankFillHeight )] =
    { NULL, DVCB_CHECK_REJECT_DEF, 0, TANK_HEIGHT_MAX, 0 },            // 144.00
    
    [IDX( TankFillPressure )] =
    { NULL, DVCB_CHECK_COERCE, 0, 99999, 0 },
    
    [IDX( TankFillVoltage )] =
    { NULL, DVCB_CHECK_COERCE, 0, 0x7FFFFFFF, 0 },
    
    [IDX( TankFillVolume )] =
    { NULL, DVCB_CHECK_REJECT_DEF, 0, 0xFFFFFFFF, 10000 },    // 100.00
    
    [IDX( TankSensorHeight )] =
    { NULL, DVCB_CHECK_REJECT_DEF, 0, TANK_HEIGHT_MAX, 0 },             // 144.00
    
    [IDX( TankSensorVolume )] =
    { NULL, DVCB_CHECK_REJECT_DEF, 0, 0x7FFFFFFF, 0 },
    
    [IDX( TankSensorPosition )] =
    { NULL, DVCB_CHECK_REJECT_DEF, 0, NUMBER_SENSOR_POSITIONS-1, POSITION_ABOVE_TANK },
    
    [IDX( TankSetupComplete )] =
    { NULL, DVCB_CHECK_REJECT_DEF, 0, 1, 0 },    
    
    [IDX( MaxTankVolume )] =
    { NULL, DVCB_CHECK_REJECT_DEF, 0, 0x7FFFFFFF, MAX_TANK_DEF },    // 100.00
    
    [IDX( PressureVSHeightRadius_x10000 )] =
    { NULL, DVCB_CHECK_REJECT_DEF, 0, 0x7FFFFFFF, 0 },
    
    [IDX( FlowRateLowmASetpoint )] =
    { NULL, DVCB_CHECK_REJECT_DEF, MIN_FLOW_RATE_GAL, MAX_FLOW_RATE_GAL, 1000 },
    
    [IDX( FlowRateHighmASetpoint )] =
    { NULL, DVCB_CHECK_REJECT_DEF, MIN_FLOW_RATE_GAL, MAX_FLOW_RATE_GAL, 1000 },
    
    [IDX( AnalogInControl )] =
    { NULL, DVCB_CHECK_REJECT_DEF, 0, 1, 0 },    // update the max if the number of ANALOG IN controllables changes
    
    [IDX( HighmASetpoint )] =
    { NULL, DVCB_CHECK_REJECT_DEF, 400, 2000, 2000 },
    
    [IDX( LowmASetpoint )] =
    { NULL, DVCB_CHECK_REJECT_DEF, 400, 2000, 400 },
    
    [IDX( ModbusParity )] =
    { NULL, DVCB_CHECK_COERCE, 0, 2, 0 },
    
    [IDX( ModbusStopBits )] =
    { NULL, DVCB_CHECK_COERCE, 0, 1, 0 },    
    
    [IDX( ModbusCommsEnable )] =
    { NULL, DVCB_CHECK_REJECT_DEF, 0, 1, 0 },
    
    [IDX( AnalogOutControl )] =
    { NULL, DVCB_CHECK_REJECT_DEF, 0, 1, 0 },
    
    [IDX( AoutOutputType )] =
    { NULL, DVCB_CHECK_REJECT_DEF, 0, 0xFFFFFFFF, 0 },
    
    [IDX( AoutMinOut )] =
    { NULL, DVCB_CHECK_REJECT_DEF, 0, 0xFFFFFFFF, 0 },
    
    [IDX( AoutMaxOut )] =
    { NULL, DVCB_CHECK_REJECT_DEF, 0, 0xFFFFFFFF, 0 },      // 0V default ensures 0V out in normal operation (Analog out == OFF)
    
    [IDX( TempControl )] =
    { NULL, DVCB_CHECK_REJECT_DEF, 0, 3, 0},
    
    [IDX( TempSetpoint )] =
    { NULL, DVCB_CHECK_REJECT_DEF, 0, 0xFFFFFFFF, 0 },
    
    [IDX( MtrProtectionEnabled )] =
    { NULL, DVCB_CHECK_REJECT_DEF, 0, 1, 0 },      
    
    [IDX( MtrVoltage )] =
    { NULL, DVCB_CHECK_REJECT_DEF, 0, 3, 0 },     
    
    [IDX( MtrSelection )] =
    { NULL, DVCB_CHECK_REJECT_DEF, 0, 5, 0 },      
    
    [IDX( MaxMtrCurrentmV )] =
    { NULL, DVCB_CHECK_REJECT_DEF, 0, 5000, 700 },
    
    [IDX( CycleSwitchEnable )] =
    { NULL, DVCB_CHECK_REJECT_DEF, 0, 1, 1 },
    
    [IDX( RpmNameplate )] =
    { NULL, DVCB_CHECK_REJECT_DEF, 0, 999, DEFAULT_PUMP_RPM },
};
static const DVCB_segmentSet_t sDvcbSetupSegment = {
    .baseAddr     = DVA17G721_gSetup_BASE,
    .segmentLen   = DVA17G721_gSetup_LEN,
    .segmentCount = 1,
    .dvcbArray    = sDvcbSetup,
    .dvarCount    = sizeof (DVS17G721_gSetup_t) / sizeof(DistVarType),
};


// *****************************************************************************
// * PUBLIC FUNCTIONS
// *****************************************************************************

sint8 DVSEG_17G721_SETUP_Initialize( bool resetDefaults )
{
    DVarErrorCode dvarError = DVarError_Ok;
    sint8 rVal = 0;

    dvarError = DVAR_RegisterSegment(
                                      DVA17G721_gSetup_BASE,                      // base address
                                      sizeof (DVS17G721_gSetup_t) / sizeof (DistVarType), 		// length
                                      VARIABLE_FLAVOR_SHADOWED_EEPROM,		// flavor
                                      (DistVarType*) & gSetup,			// RAM location
                                      EE_gSetup_BASE );				// eeprom offset

    dvarError |= DVAR_RegisterOwnership(
                                         DVA17G721_gSetup_BASE,			// base address
                                         sizeof (DVS17G721_gSetup_t) / sizeof (DistVarType), 		// length
                                         Callback,					// calback function
                                         0 );					// lint !e655 bit-wise operation ok // Auto-broadcast period

    assert( !dvarError );

    if ( resetDefaults )
    {
        dvarError = (DVarErrorCode)DVCB_HELPER_SetDefaults( &sDvcbSetupSegment );
    }
    else
    {
        dvarError = (DVarErrorCode)DVCB_HELPER_Validate( &sDvcbSetupSegment );
    }

    assert( !dvarError );

    if( dvarError )	
    {
        rVal = -1;
    }

    return rVal;
}

DistVarType DVSEG_17G721_SETUP_GetDvar( DistVarID dvar_id )
{
    DistVarType rtnval = 0;

    if( dvar_id >= DVA17G721_gSetup_BASE )
    {
        dvar_id = dvar_id - DVA17G721_gSetup_BASE;

        if( dvar_id < (sizeof (gSetup) / sizeof (DistVarType)) )
        {
            rtnval = *((DistVarType*) & gSetup + dvar_id);
        }
    }

    return rtnval;
}

// *****************************************************************************
// * PRIVATE FUNCTION
// *****************************************************************************

static DistVarType Callback( DistVarID id, DistVarType oldVal, DistVarType newVal )
{
    DistVarType rVal = 0;

    //route all DVar callback traffic through DVCB_Helper
    if ( DVCB_HELPER_Callback( &sDvcbSetupSegment, id, oldVal, newVal, &rVal ) )
    {
        return 0xffffffff;
    }

    return rVal;
}

bool DVSEG_17G721_SETUP_MagicNumberMismatch( void )
{
    uint32 val;
    bool retVal = TRUE;

    (void)EEPROM_ReadLongword( EE_gSetup_BASE, &val );

    retVal = (val != MAGIC_NUMBER_AND_VERSION);

    return retVal;
}

static DistVarType systemPressure_1_OffsetCallback( DistVarID id,
                                                 DistVarType oldVal,
                                                 DistVarType newVal,
                                                 DistVarType defVal )
{
    IOErrorCode_t ioerror = IOError_Ok;

    if( newVal != oldVal )
    {
        ioerror = IOHARDWARE_PressureSensorCal( IOHARDWARE_PRESSURESENSOR_A, newVal, gSetup.Pressure_1_Slope );
        if( ioerror != IOError_Ok )
        {
            return defVal;
        }
        else
            return newVal;
    }
    else
    {
        return oldVal;
    }
}

static DistVarType systemPressure_2_OffsetCallback( DistVarID id,
                                                 DistVarType oldVal,
                                                 DistVarType newVal,
                                                 DistVarType defVal )
{
    IOErrorCode_t ioerror = IOError_Ok;

    if( newVal != oldVal )
    {
        ioerror = IOHARDWARE_PressureSensorCal( IOHARDWARE_PRESSURESENSOR_B, newVal, gSetup.Pressure_2_Slope );
        if( ioerror != IOError_Ok )
        {
            return defVal;
        }
        else
            return newVal;
    }
    else
    {
        return oldVal;
    }
}

static DistVarType systemPressure_1_SlopeCallback( DistVarID id,
                                                DistVarType oldVal,
                                                DistVarType newVal,
                                                DistVarType defVal )
{
    IOErrorCode_t ioerror = IOError_Ok;

    if( newVal != oldVal )
    {
        ioerror = IOHARDWARE_PressureSensorCal( IOHARDWARE_PRESSURESENSOR_A, gSetup.Pressure_1_Offset, newVal );
        if( ioerror != IOError_Ok )
        {
            return defVal;
        }
        else
            return newVal;
    }
    else
    {
        return oldVal;
    }
}

static DistVarType systemPressure_2_SlopeCallback( DistVarID id,
                                                DistVarType oldVal,
                                                DistVarType newVal,
                                                DistVarType defVal )
{
    IOErrorCode_t ioerror = IOError_Ok;

    if( newVal != oldVal )
    {
        ioerror = IOHARDWARE_PressureSensorCal( IOHARDWARE_PRESSURESENSOR_B, gSetup.Pressure_2_Offset, newVal );
        if( ioerror != IOError_Ok )
        {
            return defVal;
        }
        else
            return newVal;
    }
    else
    {
        return oldVal;
    }
}

static IOErrorCode_t IOHARDWARE_PressureSensorCal( IOHARDWARE_PRESSURESENSOR_t port, s16d16_t zero_cal_mVpV_s16d16, s16d16_t sensitivity_cal_mVpV_s16d16 )
{
    IOErrorCode_t ioerror = IOError_Ok;

    if( (port >= PS_NUM_PORTS) )
    {
        return IOError_OutOfRange;
    }    
    
    if (port == PS_PORT_A)
    {
        if( zero_cal_mVpV_s16d16 > (s16d16_t)0x00140000 )
        {
            zero_cal_mVpV_s16d16 = (s16d16_t)0x00140000;
            ioerror = IOError_OutOfRange;
        }
        else if( zero_cal_mVpV_s16d16 < (s16d16_t)0xFFEC0000 )
        {
            zero_cal_mVpV_s16d16 = (s16d16_t)0xFFEC0000 ;
            ioerror = IOError_OutOfRange;
        }

        if( sensitivity_cal_mVpV_s16d16 <= (sint32)0 )
        {
            // Sensitivity must be greater than 0 to avoid divide by zero errors
            sensitivity_cal_mVpV_s16d16 = 0x00230000;
            ioerror = IOError_OutOfRange;
        }
    }

    // Need to recalibrate even if errors are present because values may have changed to default values
    ioerror |= IN_Pressure_Cal( port, zero_cal_mVpV_s16d16, sensitivity_cal_mVpV_s16d16 );

    return ioerror;
}

IOrtn_mV_to_psi_int_t IOHARDWARE_PressureSensorRef (IOHARDWARE_PRESSURESENSOR_t port)
{
    IOrtn_mV_to_psi_int_t referencePressure_s16d16;
    
    // Set offset to the default
    (void)IN_Pressure_Cal(IOHARDWARE_PRESSURESENSOR_B, pressure_420420_definition.zeroBarOffset_mV_s16d16, gSetup.Pressure_2_Slope);
    
    referencePressure_s16d16 = GetReferencePSIAveraged(IOHARDWARE_PRESSURESENSOR_B);
    printf("*************refPSI*****************************: %d\r\n", referencePressure_s16d16.psi_int);
    
    // Restore offset to its previous value
    (void)IN_Pressure_Cal(IOHARDWARE_PRESSURESENSOR_B, gSetup.Pressure_2_Offset, gSetup.Pressure_2_Slope);    
    
    return referencePressure_s16d16;
}

static IOrtn_mV_to_psi_int_t GetReferencePSIAveraged(PressureSensor_Port_t port)
{
    uint8 avg_cnt = 0;
    uint32 avg_accum_ref_psi = 0;
    uint32 avg_accum_ref_mv = 0;    
    uint32 ReferencePsi = 0;
    uint32 ReferenceMv = 0;    
    uint32 ReferencePsiAveraged = 0;
    uint32 ReferenceMvAveraged = 0;
    IOrtn_mV_to_psi_s16d16_t psi_to_mV_s16d16;
    IOrtn_mV_to_psi_int_t retVal;
    uint8 i = 0;
    
    retVal.error = IOError_UNDEFINED;

    for (i=0; i < 10; i++)
    {
        psi_to_mV_s16d16 = GetPressurePSI(port);

        if (psi_to_mV_s16d16.error == IOError_Ok)
        {
            avg_cnt++;
            retVal.error = psi_to_mV_s16d16.error;  // Returns OK if there was at least one good pressure reading 
            
            ReferenceMv = FixedPointToInteger(psi_to_mV_s16d16.mV_s16d16, DECIMAL_PLACE_THREE);
            ReferencePsi = FixedPointToInteger(psi_to_mV_s16d16.psi_s16d16, DECIMAL_PLACE_THREE);

            avg_accum_ref_mv += ReferenceMv;
            avg_accum_ref_psi += ReferencePsi;
            
            DEBUG_PRINT_STRING(DBUG_TANK, "mV, Psi: ");
            DEBUG_PRINT_UNSIGNED_DECIMAL(DBUG_TANK, ReferenceMv);
            DEBUG_PRINT_STRING(DBUG_TANK, ", ");
            DEBUG_PRINT_UNSIGNED_DECIMAL(DBUG_TANK, ReferencePsi);
            DEBUG_PRINT_STRING(DBUG_TANK, ", \r\n");
        }
        else
        {
            DEBUG_PRINT_STRING(DBUG_TANK, "\r\nPSI Error: ");
            DEBUG_PRINT_UNSIGNED_DECIMAL(DBUG_TANK, psi_to_mV_s16d16.error);
            DEBUG_PRINT_STRING(DBUG_TANK, ", PSI: ");
            DEBUG_PRINT_UNSIGNED_DECIMAL(DBUG_TANK, FixedPointToInteger(psi_to_mV_s16d16.psi_s16d16, DECIMAL_PLACE_THREE));
            DEBUG_PRINT_STRING(DBUG_TANK, ", V: ");
            DEBUG_PRINT_UNSIGNED_DECIMAL(DBUG_TANK, FixedPointToInteger(psi_to_mV_s16d16.mV_s16d16, DECIMAL_PLACE_ONE));
            DEBUG_PRINT_STRING(DBUG_TANK, "\r\n");
        }
        delay(200);
    }
    if (avg_cnt > 0)
    {
        ReferenceMvAveraged = avg_accum_ref_mv / avg_cnt;
        ReferencePsiAveraged = avg_accum_ref_psi / avg_cnt;
    }

    retVal.mV_int = ReferenceMvAveraged;
    retVal.psi_int = ReferencePsiAveraged;
    
    return retVal;
}

// **********************************************************************************************************
// CalculateFluidDensity
// **********************************************************************************************************
bool CalculateFluidDensity(void)
{
    bool retVal = FALSE;
    uint32 fluidDensity = 0;
    sint32 height = 0;

    if (gSetup.TankSensorPosition == POSITION_ABOVE_TANK)
    {
        height = gSetup.TankFillHeight - gSetup.TankSensorHeight;
    }
    else
    {
        height = gSetup.TankFillHeight + gSetup.TankSensorHeight;
    }
    
    if ((height > TANK_HEIGHT_MIN) && (height < TANK_HEIGHT_MAX))
    {
        // PSI = 0.433 * ((HEIGHT")/(12"/Ft))*((Fluid Density PSI/Ft)/(8.33 PSI/Ft))
        fluidDensity = integerDivideRound(gSetup.TankFillPressure * TANK_VOLUME_SCALE_FACTOR, (height * 100));
     
        if ((fluidDensity < FLUID_DENSITY_MIN) || (fluidDensity > FLUID_DENSITY_MAX))
        {
            // Out of range, clamp to min or max & return error
            fluidDensity = clampValue(fluidDensity, FLUID_DENSITY_MAX, FLUID_DENSITY_MIN);
            (void)DVAR_SetPointLocal(DVA17G721_SS(gSetup, FluidDensity), fluidDensity);
        }
        else
        {
            (void)DVAR_SetPointLocal(DVA17G721_SS(gSetup, FluidDensity), fluidDensity);
            retVal = TRUE;
        }
    }
    else
    {
        (void)DVAR_SetPointLocal(DVA17G721_SS(gSetup, FluidDensity), FLUID_DENSITY_DEF);
        DEBUG_PRINT_STRING(DBUG_TANK, "Restore density default, height out of range\r\n");
    }
    return retVal;
}

// **********************************************************************************************************
// CalculatePressureSensorOffset
// **********************************************************************************************************
bool CalculatePressureSensorOffset(void)
{
    bool retVal = FALSE;
    sint32 fittingPressure_psi_s16d16 = 0;
    sint32 fittingOffset_mV_s16d16 = 0;
    sint64 tempval = 0;
    sint32 slope_psi_s16d16 = bar_s16d16_to_psi_s16d16(pressure_420420_definition.scaleFactor_bar_per_mV_s16d16);
    sint32 offsetCal_mV_s16d16 = pressure_420420_definition.zeroBarOffset_mV_s16d16;
    
    if ((slope_psi_s16d16 > 0) && (offsetCal_mV_s16d16 > 0))
    {
        // Edge cases & incorrect setups with really high sensor heights could return a value > sint32, so check for it even though it's unlikely
        tempval = integerDivideRound(gSetup.TankSensorHeight * gSetup.FluidDensity * 100, TANK_VOLUME_SCALE_FACTOR);  // returns PSI x 1000
        if (tempval > SHRT_MAX)
        {
            // Limited to SHRT_MAX so conversion to s16d16 doesn't overflow. Fitting psi should never be this high in reality.
            tempval = USHRT_MAX;
        }
        fittingPressure_psi_s16d16 = IntegerToFixedPoint(tempval, DECIMAL_PLACE_NONE);
        
        // mV = 1/slope(PSI), and 1/slope = /3.124893 = 0.32
        // in s16d16 format, this = 0x51EC
        tempval = ((sint64)fittingPressure_psi_s16d16 * (0x51ECULL)) >> 16;
        
        if ((tempval > LONG_MIN) && (tempval < LONG_MAX))
        {
            fittingOffset_mV_s16d16 = (uint32)tempval;
            
            if (gSetup.TankSensorPosition == POSITION_ABOVE_TANK)
            {
                offsetCal_mV_s16d16 -= fittingOffset_mV_s16d16;
            }
            else
            {
                offsetCal_mV_s16d16 += fittingOffset_mV_s16d16;
            }
            
            if ((offsetCal_mV_s16d16 < OFFSET_MIN) || (offsetCal_mV_s16d16 > OFFSET_MAX))
            {
                // Out of range, restore default and return error
                (void)DVAR_SetPointLocal_wCallback(DVA17G721_SS(gSetup, Pressure_2_Offset), (DistVarType)pressure_420420_definition.zeroBarOffset_mV_s16d16);
                DEBUG_PRINT_STRING(DBUG_TANK, "Restore offset default, offset out of range\r\n");
            }
            else
            {
                (void)DVAR_SetPointLocal_wCallback(DVA17G721_SS(gSetup, Pressure_2_Offset), (DistVarType)offsetCal_mV_s16d16);
                retVal = TRUE;
            }
        }
        else
        {
            // fitting pressure calculation error, restore default & return error
            (void)DVAR_SetPointLocal_wCallback(DVA17G721_SS(gSetup, Pressure_2_Offset), pressure_420420_definition.zeroBarOffset_mV_s16d16);
            DEBUG_PRINT_STRING(DBUG_TANK, "Restore offset default, fitting pressure error\r\n");
        }
    }
    printf("offsetCal s16d16: %x\r\n", offsetCal_mV_s16d16);
    printf("offsetCal int: %d\r\n", FixedPointToInteger(offsetCal_mV_s16d16, DECIMAL_PLACE_ONE));
    return retVal;
}

void Init_Pressure_Transducer(TANK_TYPES_t TankType)
{   
    // Line Pressure Transducer
    (void)IN_Pressure_Init(IOHARDWARE_PRESSURESENSOR_A, &pressure_15M669_definition);
    (void)IN_Pressure_Cal(IOHARDWARE_PRESSURESENSOR_A, gSetup.Pressure_1_Offset, gSetup.Pressure_1_Slope);
    
    // Enable tank monitor transducer.  0-10V input and output.  Set analog output to 2500mV = 10V
    (void)OUT_Digital_Latch_Set(IOPIN_EXT_CNTL_INPUT_SELECT, NOT_ASSERTED); // Low = current input, high = voltage input (not populated)
    (void)OUT_Digital_Latch_Set(IOPIN_FEEDBACK_OUTPUT_SELECT, ASSERTED);    // Low = current output, high = voltage output
    (void)D2A_Output_Set(D2A_MODULE_0_CHB, (D2A_FULLSCALE * 2500) / 2500);

    (void)IN_Pressure_Init(IOHARDWARE_PRESSURESENSOR_B, &pressure_420420_definition);
    (void)IN_Pressure_Cal(IOHARDWARE_PRESSURESENSOR_B, gSetup.Pressure_2_Offset, gSetup.Pressure_2_Slope);
}

void Restore_4to20mA_Transducer_Defaults(void)
{   
    (void)DVAR_SetPointLocal( DVA17G721_SS( gSetup, Pressure_2_Slope ), pressure_420420_definition.sensitivity_cal_mVpV_s16d16 );
    (void)DVAR_SetPointLocal( DVA17G721_SS( gSetup, Pressure_2_Offset), pressure_420420_definition.zeroBarOffset_mV_s16d16 );
    (void)IN_Pressure_Cal(IOHARDWARE_PRESSURESENSOR_B, gSetup.Pressure_2_Offset, gSetup.Pressure_2_Slope);
}

