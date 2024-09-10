//! \file	in_pressure.c
//! \brief Functional Module for EFCM Analog Voltage Inputs
//!
//! Copyright 2012
//! Graco Inc., Minneapolis, MN
//! All Rights Reserved
//!
//! \b DESCRIPTION:
//!    This module will convert the the Analog Voltage Inputs
//!    on the EFCM (Enhanced Fluid Control Module) component
//!    in Pressure Inputs

#include "in_pressure.h"
#include "in_voltage.h"
#include "limits.h"
#include "debug.h"
#include "units_pressure.h"
#include "dvseg_17G721_setup.h"
#include "stdio.h"
#include "inttypes.h"
#include "debug_app.h"
#include "CountDigit.h"
#include "stdio.h"

// ***************************************************
// * TYPEDEFS & STRUCTURES
// ***************************************************


// ***************************************************
// * PRIVATE (STATIC) VARIABLES
// ***************************************************

static PressureSensor_t mPressureSensors[PS_NUM_PORTS];

// ***************************************************
// * PRIVATE FUNCTION PROTOTYPES
// ***************************************************


// ***************************************************
// * CONSTANTS
// ***************************************************


// ***************************************************
// * MACROS
// ***************************************************

#define EXCITATION_VOLTAGE                 (5)
#define PRESSURE_SENSOR_CHANNEL_DIFF       (0)
#define PRESSURE_SENSOR_CHANNEL_1TO5V      (4)
#define PRESSURE_SENSOR_CHANNEL_4TO20MA    (5)

// ***************************************************
// * PUBLIC FUCTIONS
// ***************************************************

IOErrorCode_t IN_Pressure_Init( PressureSensor_Port_t port, const PressureSensor_t *pressureSensor )
{
    IOErrorCode_t rVal = IOError_UNDEFINED;

	if( port >= PS_NUM_PORTS)
	{
		rVal = IOError_InvalidPinName;
	}   
    else if( pressureSensor->fullScale_bar_s16d16 == 0 ||
         pressureSensor->sensitivity_cal_mVpV_s16d16 == 0 ) // Both these values must be non-zero
    {
        rVal = IOError_InvalidPinConfiguration;
    }    
    else
    {
        mPressureSensors[port] = *pressureSensor;
        
        rVal = IOError_Ok;        
    }

    return rVal;
}

IOErrorCode_t IN_Pressure_Cal( PressureSensor_Port_t port, s16d16_t zero_cal_mVpV_s16d16, s16d16_t sensitivity_cal_mVpV_s16d16 )
{
    IOErrorCode_t rVal = IOError_UNDEFINED;

    if( port >= PS_NUM_PORTS )
    {
        rVal = IOError_InvalidPinName;
    }
    else if( mPressureSensors[port].fullScale_bar_s16d16 == 0 ) // Make sure the sensor has been initialized
    {
        rVal = IOError_InvalidPinConfiguration;
    }
    else if( sensitivity_cal_mVpV_s16d16 == 0 )
    {
        rVal = IOError_OutOfRange;
    }
    else
    {    
        if (port == PS_PORT_A)
        {
            mPressureSensors[port].zeroBarOffset_mV_s16d16 = zero_cal_mVpV_s16d16 * EXCITATION_VOLTAGE;
            mPressureSensors[port].scaleFactor_bar_per_mV_s16d16 =
                (sint32)(((sint64)mPressureSensors[port].fullScale_bar_s16d16 << 16) /
                         ((sint64)(sensitivity_cal_mVpV_s16d16) * EXCITATION_VOLTAGE));
            mPressureSensors[port].zero_cal_mVpV_s16d16 = zero_cal_mVpV_s16d16;
            mPressureSensors[port].sensitivity_cal_mVpV_s16d16 = sensitivity_cal_mVpV_s16d16;
        }
        else
        {
            mPressureSensors[port].zeroBarOffset_mV_s16d16 = zero_cal_mVpV_s16d16;
            mPressureSensors[port].scaleFactor_bar_per_mV_s16d16 = sensitivity_cal_mVpV_s16d16;
            mPressureSensors[port].zero_cal_mVpV_s16d16 = zero_cal_mVpV_s16d16;
            mPressureSensors[port].sensitivity_cal_mVpV_s16d16 = sensitivity_cal_mVpV_s16d16;
        }            
    
        rVal = IOError_Ok; 
    }

    return rVal;
}

IOrtn_mV_to_bar_s16d16_t IN_Pressure_Get_Diff( PressureSensor_Port_t port )
{
    IOrtn_mV_to_bar_s16d16_t rtnval;
    IOrtn_mV_s16d16_t DM_mV_s16d16;
    sint64 pressure_bar_s48d16;
    
    rtnval.mV_s16d16 = 0;
    rtnval.bar_s16d16 = 0;
    rtnval.error = IOError_UNDEFINED;
	
    DM_mV_s16d16 = IN_Voltage_Pressure_Get_Diff( PRESSURE_SENSOR_CHANNEL_DIFF );

    if( DM_mV_s16d16.error != IOError_Ok )
    {
        rtnval.error = DM_mV_s16d16.error;
    }
    else if( mPressureSensors[port].fullScale_bar_s16d16 == 0 ) // Make sure the sensor has been initialized
    {
        rtnval.error = IOError_InvalidPinConfiguration;
    }
    else
    {
        rtnval.mV_s16d16 = DM_mV_s16d16.mV_s16d16;

        if (DM_mV_s16d16.mV_s16d16 >= mPressureSensors[port].zeroBarOffset_mV_s16d16)
        {
            pressure_bar_s48d16 = ((sint64)(DM_mV_s16d16.mV_s16d16 - mPressureSensors[port].zeroBarOffset_mV_s16d16) *
                                   (sint64)mPressureSensors[port].scaleFactor_bar_per_mV_s16d16) >> 16;

            if( pressure_bar_s48d16 > LONG_MAX )
            {
                rtnval.bar_s16d16 = LONG_MAX;
                rtnval.error = IOError_OutOfRange;
            }
            else if( pressure_bar_s48d16 < LONG_MIN )
            {
                rtnval.bar_s16d16 = LONG_MIN;
                rtnval.error = IOError_OutOfRange;
            }
            else if( pressure_bar_s48d16 >= mPressureSensors[port].fullScale_bar_s16d16 )
            {
                rtnval.bar_s16d16 = mPressureSensors[port].fullScale_bar_s16d16;
                rtnval.error = IOError_Ok;
            }
            else
            {
                rtnval.bar_s16d16 = (bar_s16d16_t)pressure_bar_s48d16;
                rtnval.error = IOError_Ok;
            }
        }
        else
        {
            rtnval.error = IOError_OutOfRange;
        }
    }

    return rtnval;
}

IOrtn_mV_to_psi_s16d16_t GetPressurePSI( PressureSensor_Port_t port )
{
    IOrtn_mV_to_bar_s16d16_t Pressure_s16d16_Bar;
    IOrtn_mV_to_psi_s16d16_t rtnval;
    sint32 Pressure_s16d16_PSI;

    rtnval.mV_s16d16 = 0;
    rtnval.psi_s16d16 = 0;
    rtnval.error = IOError_UNDEFINED;
    Pressure_s16d16_Bar.error = IOError_UNDEFINED;
    
    if( port >= PS_NUM_PORTS )
    {
        rtnval.error = IOError_InvalidPinName;
    }
    
    if (port == PS_PORT_A)
    {
        Pressure_s16d16_Bar = IN_Pressure_Get_Diff(PS_PORT_A);
    }
    else if (port == PS_PORT_B)
    {
        Pressure_s16d16_Bar = IN_Pressure_Get_4to20mA(PS_PORT_B);
    }
    rtnval.mV_s16d16 = Pressure_s16d16_Bar.mV_s16d16;   // Return voltage, regardless of any pressure errors
   
    if (Pressure_s16d16_Bar.error != IOError_Ok)
    {
        rtnval.error = Pressure_s16d16_Bar.error;
    }
    else
    {  
        Pressure_s16d16_PSI = bar_s16d16_to_psi_s16d16(Pressure_s16d16_Bar.bar_s16d16);

        if (port == PS_PORT_A)   
        {
            rtnval.psi_s16d16 = Pressure_s16d16_PSI >> 16;
        }
        else
        {
            // Keep decimal points for tank monitor for increased resolution
            rtnval.psi_s16d16 = Pressure_s16d16_PSI;    
        }
        
        rtnval.error = IOError_Ok;        
    }
    return rtnval;
}

IOrtn_mV_to_bar_s16d16_t GetPressureBar ( PressureSensor_Port_t port )
{
    IOrtn_mV_to_bar_s16d16_t Pressure_s16d16_Bar;
    IOrtn_mV_to_bar_s16d16_t rtnval;
    
    rtnval.mV_s16d16 = 0;
    rtnval.bar_s16d16 = 0;
    rtnval.error = IOError_UNDEFINED;
    Pressure_s16d16_Bar.error = IOError_UNDEFINED;
    
    if( port >= PS_NUM_PORTS )
    {
        rtnval.error = IOError_InvalidPinName;
    }    

    if (port == PS_PORT_A)
    {
        Pressure_s16d16_Bar = IN_Pressure_Get_Diff(PS_PORT_A);
    }
    else if (port == PS_PORT_B)
    {
        Pressure_s16d16_Bar = IN_Pressure_Get_4to20mA(PS_PORT_B);
    }
    rtnval.mV_s16d16 = Pressure_s16d16_Bar.mV_s16d16;   // Return voltage, regardless of any pressure errors

    if(Pressure_s16d16_Bar.error != IOError_Ok)
    {
        rtnval.error = Pressure_s16d16_Bar.error;
    }
    else
    {
        if (port == PS_PORT_A)
        {  
            rtnval.bar_s16d16 = Pressure_s16d16_Bar.bar_s16d16 >> 16;          
        }
        else
        {
            // Keep decimal points for tank monitor for increased resolution
            rtnval.bar_s16d16 = Pressure_s16d16_Bar.bar_s16d16;
        }
        
        rtnval.error = IOError_Ok;
    }
    
    return rtnval;
}

IOrtn_mV_to_bar_s16d16_t IN_Pressure_Get_1to5V( PressureSensor_Port_t port )
{
    IOrtn_mV_to_bar_s16d16_t rtnval;
    IOrtn_mV_s16d16_t DM_mV_s16d16;
    sint64 pressure_bar_s48d16;
    
    rtnval.mV_s16d16 = 0;
    rtnval.bar_s16d16 = 0;
    rtnval.error = IOError_UNDEFINED;
	
    DM_mV_s16d16 = IN_Voltage_Pressure_Get_1to5V( PRESSURE_SENSOR_CHANNEL_1TO5V );
            
    if( DM_mV_s16d16.error != IOError_Ok )
    {
        rtnval.error = DM_mV_s16d16.error;
    }
    else if( mPressureSensors[port].fullScale_bar_s16d16 == 0 ) // Make sure the sensor has been initialized
    {
        rtnval.error = IOError_InvalidPinConfiguration;
    }
    else
    {
        rtnval.mV_s16d16 = DM_mV_s16d16.mV_s16d16;
        
        if (DM_mV_s16d16.mV_s16d16 >= mPressureSensors[port].zeroBarOffset_mV_s16d16)
        {
            pressure_bar_s48d16 = (((sint64)(DM_mV_s16d16.mV_s16d16 - mPressureSensors[port].zeroBarOffset_mV_s16d16) *
                                   (sint64)mPressureSensors[port].scaleFactor_bar_per_mV_s16d16) / 1000) >> 16;        

            if( pressure_bar_s48d16 > LONG_MAX )
            {
                rtnval.bar_s16d16 = LONG_MAX;
                rtnval.error = IOError_OutOfRange;
            }
            else if( pressure_bar_s48d16 < LONG_MIN )
            {
                rtnval.bar_s16d16 = LONG_MIN;
                rtnval.error = IOError_OutOfRange;
            }
            else if (pressure_bar_s48d16 >= mPressureSensors[port].fullScale_bar_s16d16)
            {
                rtnval.bar_s16d16 = mPressureSensors[port].fullScale_bar_s16d16;
                rtnval.error = IOError_Ok;
            }            
            else
            {
                rtnval.bar_s16d16 = (bar_s16d16_t)pressure_bar_s48d16;
                rtnval.error = IOError_Ok;
            }
        }
        else
        {
            rtnval.error = IOError_OutOfRange;
        } 
    }

    return rtnval;
}

IOrtn_mV_to_bar_s16d16_t IN_Pressure_Get_4to20mA( PressureSensor_Port_t port )
{
    IOrtn_mV_to_bar_s16d16_t rtnval;
    IOrtn_mV_s16d16_t DM_mV_s16d16;
    sint64 pressure_bar_s48d16 = 0;
    
    rtnval.mV_s16d16 = 0;
    rtnval.bar_s16d16 = 0;
    rtnval.error = IOError_UNDEFINED;
	
    DM_mV_s16d16 = IN_Voltage_Pressure_Get_4to20mA( PRESSURE_SENSOR_CHANNEL_4TO20MA );
    rtnval.mV_s16d16 = DM_mV_s16d16.mV_s16d16;      // Return the value, regardless of error
            
    if( DM_mV_s16d16.error != IOError_Ok )
    {
        rtnval.error = DM_mV_s16d16.error;
        DEBUG_PRINT_STRING(DBUG_MV, "mV error\r\n");
    }
    else if( mPressureSensors[port].fullScale_bar_s16d16 == 0 ) // Make sure the sensor has been initialized
    {
        rtnval.error = IOError_InvalidPinConfiguration;
        DEBUG_PRINT_STRING(DBUG_MV, "Invalid full scale value\r\n");        
    }
    else
    {	
        if (DM_mV_s16d16.mV_s16d16 >= mPressureSensors[port].zeroBarOffset_mV_s16d16)
        {
            pressure_bar_s48d16 = (((sint64)(DM_mV_s16d16.mV_s16d16 - mPressureSensors[port].zeroBarOffset_mV_s16d16) *
                                   (sint64)mPressureSensors[port].scaleFactor_bar_per_mV_s16d16) / 1000) >> 16;
            
            if (g_DebugMask & DBUG_MV)
            {
                printf("DM_mV: %x\r\n", DM_mV_s16d16.mV_s16d16);
                printf("zeroBarOffset: %x\r\n", mPressureSensors[port].zeroBarOffset_mV_s16d16);
                printf("Bar s16d16: 0x%" PRIx64 "\r\n", pressure_bar_s48d16);
            }
                
            DEBUG_PRINT_STRING(DBUG_MV, "mV offset:");
            DEBUG_PRINT_UNSIGNED_DECIMAL(DBUG_MV, FixedPointToInteger(mPressureSensors[port].zeroBarOffset_mV_s16d16, NO_DECIMAL_POINT));
            DEBUG_PRINT_STRING(DBUG_MV, "\r\n");

            if( pressure_bar_s48d16 > LONG_MAX )
            {
                rtnval.bar_s16d16 = LONG_MAX;
                rtnval.error = IOError_OutOfRange;
                DEBUG_PRINT_STRING(DBUG_MV, "PSI out of range, too high\r\n");
            }
            else if( pressure_bar_s48d16 < LONG_MIN )
            {
                rtnval.bar_s16d16 = LONG_MIN;
                rtnval.error = IOError_OutOfRange;
                DEBUG_PRINT_STRING(DBUG_MV, "PSI out of range, too low\r\n");
            }
			else if (pressure_bar_s48d16 >= mPressureSensors[port].fullScale_bar_s16d16)
			{
				rtnval.bar_s16d16 = mPressureSensors[port].fullScale_bar_s16d16;
				rtnval.error = IOError_Ok;
			}			
            else
            {
                rtnval.bar_s16d16 = (bar_s16d16_t)pressure_bar_s48d16;
                rtnval.error = IOError_Ok;
            }
        }
        else
        {
            rtnval.error = IOError_OutOfRange;
            DEBUG_PRINT_STRING(DBUG_MV, "MV < Offset Error\r\n");
        } 
    }

    return rtnval;
}

IOrtn_mV_s16d16_t GetPressureOffset_mV(void)
{
    IOrtn_mV_s16d16_t DM_mV_s16d16;
    const sint32 TransducerVoltsMax = 5000;
    
    DM_mV_s16d16.mV_s16d16 = 0;
    DM_mV_s16d16.error = IOError_UNDEFINED;
	
    DM_mV_s16d16 = IN_Voltage_Pressure_Get_4to20mA( PRESSURE_SENSOR_CHANNEL_4TO20MA );  
            
    if( DM_mV_s16d16.error == IOError_Ok )
    {
        if( DM_mV_s16d16.mV_s16d16 > TransducerVoltsMax << 16 )
        {
            DM_mV_s16d16.mV_s16d16 = TransducerVoltsMax << 16;
            DM_mV_s16d16.error = IOError_OutOfRange;
        }
        else
        {
            DM_mV_s16d16.error = IOError_Ok;
        }
    }
    return DM_mV_s16d16;    
}




