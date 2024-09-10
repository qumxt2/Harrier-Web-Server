//! \file	dvinterface_gcaCanBootloader.h
//! Copyright 2006-12
//! Graco Inc., Minneapolis, MN
//! All Rights Reserved
//!

#ifndef DVINTERFACE_GCACANBOOTLOADER_H
#define DVINTERFACE_GCACANBOOTLOADER_H

typedef enum
{
	APP_PN_DVAR_IDX_STR_3_0 = 0,
	APP_PN_DVAR_IDX_STR_7_4,
	APP_PN_DVAR_IDX_STR_11_8,
	APP_PN_DVAR_IDX_STR_15_12,

	APP_PN_DVAR_MAX_NUM,
} APP_PN_DVAR_IDX;

// Bits for Status_bitfield in DVSTRUCT_CAN_BL_HOST_t structure
#define DVB_CAN_BL_HOST_STATUS_HOST_ACTIVE_bitnum						(0)	// Indicates that one or more module has indicated that it will accept an update and we're in the process of sending updates.
#define DVB_CAN_BL_HOST_STATUS_HOST_PENDING_bitnum						(1)	// Indicates that an update could occur as a token is present, but that the system is still establishing communications, etc.
#define DVB_CAN_BL_HOST_STATUS_WAIT_ON_USER_FDBCK_bitnum				(2)	// Indicates that we need the user to choose where software goes.
#define DVB_CAN_BL_HOST_STATUS_WAIT_ON_USER_FDBCK_SCAN_bitnum			(3)	// Indicates that we're not activlely waiting for feedback but are waiting for the token scan to advance
#define DVB_CAN_BL_HOST_STATUS_WAIT_ON_UPDATE_PROCEED_bitnum			(4)	// Indicates that the HMI must make the next move to allow bootloading to continue.
#define DVB_CAN_BL_HOST_STATUS_WAIT_ON_UPDATE_COMPLETE_bitnum			(5)	// Indicates that the user must acknowledge the recent update process.
#define DVB_CAN_BL_HOST_STATUS_NOT_INITIATED_bitnum    					(31) // Indicate that the CAN bootload process was not initiated

#define DVMASK_CAN_BL_HOST_STATUS_ANY_ACTIVITY                          (0xFFFFFFFFUL ^ (1UL<<DVB_CAN_BL_HOST_STATUS_NOT_INITIATED_bitnum))

// Bits for UpdateCtrlRegister_bitfield in DVSTRUCT_CAN_BL_HOST_t structure
#define DVB_CAN_BL_HOST_CTRL_REGISTER_PROCEED_bitnum					(0)

#define NUM_NODES_PER_APP_SELECTION_DVAR								(2)
#define NUM_NODE_APP_SELECTION_DVARS									(64/NUM_NODES_PER_APP_SELECTION_DVAR)

#define MAX_NUM_SOFTWARE_PART_NUMS										(8)
#define APPLICATION_ID_BITMASK											(0xFFFFUL)

#define DVSEG_INVALID_INDEX												(0xFFFFFFFFUL)

typedef struct
{
	DistVarType	Status_bitfield;
	DistVarType UpdatesCompleted_u32;
	DistVarType UpdatesFailed_u32;
	DistVarType UpdateStatus_Overall_percent_u8d0;
	DistVarType UpdateStatus_Overall_ElapsedTime_seconds_u32d0;
	DistVarType UpdateStatus_Overall_TimeRemaining_seconds_u32d0;
	DistVarType UpdateStatus_CurrentApp_AcceptedNodes_31_0_bitfield;
	DistVarType UpdateStatus_CurrentApp_AcceptedNodes_63_32_bitfield;
	DistVarType UpdateCtrlRegister_bitfield;

	DistVarType NodeAppSelection_bitfield[NUM_NODE_APP_SELECTION_DVARS];

	DistVarType MultAppModule_NumAvailable_u32;
	DistVarType MultAppModule_Index_u32;
	DistVarType MultAppModule_Response_Index_u32;
	DistVarType MultAppModule_Response_AppId_uint32;
	DistVarType MultAppModule_Response_PartNum_3_0;
	DistVarType MultAppModule_Response_PartNum_7_4;
	DistVarType MultAppModule_Response_PartNum_11_8;
	DistVarType MultAppModule_Response_PartNum_15_12;

    DistVarType ModulesWithoutCanBL_u32;
} DVSTRUCT_CAN_BL_HOST_t;

typedef enum
{
	// *** IDENTIFICATION DVARS ***
	DVOFFSET_CAN_BL_HOST_SEGMENT_BASE 				= 0x0000,

	DVOFFSET_CAN_BL_HOST_STATUS						= 0x0000,

	DVOFFSET_CAN_BL_HOST_UPDATES_COMPLETED			= 0x0001,
	DVOFFSET_CAN_BL_HOST_UPDATES_FAILED				= 0x0002,

	DVOFFSET_CAN_BL_HOST_STATUS_OVERALL_PERCENT		= 0x0003,
	DVOFFSET_CAN_BL_HOST_STATUS_OVERALL_ELAPSED		= 0x0004,
	DVOFFSET_CAN_BL_HOST_STATUS_OVERALL_REMAINING	= 0x0005,

	DVOFFSET_CAN_BL_HOST_ACCEPTED_NODES_31_0		= 0x0006,
	DVOFFSET_CAN_BL_HOST_ACCEPTED_NODES_63_32		= 0x0007,

	DVOFFSET_CAN_BL_HOST_CTRL_REGISTER				= 0x0008,

	DVOFFSET_CAN_BL_HOST_NODE_APP_SELECTION_1_0		= 0x0009,
//	... nodes 2-61
//	DVOFFSET_CAN_BL_HOST_NODE_APP_SELECTION_63_62	= 0x0028,

	DVOFFSET_CAN_BL_HOST_MAM_NumAvailable			= 0x0029,
	DVOFFSET_CAN_BL_HOST_MAM_Index_u32				= 0x002A,
	DVOFFSET_CAN_BL_HOST_MAM_Rsp_Index_u32			= 0x002B,
	DVOFFSET_CAN_BL_HOST_MAM_Rsp_AppId				= 0x002C,
	DVOFFSET_CAN_BL_HOST_MAM_Rsp_PartNum_3_0		= 0x002D,
	DVOFFSET_CAN_BL_HOST_MAM_Rsp_PartNum_7_4		= 0x002E,
	DVOFFSET_CAN_BL_HOST_MAM_Rsp_PartNum_11_8		= 0x002F,
	DVOFFSET_CAN_BL_HOST_MAM_Rsp_PartNum_15_12		= 0x0030,

    DVOFFSET_CAN_BL_HOST_MODULES_WITHOUT_CAN_BL     = 0x0031,
} DVOFFSET_CAN_BL_HOST_t;

#define DVBASE_CAN_BL_HOST									(0x00010000UL)
#define DVCOUNT_CAN_BL_HOST_SEGMENT							(sizeof(DVSTRUCT_CAN_BL_HOST_t)/sizeof(DistVarType))

#define DVADDR_CAN_BL_HOST_STATUS							(DVBASE_CAN_BL_HOST + (uint16)DVOFFSET_CAN_BL_HOST_STATUS)

#define DVADDR_CAN_BL_HOST_UPDATES_COMPLETED				(DVBASE_CAN_BL_HOST + (uint16)DVOFFSET_CAN_BL_HOST_UPDATES_COMPLETED)
#define DVADDR_CAN_BL_HOST_UPDATES_FAILED					(DVBASE_CAN_BL_HOST + (uint16)DVOFFSET_CAN_BL_HOST_UPDATES_FAILED)

#define DVADDR_CAN_BL_HOST_STATUS_OVERALL_PERCENT			(DVBASE_CAN_BL_HOST + (uint16)DVOFFSET_CAN_BL_HOST_STATUS_OVERALL_PERCENT)
#define DVADDR_CAN_BL_HOST_STATUS_OVERALL_ELAPSED			(DVBASE_CAN_BL_HOST + (uint16)DVOFFSET_CAN_BL_HOST_STATUS_OVERALL_ELAPSED)
#define DVADDR_CAN_BL_HOST_STATUS_OVERALL_REMAINING			(DVBASE_CAN_BL_HOST + (uint16)DVOFFSET_CAN_BL_HOST_STATUS_OVERALL_REMAINING)

#define DVADDR_CAN_BL_HOST_ACCEPTED_NODES_31_0				(DVBASE_CAN_BL_HOST + (uint16)DVOFFSET_CAN_BL_HOST_ACCEPTED_NODES_31_0)
#define DVADDR_CAN_BL_HOST_ACCEPTED_NODES_63_32				(DVBASE_CAN_BL_HOST + (uint16)DVOFFSET_CAN_BL_HOST_ACCEPTED_NODES_63_32)

#define DVADDR_CAN_BL_HOST_CTRL_REGISTER					(DVBASE_CAN_BL_HOST + (uint16)DVOFFSET_CAN_BL_HOST_CTRL_REGISTER)

#define DVADDR_CAN_BL_HOST_NODE_APP_SELECTION(node)			(DVBASE_CAN_BL_HOST + (uint16)DVOFFSET_CAN_BL_HOST_NODE_APP_SELECTION_1_0+(node/NUM_NODES_PER_APP_SELECTION_DVAR))

#define DVADDR_CAN_BL_HOST_MAM_NUM_AVAILABLE				(DVBASE_CAN_BL_HOST + (uint16)DVOFFSET_CAN_BL_HOST_MAM_NumAvailable)
#define DVADDR_CAN_BL_HOST_MAM_INDEX						(DVBASE_CAN_BL_HOST + (uint16)DVOFFSET_CAN_BL_HOST_MAM_Index_u32)
#define DVADDR_CAN_BL_HOST_MAM_RSP_INDEX					(DVBASE_CAN_BL_HOST + (uint16)DVOFFSET_CAN_BL_HOST_MAM_Rsp_Index_u32)
#define DVADDR_CAN_BL_HOST_MAM_RSP_APP_ID					(DVBASE_CAN_BL_HOST + (uint16)DVOFFSET_CAN_BL_HOST_MAM_Rsp_AppId)
#define DVADDR_CAN_BL_HOST_MAM_RSP_PN_STR_3_0				(DVBASE_CAN_BL_HOST + (uint16)DVOFFSET_CAN_BL_HOST_MAM_Rsp_PartNum_3_0)
#define DVADDR_CAN_BL_HOST_MAM_RSP_PN_STR_7_4				(DVBASE_CAN_BL_HOST + (uint16)DVOFFSET_CAN_BL_HOST_MAM_Rsp_PartNum_7_4)
#define DVADDR_CAN_BL_HOST_MAM_RSP_PN_STR_11_8				(DVBASE_CAN_BL_HOST + (uint16)DVOFFSET_CAN_BL_HOST_MAM_Rsp_PartNum_11_8)
#define DVADDR_CAN_BL_HOST_MAM_RSP_PN_STR_15_12				(DVBASE_CAN_BL_HOST + (uint16)DVOFFSET_CAN_BL_HOST_MAM_Rsp_PartNum_15_12)

#define DVADDR_CAN_BL_HOST_MODULES_WITHOUT_CAN_BL   		(DVBASE_CAN_BL_HOST + (uint16)DVOFFSET_CAN_BL_HOST_MODULES_WITHOUT_CAN_BL)

#endif // DVINTERFACE_GCACANBOOTLOADER_H
