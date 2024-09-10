//! \file	modbus.c
//! \brief  Standard Bitmaps Library
//!
//! Copyright 2013
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

#ifndef _MODBUS_H
#define _MODBUS_H

// ******************************************************************************************
// * HEADER FILES
// ******************************************************************************************
#include "typedef.h"
#include "dvar.h"

//! \defgroup modbus Modbus
//! \code #include "modbus.h" \endcode
//!
//! This module defines the interface for the application. It contains
//! the basic functions and types required to use the Modbus protocol stack.
//! A typical application will want to call MODBUS_Initialize() first. If the device
//! is ready to answer network requests it must then call MODBUS_Enable() to activate
//! the protocol stack. In the main loop the function MODBUS_Poll() must be called
//! periodically. The time interval between pooling depends on the configured
//! Modbus timeout. If an RTOS is available a separate task should be created
//! and the task should always call the function MODBUS_Poll().
//! See modbus_task.h for RTOS task functions
//!

// ******************************************************************************************
// * MACROS & CONSTANTS
// ******************************************************************************************

// ******************************************************************************************
// * TYPEDEFS & STRUCTURES
// ******************************************************************************************

//! \ingroup modbus
//! \brief If register should be written or read.
//!
//! This value is passed to the callback functions which support either
//! reading or writing register values. Writing means that the application
//! registers should be updated and reading means that the modbus protocol
//! stack needs to know the current register values.
//!
//! \see MODBUS_RegHoldingCB( ), MODBUS_egCoilsCB( ),
//! MODBUS_RegDiscreteCB( ) and MODBUS_RegInputCB( ).

typedef enum
{
    MB_READ,                /*!< Read register values and pass to protocol stack. */
    MB_WRITE                /*!< Update register values. */
} Modbus_RegisterMode;

//! \ingroup modbus
//! \brief Errorcodes used by all function in the protocol stack.
typedef enum
{
    MB_NOERR,                  /*!< no error. */
    MB_NOREG,                  /*!< illegal register address. */
    MB_INVAL,                  /*!< illegal argument. */
    MB_PORTERR,                /*!< porting layer error. */
    MB_NORES,                  /*!< insufficient resources. */
    MB_IO,                     /*!< I/O error. */
    MB_ILLSTATE,               /*!< protocol stack in illegal state. */
    MB_TIMEDOUT                /*!< timeout error occurred. */
} Modbus_ErrorCode;

typedef enum
{
    MB_NONE_PAR,                /*!< No parity. */
    MB_ODD_PAR,                 /*!< Odd parity. */
    MB_EVEN_PAR                 /*!< Even parity. */
} Modbus_Parity;

typedef uint8( *pxMODBUSFunctionHandler ) ( uint8 *pucRcvAddress, uint8 *pucFrame, uint16 *pusLength );

// Map Related Variables
typedef uint32( *Modbus_RegCallback )(bool set, uint32 val, void *MbMapRecord);

typedef enum Modbus_RegSize
{
    MBR_UINT16 = 0,
    MBR_UINT32
} Modbus_RegSize;

typedef enum Modbus_RegPermissions
{
    MBR_READ = 0,
    MBR_WRITE,
    MBR_READ_WRITE
} Modbus_RegPermissions;

typedef struct Modbus_MapVar
{
    uint32 ModbusAddr;
    DistVarID DvarId;
    Modbus_RegCallback MbRegCallback;
    Modbus_RegPermissions MbRegPermissions;
    Modbus_RegSize MbRegSize;
} Modbus_MapVar;

typedef struct
{
    Modbus_MapVar* pModbusMap;
    uint16 mapSize;
} Modbus_MapIndex;


// ******************************************************************************************
// * PUBLIC FUNCTION PROTOTYPES
// ******************************************************************************************
///------------------------------------------------------------------------------------------
//! \ingroup modbus
//! \fn MODBUS_Initialize( uint8 *ucSlaveAddress, uint8 ucPort, uint32 ulBaudRate,
//!                        Modbus_Parity eParity, uint8 stopBits )
//! \brief  Initialize the Modbus protocol stack.
//!         This functions initializes theRTU module and calls the
//!         init functions of the porting layer to prepare the hardware. Please
//!         note that the receiver is still disabled and no Modbus frames are
//!         processed until MODBUS_Enable( ) has been called.
//!
//! \param  ucSlaveAddress The slave address list. Only frames sent to this
//!         addresses or to the broadcast address are processed.
//! \param  ucPort The port to use. E.g. 1 for COM1 on windows. This value
//!         is platform dependent and some ports simply choose to ignore it.
//! \param  ulBaudRate The baudrate. E.g. 19200. Supported baudrates depend
//!         on the porting layer.
//! \param  MODBUS_Parity Parity used for serial transmission.
//!
//! \return If no error occurs the function returns Modbus_ErrorCode::MB_NOERR.
//!         The protocol is then in the disabled state and ready for activation
//!         by calling MODBUS_Enable( ). Otherwise one of the following error codes
//!         is returned:
//!         - Modbus_ErrorCode::MB_INVAL If the slave address was not valid. Valid
//!           slave addresses are in the range 1 - 247.
//!         - Modbus_ErrorCode::MB_PORTERR IF the porting layer returned an error.
///------------------------------------------------------------------------------------------
Modbus_ErrorCode MODBUS_Initialize( uint8 *ucSlaveAddress, uint8 ucPort, uint32 ulBaudRate,
                                    Modbus_Parity eParity, uint8 stopBits );


///------------------------------------------------------------------------------------------
//!\ingroup modbus
//! \brief  Release resources used by the protocol stack.
//!
//!         This function disables the Modbus protocol stack and release all
//!         hardware resources. It must only be called when the protocol stack
//!         is disabled.
//!
//! \note   Note all ports implement this function. A port which wants to
//!         get an callback must define the macro MB_PORT_HAS_CLOSE to 1.
//!
//! \return If the resources where released it return Modbus_ErrorCode::MB_NOERR.
//!         If the protocol stack is not in the disabled state it returns
//!         Modbus_ErrorCode::MB_ILLSTATE.
///------------------------------------------------------------------------------------------
Modbus_ErrorCode MODBUS_Close( void );


///------------------------------------------------------------------------------------------
//! \ingroup modbus
//! \brief  Enable the Modbus protocol stack.
//!
//!         This function enables processing of Modbus frames. Enabling the protocol
//!         stack is only possible if it is in the disabled state.
//!
//! \return If the protocol stack is now in the state enabled it returns
//!         Modbus_ErrorCode::MB_NOERR. If it was not in the disabled state it
//!         return Modbus_ErrorCode::MB_ILLSTATE.
///------------------------------------------------------------------------------------------
Modbus_ErrorCode MODBUS_Enable( void );


///------------------------------------------------------------------------------------------
//! \ingroup modbus
//! \brief  Disable the Modbus protocol stack.
//!
//!         This function disables processing of Modbus frames.
//!
//! \return If the protocol stack has been disabled it returns
//!         Modbus_ErrorCode::MB_NOERR. If it was not in the enabled state it returns
//!         Modbus_ErrorCode::MB_ILLSTATE.
///------------------------------------------------------------------------------------------
Modbus_ErrorCode MODBUS_Disable( void );


///------------------------------------------------------------------------------------------
//! \ingroup modbus
//! \brief  The main pooling loop of the Modbus protocol stack.
//!
//!         This function must be called periodically. The timer interval required
//!         is given by the application dependent Modbus slave timeout. Internally the
//!         function calls xMBPortEventGet() and waits for an event from the receiver or
//!         transmitter state machines.
//!
//! \return If the protocol stack is not in the enabled state the function
//!         returns Modbus_ErrorCode::MB_ILLSTATE. Otherwise it returns
//!         Modbus_ErrorCode::MB_NOERR.
///------------------------------------------------------------------------------------------
Modbus_ErrorCode MODBUS_Poll( void );


///------------------------------------------------------------------------------------------
//! \ingroup modbus
//! \brief  Configure the slave id of the device.
//!
//!         This function should be called when the Modbus function <em>Report Slave ID</em>
//!         is enabled ( By defining MB_FUNC_OTHER_REP_SLAVEID_ENABLED in mbconfig.h ).
//!
//! \param  ucSlaveID Values is returned in the <em>Slave ID</em> byte of the
//!         <em>Report Slave ID</em> response.
//! \param  xIsRunning If TRUE the <em>Run Indicator Status</em> byte is set to 0xFF.
//!         otherwise the <em>Run Indicator Status</em> is 0x00.
//! \param  pucAdditional Values which should be returned in the <em>Additional</em>
//!         bytes of the <em> Report Slave ID</em> response.
//! \param  usAdditionalLen Length of the buffer <code>pucAdditonal</code>.
//!
//! \return If the static buffer defined by MB_FUNC_OTHER_REP_SLAVEID_BUF in
//!         mbconfig.h is to small it returns Modbus_ErrorCode::MB_NORES. Otherwise
//!         it returns Modbus_ErrorCode::MB_NOERR.
///------------------------------------------------------------------------------------------
Modbus_ErrorCode MODBUS_SetSlaveID( uint8 ucSlaveID, bool xIsRunning, uint8 const *pucAdditional, uint16 usAdditionalLen );


///------------------------------------------------------------------------------------------
//! \ingroup modbus
//! \fn MODBUS_Map_Initialize( Modbus_MapIndex *Mb_mIndex, uint8 maxIds )
//! \brief  Initialize Modbus Map.
//!         This function should be called when the Modbus is initilized
//!         prior adding new map.
//! \param  Mb_mIndex is a pointer to locally initialized array of type
//!         Mudbus_MapIndex with size of max Slave IDs available on the device.
//!         Generally array size should equal to number of Modbus maps.
//! \param  maxIds is the size of the Mb_mIndex array. Ex. sizeof(Mb_mIndex).
//!
//! \return void.
///------------------------------------------------------------------------------------------
void MODBUS_Map_Initialize( Modbus_MapIndex *Mb_mIndex, uint8 maxIds );


///------------------------------------------------------------------------------------------
//! \ingroup modbus
//! \fn MODBUS_Map_Add( uint8 slaveId, Modbus_MapVar* pModbusMap )
//! \brief  Add Modbus Map associated to a certain Slave ID.
//!         This function should be called when the Modbus is initilized
//!         for each Slave ID - Map pair.
//! \param  slaveId integer type Slave ID value between 1 and 247.
//! \param  pModbusMap pointer to a Modbus map initialized prior
//!         to calling this function.
//!
//! \notes  Example of Modbus Map:
//!         //  Mb Register|DVAR Value|Callback(set,val)|Reigster Permission|Size
//!
//!         const Modbus_MapVar ModbusMap[] = {
//!            { 402000,	DVR(dateYear_u8),   FuncCb, MBR_READ_WRITE,		MBR_UINT16 },
//!            { 402001,	DVR(dateMonth_u8),  NULL,	MBR_READ_WRITE,		MBR_UINT16 },
//!            { 402002,	DVR(dateDay_u8), 	NULL,	MBR_READ_WRITE,		MBR_UINT16 },
//!            { 402003,	DVR(timeHour_u8), 	NULL,	MBR_READ_WRITE,		MBR_UINT16 },
//!            { 402004,	DVR(timeMinute_u8), NULL,	MBR_READ_WRITE,		MBR_UINT16 },
//!            { 402005,	DVR(timeSecond_u8), NULL,	MBR_READ_WRITE,		MBR_UINT16 },
//!            { 0xffffffff,0,                  NULL,   MBR_READ,           MBR_UINT16 }};
//!         //  Always end map with termination value shown above
//!
//! \return void.
///------------------------------------------------------------------------------------------
bool MODBUS_Map_Add( uint8 slaveId, Modbus_MapVar* pModbusMap );


///------------------------------------------------------------------------------------------
//! \ingroup modbus
//! \fn MODBUS_Map_Get( uint8 slaveId )
//! \brief  Get Modbus Map pointer.
//!         This function is used to handle Modbus request to return and set
//!         approriate register value.
//! \param  slaveId integer type Slave ID value between 1 and 247.
//!
//! \return Modbus map pointer tyte Modbus_MapVar.
///------------------------------------------------------------------------------------------
Modbus_MapVar* MODBUS_Map_Get( uint8 slaveId );


///------------------------------------------------------------------------------------------
//! \ingroup modbus
//! \fn MODBUS_Map_GetSize( uint8 slaveId )
//! \brief  Get Size of a Modbus Map associate to a provided Slave ID.
//!         This function primary called by register callback and
//!         register search functions, and in general cases is not used.
//! \param  slaveId integer type Slave ID value between 1 and 247.
//!
//! \return Integer (uint16) value of the number elements in the map.
///------------------------------------------------------------------------------------------
uint16 MODBUS_Map_GetSize( uint8 slaveId );


///------------------------------------------------------------------------------------------
//! \ingroup modbus
//! \fn MODBUS_Map_GetRegisterIndex( uint8 slaveId, uint32 modbusRegister )
//! \brief  Get index in the modbus map of the modbus register by Slave ID.
//!         This function performs binary array search to find the index.
//!         Index is used to extract Dvar/Callbacks/Permissions/Size of the register.
//! \param  slaveId integer type Slave ID value between 1 and 247.
//! \param  modbusRegister is Modbus register value to looke up.
//!
//! \return Integer (uint32) value of the map index.
///------------------------------------------------------------------------------------------
sint32 MODBUS_Map_GetRegisterIndex( uint8 slaveId, uint32 modbusRegister );


///------------------------------------------------------------------------------------------
//! \ingroup modbus
//! \brief  Registers a callback handler for a given function code.
//!
//!         This function registers a new callback handler for a given function code.
//!         The callback handler supplied is responsible for interpreting the Modbus PDU and
//!         the creation of an appropriate response. In case of an error it should return
//!         one of the possible Modbus exceptions which results in a Modbus exception frame
//!         sent by the protocol stack.
//!
//! \param  ucFunctionCode The Modbus function code for which this handler should
//!         be registers. Valid function codes are in the range 1 to 127.
//! \param  pxHandler The function handler which should be called in case
//!         such a frame is received. If \c NULL a previously registered function handler
//!         for this function code is removed.
//!
//! \return Modbus_ErrorCode::MB_NOERR if the handler has been installed. If no
//!         more resources are available it returns Modbus_ErrorCode::MB_NORES. In this
//!         case the values in mbconfig.h should be adjusted. If the argument was not
//!         valid it returns Modbus_ErrorCode::MB_INVAL.
///------------------------------------------------------------------------------------------
Modbus_ErrorCode MODBUS_RegisterCB( uint8 ucFunctionCode, pxMODBUSFunctionHandler pxHandler );


// ******************************************************************************************
// * MODBUS REGISTER CALLBACK
// ******************************************************************************************
///------------------------------------------------------------------------------------------
//! \defgroup modbus_registers Modbus Registers
//! \code #include "modbus.h" \endcode
//! The protocol stack does not internally allocate any memory for the
//! registers. This makes the protocol stack very small and also usable on
//! low end targets. In addition the values don't have to be in the memory
//! and could for example be stored in a flash.<br>
//! Whenever the protocol stack requires a value it calls one of the callback
//! function with the register address and the number of registers to read
//! as an argument. The application should then read the actual register values
//! (for example the ADC voltage) and should store the result in the supplied
//! buffer.<br>
//! If the protocol stack wants to update a register value because a write
//! register function was received a buffer with the new register values is
//! passed to the callback function. The function should then use these values
//! to update the application register values.
//!
//! Note: If the callback function is not defined in the application, the 
//! functional library will fall back to standard regester processor. This 
//! will work for holding registers only (MODBUS_RegHoldingCB). Other type
//! of requests will return MB_INVAL if no callback is defined in the application.
///------------------------------------------------------------------------------------------

///------------------------------------------------------------------------------------------
//! \ingroup modbus_registers
//! \brief  Callback function used if the value of a <em>Input Register</em>
//!         is required by the protocol stack. The starting register address is given
//!         by \c usAddress and the last register is given by <tt>usAddress +
//!         usNRegs - 1</tt>.
//!
//! \para   pucRcvAddress integer pointer to the requested Slave ID value.
//!         Used to determine the corresponding Modbus map.
//! \param  pucRegBuffer A buffer where the callback function should write
//!         the current value of the modbus registers to.
//! \param  usAddress The starting address of the register. Input registers
//!         are in the range 1 - 65535.
//! \param  usNRegs Number of registers the callback function must supply.
//!
//! \return The function must return one of the following error codes:
//!         - Modbus_ErrorCode::MB_NOERR If no error occurred. In this case a normal
//!           Modbus response is sent.
//!         - Modbus_ErrorCode::MB_NOREG If the application can not supply values
//!           for registers within this range. In this case a
//!           <b>ILLEGAL DATA ADDRESS</b> exception frame is sent as a response.
//!         - Modbus_ErrorCode::MB_TIMEDOUT If the requested register block is
//!           currently not available and the application dependent response
//!           timeout would be violated. In this case a <b>SLAVE DEVICE BUSY</b>
//!           exception is sent as a response.
//!         - Modbus_ErrorCode::MB_IO If an unrecoverable error occurred. In this case
//!           a <b>SLAVE DEVICE FAILURE</b> exception is sent as a response.
///------------------------------------------------------------------------------------------
Modbus_ErrorCode MODBUS_RegInputCB( uint8 *pucRcvAddress, uint8 *pucRegBuffer, uint16 usAddress, uint16 usNRegs );


///------------------------------------------------------------------------------------------
//! \ingroup modbus_registers
//! \brief  Callback function used if a <em>Holding Register</em> value is
//!         read or written by the protocol stack. The starting register address
//!         is given by \c usAddress and the last register is given by
//!         <tt>usAddress + usNRegs - 1</tt>.
//!
//! \para   pucRcvAddress integer pointer to the requested Slave ID value.
//!         Used to determine the corresponding Modbus map.
//! \param  pucRegBuffer If the application registers values should be updated the
//!         buffer points to the new registers values. If the protocol stack needs
//!         to now the current values the callback function should write them into
//!         this buffer.
//! \param  usAddress The starting address of the register.
//! \param  usNRegs Number of registers to read or write.
//! \param  eMode If eMBRegisterMode::MB_REG_WRITE the application register
//!         values should be updated from the values in the buffer. For example
//!         this would be the case when the Modbus master has issued an
//!         <b>WRITE SINGLE REGISTER</b> command.
//!         If the value eMBRegisterMode::MB_REG_READ the application should copy
//!         the current values into the buffer \c pucRegBuffer.
//!
//! \return The function must return one of the following error codes:
//!         - Modbus_ErrorCode::MB_NOERR If no error occurred. In this case a normal
//!           Modbus response is sent.
//!         - Modbus_ErrorCode::MB_NOREG If the application can not supply values
//!           for registers within this range. In this case a
//!           <b>ILLEGAL DATA ADDRESS</b> exception frame is sent as a response.
//!         - Modbus_ErrorCode::MB_TIMEDOUT If the requested register block is
//!           currently not available and the application dependent response
//!           timeout would be violated. In this case a <b>SLAVE DEVICE BUSY</b>
//!           exception is sent as a response.
//!         - Modbus_ErrorCode::MB_IO If an unrecoverable error occurred. In this case
//!           a <b>SLAVE DEVICE FAILURE</b> exception is sent as a response.
///------------------------------------------------------------------------------------------
Modbus_ErrorCode MODBUS_RegHoldingCB( uint8 *pucRcvAddress, uint8 *pucRegBuffer, uint16 usAddress,
                                      uint16 usNRegs, Modbus_RegisterMode eMode );

///------------------------------------------------------------------------------------------
//! \ingroup modbus_registers
//! \brief  Callback function used if a <em>Coil Register</em> value is
//!         read or written by the protocol stack. If you are going to use
//!         this function you might use the functions xMBUtilSetBits(  ) and
//!         xMBUtilGetBits(  ) for working with bitfields.
//!
//! \para   pucRcvAddress integer pointer to the requested Slave ID value.
//!         Used to determine the corresponding Modbus map.
//! \param  pucRegBuffer The bits are packed in bytes where the first coil
//!         starting at address \c usAddress is stored in the LSB of the
//!         first byte in the buffer <code>pucRegBuffer</code>.
//!         If the buffer should be written by the callback function unused
//!         coil values (I.e. if not a multiple of eight coils is used) should be set
//!         to zero.
//! \param  usAddress The first coil number.
//! \param  sNCoils Number of coil values requested.
//! \param  eMode If eMBRegisterMode::MB_REG_WRITE the application values should
//!         be updated from the values supplied in the buffer \c pucRegBuffer.
//!         If eMBRegisterMode::MB_REG_READ the application should store the current
//!         values in the buffer \c pucRegBuffer.
//!
//! \return The function must return one of the following error codes:
//!         - Modbus_ErrorCode::MB_NOERR If no error occurred. In this case a normal
//!           Modbus response is sent.
//!         - Modbus_ErrorCode::MB_NOREG If the application does not map an coils
//!           within the requested address range. In this case a
//!           <b>ILLEGAL DATA ADDRESS</b> is sent as a response.
//!         - Modbus_ErrorCode::MB_TIMEDOUT If the requested register block is
//!           currently not available and the application dependent response
//!           timeout would be violated. In this case a <b>SLAVE DEVICE BUSY</b>
//!           exception is sent as a response.
//!         - Modbus_ErrorCode::MB_IO If an unrecoverable error occurred. In this case
//!           a <b>SLAVE DEVICE FAILURE</b> exception is sent as a response.
///------------------------------------------------------------------------------------------
Modbus_ErrorCode MODBUS_RegCoilsCB( uint8 *pucRcvAddress, uint8 *pucRegBuffer, uint16 usAddress,
                                    uint16 usNCoils, Modbus_RegisterMode eMode );

///------------------------------------------------------------------------------------------
//! \ingroup modbus_registers
//! \brief  Callback function used if a <em>Input Discrete Register</em> value is
//!         read by the protocol stack.
//!
//!         If you are going to use his function you might use the functions
//!         xMBUtilSetBits(  ) and xMBUtilGetBits(  ) for working with bitfields.
//!
//! \para   pucRcvAddress integer pointer to the requested Slave ID value.
//!         Used to determine the corresponding Modbus map.
//! \param  pucRegBuffer The buffer should be updated with the current
//!         coil values. The first discrete input starting at \c usAddress must be
//!         stored at the LSB of the first byte in the buffer. If the requested number
//!         is not a multiple of eight the remaining bits should be set to zero.
//! \param  usAddress The starting address of the first discrete input.
//! \param  usNDiscrete Number of discrete input values.
//! \return The function must return one of the following error codes:
//!         - Modbus_ErrorCode::MB_NOERR If no error occurred. In this case a normal
//!           Modbus response is sent.
//!         - Modbus_ErrorCode::MB_NOREG If no such discrete inputs exists.
//!           In this case a <b>ILLEGAL DATA ADDRESS</b> exception frame is sent
//!           as a response.
//!         - Modbus_ErrorCode::MB_TIMEDOUT If the requested register block is
//!           currently not available and the application dependent response
//!           timeout would be violated. In this case a <b>SLAVE DEVICE BUSY</b>
//!           exception is sent as a response.
//!         - Modbus_ErrorCode::MB_IO If an unrecoverable error occurred. In this case
//!           a <b>SLAVE DEVICE FAILURE</b> exception is sent as a response.
///------------------------------------------------------------------------------------------
Modbus_ErrorCode MODBUS_RegDiscreteCB( uint8 *pucRcvAddress, uint8 *pucRegBuffer, uint16 usAddress,
                                       uint16 usNDiscrete );

#endif
