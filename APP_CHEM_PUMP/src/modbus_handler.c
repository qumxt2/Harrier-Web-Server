// modbus_handler.c

// Copyright 2015 - 2017
// Graco Inc., Minneapolis, MN
// All Rights Reserved

// This file implements modbus initialization & other generic modbus callbacks

// *****************************************************************************
// * HEADER FILES
// *****************************************************************************
#include "modbus.h"
#include "dvseg_17G721_run.h"
#include "dvseg_17G721_setup.h"
#include "dvinterface_17G721.h"
#include "Cpfuncs.h"

// *****************************************************************************
// * MACROS
// *****************************************************************************

// *****************************************************************************
// * TYPEDEFS & STRUCTURES
// *****************************************************************************

// *****************************************************************************
// * PUBLIC VARIABLES
// *****************************************************************************

extern Modbus_MapVar ModbusMap_1[];


// *****************************************************************************
// * PRIVATE VARIABLES
// *****************************************************************************
static Modbus_MapIndex Mb_mIndex;
static uint32 BaudRate[4] = {9600, 19200, 57600, 115200};

// *****************************************************************************
// * PRIVATE FUNCTION PROTOTYPES
// *****************************************************************************

// *****************************************************************************
// * PUBLIC FUNCTIONS
// *****************************************************************************

// *****************************************************************************
// * PRIVATE FUNCTIONS
// *****************************************************************************

sint8 Modbus_Init( uint8 SlaveID, uint8 Parity, uint8 StopBits )
{
    Modbus_ErrorCode eMBStatus = MB_NOERR;

    Modbus_MapVar * pMaps = ModbusMap_1;

    sint8 rVal = 0;
    static uint8 ModbusSlaveId = 247;

    if(SlaveID == 0) SlaveID = 247;
    ModbusSlaveId = SlaveID;
    
    (void)MODBUS_Disable( );
    (void)MODBUS_Close( );

    // Initialize Default Maps

    // Wait for DVARs to be ready
    (void)K_Resource_Wait(gDvarLockoutResID, 0);
    (void)K_Resource_Release(gDvarLockoutResID);

    MODBUS_Map_Initialize( &Mb_mIndex, sizeof (Mb_mIndex) );
    (void)MODBUS_Map_Add( ModbusSlaveId, pMaps );

    /*lint -e655 -e514 Suppress Warning 655: bit-wise operation uses (compatible) enum's*/
    // The following '|=' statements on the ioerror variable work because IOError_Ok is defined as 0
    // Checking every individual return value would be overkill and a waste of code space.
    
    eMBStatus |= MODBUS_Initialize( (uint8*)&ModbusSlaveId, 1, BaudRate[gSetup.ModbusBaudrateIndex],
                                    (Modbus_Parity)gSetup.ModbusParity, (uint8)(gSetup.ModbusStopBits + 1) );

    /*lint +e655 +e514 Re-enable Warning 655: bit-wise operation uses (compatible) enum's*/

    if( eMBStatus )
    {
        rVal = -1;
    }
    else
    {
        (void)MODBUS_Enable(  );
    }

    return rVal;
}

// *****************************************************************************
// * Custom MODBUS Holding Register Callback
// * brief - reject writes unless gSetup.ControlLocalRemote is set to remote(1)
// *****************************************************************************
//
//Modbus_ErrorCode MODBUS_RegHoldingCB( uint8 *pucRcvAddress, uint8 *pucRegBuffer, uint16 usAddress, uint16 uNRegs, Modbus_RegisterMode eMode )
//{
//    Modbus_ErrorCode    eStatus = MB_NOERR;
//    sint32              iRegIndex, ModbusMapSize;
//    uint32              tempBuffer = 0;
//    sint16              usNRegs = (sint16)uNRegs;
//    Modbus_MapVar*      ModbusMap;
//    DVarSearchContext   dvarSearchResult;
//
//    if( usAddress < REG_HOLDING_START )	return MB_NOREG;
//
//    // Get Modbus Map info
//    ModbusMap = MODBUS_Map_Get( *pucRcvAddress );
//    ModbusMapSize = MODBUS_Map_GetSize( *pucRcvAddress );
//    if( (void*)ModbusMap == NULL || (void*)ModbusMapSize == NULL )    return MB_NOREG;
//
//    iRegIndex = MODBUS_Map_GetRegisterIndex( *pucRcvAddress, REG_HOLDING_BASE + usAddress );
//
//    if( ( 0 <= iRegIndex ) && ( iRegIndex < ModbusMapSize ) && ( usAddress >= REG_HOLDING_START ) )
//    {
//        while( usNRegs > 0 )
//        {
//            if( eMode == MB_READ )
//            {
//                dvarSearchResult.id = (DistVarID)ModbusMap[iRegIndex].DvarId;
//                dvarSearchResult.value = 0x00;
//
//                // Check if requested register matches the table
//                if( ModbusMap[iRegIndex].ModbusAddr - REG_HOLDING_BASE != usAddress + uNRegs - usNRegs )
//                {
//                    eStatus = MB_NOREG;
//                    return eStatus;
//                }
//
//                // Check if callback is required
//                if ( ModbusMap[iRegIndex].MbRegCallback )
//                {
//                    dvarSearchResult.value = ModbusMap[iRegIndex].MbRegCallback( 0, 0, &ModbusMap[iRegIndex] );
//                }
//                else
//                {
//                    (void) DVAR_SeekLocalVariable( &dvarSearchResult, dvarSearchResult.id );
//                }
//
//                if( ModbusMap[iRegIndex].MbRegSize == MBR_UINT32 )
//                {
//                    if( ModbusMap[iRegIndex].MbRegPermissions == MBR_READ || ModbusMap[iRegIndex].MbRegPermissions == MBR_READ_WRITE )
//                    {
//                        *pucRegBuffer++ = (uint8)(dvarSearchResult.value >> 24);
//                        *pucRegBuffer++ = (uint8)(dvarSearchResult.value >> 16);
//                        *pucRegBuffer++ = (uint8)(dvarSearchResult.value >> 8);
//                        *pucRegBuffer++ = (uint8)(dvarSearchResult.value >> 0);
//                    }
//                    usNRegs--;
//                }
//                else if( ModbusMap[iRegIndex].MbRegSize == MBR_UINT16 )
//                {
//                    if ( ModbusMap[iRegIndex].MbRegPermissions == MBR_READ ||
//                         ModbusMap[iRegIndex].MbRegPermissions == MBR_READ_WRITE )
//                    {
//                        *pucRegBuffer++ = (uint8)(dvarSearchResult.value >> 8);
//                        *pucRegBuffer++ = (uint8)(dvarSearchResult.value);
//                    }
//                }
//            }
//            else if( eMode == MB_WRITE )
//            {
//                dvarSearchResult.id = (DistVarID)ModbusMap[iRegIndex].DvarId;
//                if( ( ModbusMap[iRegIndex].MbRegPermissions == MBR_WRITE ||
//                      ModbusMap[iRegIndex].MbRegPermissions == MBR_READ_WRITE ) &&
//                   //setup exception for ControlLocalRemote DVar to always be READ/WRITE
//                     (gSetup.ControlLocalRemote || ModbusMap[iRegIndex].DvarId == DVA16T734_SS( gSetup, ControlLocalRemote )) )
//                {
//                    if( ModbusMap[iRegIndex].MbRegSize == MBR_UINT32 )
//                    {
//                        tempBuffer = ( *pucRegBuffer++ );
//                        tempBuffer = ( tempBuffer << 8 ) + (uint8)( *pucRegBuffer++ );
//                        tempBuffer = ( tempBuffer << 8 ) + (uint8)( *pucRegBuffer++ );
//                        tempBuffer = ( tempBuffer << 8 ) + (uint8)( *pucRegBuffer++ );
//                        usNRegs--;
//                    }
//                    else if( ModbusMap[iRegIndex].MbRegSize == MBR_UINT16 )
//                    {
//                        tempBuffer = ( *pucRegBuffer++ );
//                        tempBuffer = ( tempBuffer << 8 ) + (uint8)( *pucRegBuffer++ );
//                    }
//
//                    // Check if callback is required
//                    if ( ModbusMap[iRegIndex].MbRegCallback )
//                    {
//                        dvarSearchResult.value = ModbusMap[iRegIndex].MbRegCallback( TRUE, tempBuffer, &ModbusMap[iRegIndex] );
//
//                        //ANK 4/17/14 - Changed to ACK because remote DVAR setpoints need to be sure to have been handled before moving to
//                        // the next register when receiving modbus multiple register writes
//                        if( dvarSearchResult.id )   (void)DVAR_SetPointRemote_ACK( dvarSearchResult.id, dvarSearchResult.value, 5 );
//                    }
//                    else    (void)DVAR_SetPointRemote( dvarSearchResult.id, tempBuffer );
//                }
//                else
//                {
//                    // No writing permissions to the register
//                    eStatus = MB_NOREG;
//                }
//            }
//            iRegIndex++;
//            usNRegs--;
//        }
//    }
//    else eStatus = MB_NOREG;
//
//    return eStatus;
//}
