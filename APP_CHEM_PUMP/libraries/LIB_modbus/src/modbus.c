//! \file	modbus.c
//! \brief  Standard Bitmaps Library
//!
//! Copyright 2012
//! Graco Inc., Minneapolis, MN
//! All Rights Reserved
//!
//!
//! \addtogroup gca_adcm_func_modbus
//@{
//!
//! \addtogroup gca_adcm_func_modbus	ADCM Modbus Library
//! \brief ADCM (Advanced Display Module)
//!
//! \b DESCRIPTION:
//!    This module impliments Modbus protocol on a serial port
//!    in the A/DCM (Advanced / Display Control Module).

// ******************************************************************************************
// HEADER FILES
// ******************************************************************************************
#include "mb.h"
#include "typedef.h"
#include "modbus.h"

// ******************************************************************************************
// * MACROS & CONSTANTS
// ******************************************************************************************

// ******************************************************************************************
// * PRIVATE VARIABLES
// ******************************************************************************************
static uint8 Modbus_maxIds = 1;
static uint8 Modbus_idArray[255] =  { [0 ... 254] = 0xFF };  //lint !e10
static Modbus_MapIndex *Modbus_mIndex = NULL;

// ******************************************************************************************
// * PUBLIC FUNCTIONS
// ******************************************************************************************

Modbus_ErrorCode MODBUS_Initialize( uint8 *ucSlaveAddress, uint8 ucPort, uint32 ulBaudRate,
                                    Modbus_Parity eParity, uint8 stopBits )
{
    return (Modbus_ErrorCode) eMBInit( MB_RTU, ucSlaveAddress, ucPort, ulBaudRate, (eMBParity)eParity, stopBits );
}

Modbus_ErrorCode MODBUS_Close( void )
{
    return (Modbus_ErrorCode) eMBClose( );
}

Modbus_ErrorCode MODBUS_Enable( void )
{
    return (Modbus_ErrorCode) eMBEnable( );
}

Modbus_ErrorCode MODBUS_Disable( void )
{
    return (Modbus_ErrorCode) eMBDisable( );
}

Modbus_ErrorCode MODBUS_Poll( void )
{
    return (Modbus_ErrorCode) eMBPoll( );
}

Modbus_ErrorCode MODBUS_SetSlaveID( uint8 ucSlaveID, bool xIsRunning,
                                    uint8 const *pucAdditional, uint16 usAdditionalLen )
{
    return (Modbus_ErrorCode) eMBSetSlaveID( (uchar)ucSlaveID, xIsRunning, pucAdditional, usAdditionalLen );
}

void MODBUS_Map_Initialize( Modbus_MapIndex *Mb_mIndex, uint8 maxIds )
{
    Modbus_maxIds = maxIds;
    Modbus_mIndex = Mb_mIndex;
}

bool MODBUS_Map_Add( uint8 slaveId, Modbus_MapVar* pModbusMap )
{
    uint8 i;
    bool rtnVal;

    if( Modbus_mIndex == NULL )
    {
        rtnVal = FALSE;
        return rtnVal;
    }

    if( slaveId > 255 )
    {
        rtnVal = FALSE;
        return rtnVal;
    }

    // Reuse Index Map if it was previously assigned
    if( Modbus_idArray[slaveId] != 0xFF &&
        Modbus_idArray[slaveId] >= 0 &&
        Modbus_idArray[slaveId] < Modbus_maxIds )
    {
        Modbus_mIndex[Modbus_idArray[slaveId]].pModbusMap = pModbusMap;
        rtnVal = TRUE;
    }

        // Search for open spot in the Index Array
    else
    {
        for( i = 0; i < Modbus_maxIds; i++ )
        {
            if( Modbus_mIndex[i].pModbusMap == NULL )
            {
                Modbus_mIndex[i].pModbusMap = pModbusMap;
                Modbus_idArray[slaveId] = i;

                rtnVal = TRUE;
                break;
            }
        }
    }

    return rtnVal;
}

Modbus_MapVar* MODBUS_Map_Get( uint8 slaveId )
{
    if( slaveId > 255 ||
        Modbus_idArray[slaveId] == 0xFF ||
        Modbus_idArray[slaveId] >= Modbus_maxIds ||
        Modbus_mIndex == NULL )
    {
        return NULL;
    }

    return Modbus_mIndex[Modbus_idArray[slaveId]].pModbusMap;
}

uint16 MODBUS_Map_GetSize( uint8 slaveId )
{
    uint16 i;
    uint16 rtnVal = 0;
    uint16 mapMaxSize = 1000;

    if( slaveId > 255 ||
        Modbus_idArray[slaveId] == 0xFF ||
        Modbus_idArray[slaveId] >= Modbus_maxIds ||
        Modbus_mIndex[Modbus_idArray[slaveId]].pModbusMap == NULL )
    {
        return rtnVal;
    }

    if( Modbus_mIndex[Modbus_idArray[slaveId]].mapSize == 0 )
    {
        for( i = 0; i <= mapMaxSize; i++ )
        {
            if( Modbus_mIndex[Modbus_idArray[slaveId]].pModbusMap[i].ModbusAddr == 0xffffffff )
            {
                Modbus_mIndex[Modbus_idArray[slaveId]].mapSize = i + 1;
                rtnVal = i + 1;
                break;
            };
        }
    }
    else
    {
        rtnVal = Modbus_mIndex[Modbus_idArray[slaveId]].mapSize;
    }

    return rtnVal;
}

sint32 MODBUS_Map_GetRegisterIndex( uint8 slaveId, uint32 modbusRegister )
{
    uint32 low, high, mid;
    Modbus_MapVar *ModbusMap;

    ModbusMap = MODBUS_Map_Get( slaveId );
    high = MODBUS_Map_GetSize( slaveId ) - 2;		// -2 to account for last terminate element in the array
    low = 0;

    if ( ( ModbusMap[low].ModbusAddr > modbusRegister ) ||
         ( ModbusMap[high].ModbusAddr < modbusRegister ) )
    {
        return -1;
    }

    while( low <= high )
    {
        mid = (low + high) / 2;
        //result = strncmp(dict[mid], name, compChars);
        if( ModbusMap[mid].ModbusAddr > modbusRegister )
        {
            high = mid - 1;
        }
        else if( ModbusMap[mid].ModbusAddr < modbusRegister )
        {
            low = mid + 1;
        }
        else
        {
            return mid;
        }
    }

    return -1;
}

Modbus_ErrorCode MODBUS_RegisterCB( uint8 ucFunctionCode, pxMODBUSFunctionHandler pxHandler )
{
    return (Modbus_ErrorCode) eMBRegisterCB( (uchar)ucFunctionCode, (pxMBFunctionHandler)pxHandler );
}

// ******************************************************************************************
// * CALLBACK WRAPPER
// ******************************************************************************************
#define REG_INPUT_START 					(1000)
#define REG_INPUT_NREGS 					(256)
#define REG_HOLDING_BASE					(400000)
#define REG_HOLDING_START 					(1000)
#define REG_HOLDING_NREGS 					(64535)

eMBErrorCode eMBRegInputCB( uchar * pucRcvAddress, uchar * pucRegBuffer, uint16 usAddress, uint16 usNRegs )
{
    Modbus_ErrorCode MODBUS_RegInputCB( uint8 *pucRcvAddress, uint8 *pucRegBuffer,
                                        uint16 usAddress, uint16 usNRegs ) __attribute__( (weak) );
    if( MODBUS_RegInputCB )
    {
        return (Modbus_ErrorCode) MODBUS_RegInputCB( pucRcvAddress, pucRegBuffer, usAddress, usNRegs );
    }
    else
    {
        return MB_INVAL;
    }
}

eMBErrorCode eMBRegHoldingCB( uchar * pucRcvAddress, uchar * pucRegBuffer, uint16 usAddress,
                              uint16 uNRegs, eMBRegisterMode eMode )
{
    // Check if Function is defined
    // If not, use standard routine
    Modbus_ErrorCode MODBUS_RegHoldingCB( uint8 *pucRcvAddress, uint8 *pucRegBuffer, uint16 usAddress,
                                          uint16 uNRegs, Modbus_RegisterMode eMode ) __attribute__( (weak) );

    if( MODBUS_RegHoldingCB )
    {
        return (Modbus_ErrorCode) MODBUS_RegHoldingCB( pucRcvAddress, pucRegBuffer, usAddress, uNRegs, eMode );
    }

    Modbus_ErrorCode    eStatus = MB_NOERR;
    sint32              iRegIndex;
    uint32              tempBuffer = 0;
    sint16              usNRegs = (sint16)uNRegs;
    Modbus_MapVar*      ModbusMap;
    DVarSearchContext   dvarSearchResult;
    sint32              ModbusMapSize;

    if ( usAddress < REG_HOLDING_START )	return MB_NOREG;

    // Get Modbus Map info
    ModbusMap = MODBUS_Map_Get( *pucRcvAddress );
    ModbusMapSize = MODBUS_Map_GetSize( *pucRcvAddress );
    if( ModbusMap == NULL || ModbusMapSize == NULL )    return MB_NOREG;

    iRegIndex = MODBUS_Map_GetRegisterIndex( *pucRcvAddress, REG_HOLDING_BASE + usAddress );

    if( ( 0 <= iRegIndex )
        && ( iRegIndex < ModbusMapSize )
        && ( usAddress >= REG_HOLDING_START ) )
        //&& ( usAddress + usNRegs <= REG_HOLDING_START + REG_HOLDING_NREGS ))
    {
        while( usNRegs > 0 )
        {
            if ( (Modbus_RegisterMode)eMode == MB_READ )
            {
                dvarSearchResult.id = (DistVarID)ModbusMap[iRegIndex].DvarId;
                dvarSearchResult.value = 0x00;

                // Check if requiested register matches the table
                if ( ModbusMap[iRegIndex].ModbusAddr - REG_HOLDING_BASE != usAddress + uNRegs - usNRegs )
                {
                    eStatus = MB_NOREG;
                    return eStatus;
                }

                // Check if callback is required
                if ( ModbusMap[iRegIndex].MbRegCallback )
                {
                    dvarSearchResult.value = ModbusMap[iRegIndex].MbRegCallback( 0, 0, &ModbusMap[iRegIndex] );
                }
                else
                {
                    (void) DVAR_SeekLocalVariable( &dvarSearchResult, dvarSearchResult.id );
                }

                if ( ModbusMap[iRegIndex].MbRegSize == MBR_UINT32 )
                {
                    if ( ModbusMap[iRegIndex].MbRegPermissions == MBR_READ ||
                         ModbusMap[iRegIndex].MbRegPermissions == MBR_READ_WRITE )
                    {
                        *pucRegBuffer++ = (uint8)(dvarSearchResult.value >> 24);
                        *pucRegBuffer++ = (uint8)(dvarSearchResult.value >> 16);
                        *pucRegBuffer++ = (uint8)(dvarSearchResult.value >> 8);
                        *pucRegBuffer++ = (uint8)(dvarSearchResult.value >> 0);
                    }
                    usNRegs--;
                }
                else if ( ModbusMap[iRegIndex].MbRegSize == MBR_UINT16 )
                {
                    if ( ModbusMap[iRegIndex].MbRegPermissions == MBR_READ ||
                         ModbusMap[iRegIndex].MbRegPermissions == MBR_READ_WRITE )
                    {
                        *pucRegBuffer++ = (uint8)(dvarSearchResult.value >> 8);
                        *pucRegBuffer++ = (uint8)(dvarSearchResult.value);
                    }
                }
            }
            else if ( (Modbus_RegisterMode)eMode == MB_WRITE )
            {
                dvarSearchResult.id = (DistVarID)ModbusMap[iRegIndex].DvarId;
                if ( ModbusMap[iRegIndex].MbRegPermissions == MBR_WRITE ||
                     ModbusMap[iRegIndex].MbRegPermissions == MBR_READ_WRITE )
                {
                    if ( ModbusMap[iRegIndex].MbRegSize == MBR_UINT32 )
                    {
                        tempBuffer = ( *pucRegBuffer++ );
                        tempBuffer = ( tempBuffer << 8 ) + (uint8)( *pucRegBuffer++ );
                        tempBuffer = ( tempBuffer << 8 ) + (uint8)( *pucRegBuffer++ );
                        tempBuffer = ( tempBuffer << 8 ) + (uint8)( *pucRegBuffer++ );
                    }
                    else if ( ModbusMap[iRegIndex].MbRegSize == MBR_UINT16 )
                    {
                        tempBuffer = ( *pucRegBuffer++ );
                        tempBuffer = ( tempBuffer << 8 ) + (uint8)( *pucRegBuffer++ );
                    }

                    // Check if callback is required
                    if ( ModbusMap[iRegIndex].MbRegCallback )
                    {
                        dvarSearchResult.value = ModbusMap[iRegIndex].MbRegCallback( TRUE, tempBuffer, &ModbusMap[iRegIndex] );
                        if( dvarSearchResult.id )
                        {
                            (void)DVAR_SetPointRemote( dvarSearchResult.id, dvarSearchResult.value );
                        }
                    }
                    else
                    {
                        (void)DVAR_SetPointRemote( dvarSearchResult.id, tempBuffer );
                    }
                }
                else
                {
                    // No writing permissions to the register
                    eStatus = MB_NOREG;
                }
                usNRegs--;
            }
            iRegIndex++;
            usNRegs--;
        }
    }
    else eStatus = MB_NOREG;

    return eStatus;
}

eMBErrorCode eMBRegCoilsCB( uchar * pucRcvAddress, uchar * pucRegBuffer, uint16 usAddress,
                            uint16 usNCoils, eMBRegisterMode eMode )
{
    Modbus_ErrorCode MODBUS_RegCoilsCB( uint8 *pucRcvAddress, uint8 *pucRegBuffer, uint16 usAddress,
                                        uint16 usNCoils, Modbus_RegisterMode eMode ) __attribute__( (weak) );
    if( MODBUS_RegCoilsCB )
    {
        return (Modbus_ErrorCode) MODBUS_RegCoilsCB( pucRcvAddress, pucRegBuffer, usAddress,
                                                     usNCoils, eMode );
    }
    else
    {
        return MB_INVAL;
    }
}

eMBErrorCode eMBRegDiscreteCB( uchar * pucRcvAddress, uchar * pucRegBuffer, uint16 usAddress, uint16 usNDiscrete )
{
    Modbus_ErrorCode MODBUS_RegDiscreteCB( uint8 * pucRcvAddress, uint8 * pucRegBuffer,
                                           uint16 usAddress, uint16 usNDiscrete ) __attribute__( (weak) );

    if( MODBUS_RegDiscreteCB )
    {
        return (Modbus_ErrorCode) MODBUS_RegDiscreteCB( pucRcvAddress, pucRegBuffer, usAddress, usNDiscrete );
    }
    else
    {
        return MB_INVAL;
    }
}
