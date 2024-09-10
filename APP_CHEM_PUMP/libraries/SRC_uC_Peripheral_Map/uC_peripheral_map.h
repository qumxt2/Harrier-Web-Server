//! \file	uC_peripheral_map.h
//! \brief type defines for Input and/or Output modules.
//!
//! Copyright 2006-2010
//! Graco Inc., Minneapolis, MN
//! All Rights Reserved
//!


#ifndef UC_PERIPHERAL_MAP_H
#define UC_PERIPHERAL_MAP_H

#include "typedef.h"

#include "p32mx795f512l.h"


// ***************************************************
// * MACROS
// ***************************************************

#define CLR_REG_OFFSET		(0x4)
#define SET_REG_OFFSET		(0x8)

//-----------------------------------
// Serial Port U1A mappings
//-----------------------------------
// Serial port U1A "U1RX" is pin 52, RF2.
// Serial port U1A "U1TX" is pin 53, RF8.
// U1A RX Receiver
#define TRIS_U1A_RX					(TRISF)
#define TRISCLR_U1A_RX				(TRISFCLR)
#define TRISSET_U1A_RX				(TRISFSET)
#define BIT_U1A_RX					(2)
#define BITMASK_U1A_RX				(0x0001 << BIT_U1A_RX)

// U1A TX Transmitter
#define TRIS_U1A_TX					(TRISF)
#define TRISCLR_U1A_TX				(TRISFCLR)
#define TRISSET_U1A_TX				(TRISFSET)
#define BIT_U1A_TX					(8)
#define BITMASK_U1A_TX				(0x0001 << BIT_U1A_TX)

// U1A CTS Clear To Send
#define TRIS_U1A_CTS				(TRISD)
#define TRISCLR_U1A_CTS				(TRISDCLR)
#define TRISSET_U1A_CTS				(TRISDSET)
#define BIT_U1A_CTS					(14)
#define BITMASK_U1A_CTS				(0x0001 << BIT_U1A_CTS)

// U1A RTS Request To Send
#define TRIS_U1A_RTS				(TRISD)
#define TRISCLR_U1A_RTS				(TRISDCLR)
#define TRISSET_U1A_RTS				(TRISDSET)
#define BIT_U1A_RTS					(15)
#define BITMASK_U1A_RTS				(0x0001 << BIT_U1A_RTS)


//-----------------------------------
// Digital I/O
//-----------------------------------

// INPUT_1
#define TRIS_INPUT_1                            (_TRISC1)
#define PORT_INPUT_1                            (PORTC)
#define BIT_INPUT_1                             (1)
#define BITMASK_INPUT_1                         (0x0001U<<BIT_INPUT_1)

// INPUT_2
#define TRIS_INPUT_2                            (_TRISC2)
#define PORT_INPUT_2                            (PORTC)
#define BIT_INPUT_2                             (2)
#define BITMASK_INPUT_2                         (0x0001U<<BIT_INPUT_2)

// INPUT_3
#define TRIS_INPUT_3                            (_TRISC3)
#define PORT_INPUT_3                            (PORTC)
#define BIT_INPUT_3                             (3)
#define BITMASK_INPUT_3                         (0x0001U<<BIT_INPUT_3)

// INPUT_4
#define TRIS_INPUT_4                            (_TRISC4)
#define PORT_INPUT_4                            (PORTC)
#define BIT_INPUT_4                             (4)
#define BITMASK_INPUT_4                         (0x0001U<<BIT_INPUT_4)

// OUTPUT_1 - Motor 1 Control
#define TRIS_OUTPUT1                            (_TRISD10)
#define LAT_OUTPUT1                             (_LATD10)
#define LAT_SET_OUTPUT_1                        (LATDSET)
#define LAT_CLR_OUTPUT_1                        (LATDCLR)
#define LAT_READ_OUTPUT_1                       (LATD)
#define BIT_OUTPUT_1                            (10)
#define BITMASK_OUTPUT_1                        (0x0001U<<BIT_OUTPUT_1)

// OUTPUT_2 - Safety Relay Control
#define TRIS_OUTPUT2                            (_TRISD13)
#define LAT_OUTPUT2                             (_LATD13)
#define LAT_SET_OUTPUT_2                        (LATDSET)
#define LAT_CLR_OUTPUT_2                        (LATDCLR)
#define LAT_READ_OUTPUT_2                       (LATD)
#define BIT_OUTPUT_2                            (13)
#define BITMASK_OUTPUT_2                        (0x0001U<<BIT_OUTPUT_2)

// OUTPUT_3 - Motor 2 Control
#define TRIS_OUTPUT3                            (_TRISD9)
#define LAT_OUTPUT3                             (_LATD9)
#define LAT_SET_OUTPUT_3                        (LATDSET)
#define LAT_CLR_OUTPUT_3                        (LATDCLR)
#define LAT_READ_OUTPUT_3                       (LATD)
#define BIT_OUTPUT_3                            (9)
#define BITMASK_OUTPUT_3                        (0x0001U<<BIT_OUTPUT_3)

// OUTPUT_4
//#define TRIS_OUTPUT4                            (_TRISD11)
//#define LAT_OUTPUT4                             (_LATD11)
//#define LAT_SET_OUTPUT_4                        (LATDSET)
//#define LAT_CLR_OUTPUT_4                        (LATDCLR)
//#define LAT_READ_OUTPUT_4                       (LATD)
//#define BIT_OUTPUT_4                            (11)
//#define BITMASK_OUTPUT_4                        (0x0001U<<BIT_OUTPUT_4)

// Alarm LED
#define TRIS_ALARM_LED                          (_TRISD0)
#define LAT_ALARM_LED                           (_LATD0)
#define LAT_SET_ALARM_LED                       (LATDSET)
#define LAT_CLR_ALARM_LED                       (LATDCLR)
#define LAT_READ_ALARM_LED                      (LATD)
#define BIT_ALARM_LED                           (0)
#define BITMASK_ALARM_LED                       (0x0001U<<BIT_ALARM_LED)

// Pump LED
#define TRIS_PUMP_LED                           (_TRISD2)
#define LAT_PUMP_LED                            (_LATD2)
#define LAT_SET_PUMP_LED                        (LATDSET)
#define LAT_CLR_PUMP_LED                        (LATDCLR)
#define LAT_READ_PUMP_LED                       (LATD)
#define BIT_PUMP_LED                            (2)
#define BITMASK_PUMP_LED                        (0x0001U<<BIT_PUMP_LED)

// Cycle LED
#define TRIS_CYCLE_LED                          (_TRISD1)
#define LAT_CYCLE_LED                           (_LATD1)
#define LAT_SET_CYCLE_LED                       (LATDSET)
#define LAT_CLR_CYCLE_LED                       (LATDCLR)
#define LAT_READ_CYCLE_LED                      (LATD)
#define BIT_CYCLE_LED                           (1)
#define BITMASK_CYCLE_LED                       (0x0001U<<BIT_CYCLE_LED)

// Heater Enable
#define TRIS_HEAT_EN                            (_TRISD7)
#define LAT_HEAT_EN                             (_LATD7)
#define LAT_SET_HEAT_EN                         (LATDSET)
#define LAT_CLR_HEAT_EN                         (LATDCLR)
#define LAT_READ_HEAT_EN                        (LATD)
#define BIT_HEAT_EN                             (7)
#define BITMASK_HEAT_EN                         (0x0001U<<BIT_HEAT_EN)

// Modem Reset
#define TRIS_MODEM_RESET                        (_TRISD8)
#define LAT_MODEM_RESET                         (_LATD8)
#define LAT_SET_MODEM_RESET                     (LATDSET)
#define LAT_CLR_MODEM_RESET                     (LATDCLR)
#define LAT_READ_MODEM_RESET                    (LATD)
#define BIT_MODEM_RESET                         (8)
#define BITMASK_MODEM_RESET                     (0x0001U<<BIT_MODEM_RESET)

// ExtCtrInputSel - Outside Control Selector for expecting 4-20mA or 0-10V input signal
#define TRIS_EXT_CONTROL_SELECT                 (_TRISD4)
#define LAT_EXT_CONTROL_SELECT                  (_LATD4)
#define LAT_SET_EXT_CONTROL_SELECT              (LATDSET)
#define LAT_CLR_EXT_CONTROL_SELECT              (LATDCLR)
#define LAT_READ_EXT_CONTROL_SELECT             (LATD)
#define BIT_EXT_CONTROL_SELECT                  (4)
#define BITMASK_EXT_CONTROL_SELECT              (0x0001U<<BIT_EXT_CONTROL_SELECT)

// ExtFeedbackSel - H+ Feedback Selector for 4-20mA or 0-10V output signal
#define TRIS_EXT_FEEDBACK_SELECT                (_TRISD5)
#define LAT_EXT_FEEDBACK_SELECT                 (_LATD5)
#define LAT_SET_EXT_FEEDBACK_SELECT             (LATDSET)
#define LAT_CLR_EXT_FEEDBACK_SELECT             (LATDCLR)
#define LAT_READ_EXT_FEEDBACK_SELECT            (LATD)
#define BIT_EXT_FEEDBACK_SELECT                 (5)
#define BITMASK_EXT_FEEDBACK_SELECT             (0x0001U<<BIT_EXT_FEEDBACK_SELECT)

// MtrOutputSel - H+ Speed Control Selector for 4-20mA or 0-10V output signal
#define TRIS_MOTOR_SELECT                       (_TRISD3)
#define LAT_MOTOR_SELECT                        (_LATD3)
#define LAT_SET_MOTOR_SELECT                    (LATDSET)
#define LAT_CLR_MOTOR_SELECT                    (LATDCLR)
#define LAT_READ_MOTOR_SELECT                   (LATD)
#define BIT_MOTOR_SELECT                        (3)
#define BITMASK_MOTOR_SELECT                    (0x0001U<<BIT_MOTOR_SELECT)

// TP1 Enable
#define TRIS_TP1                                (_TRISA6)
#define LAT_TP1                                 (_LATA6)
#define LAT_SET_TP1                             (LATASET)
#define LAT_CLR_TP1                             (LATACLR)
#define LAT_READ_TP1                            (LATA)
#define BIT_TP1                                 (6)
#define BITMASK_TP1                             (0x0001U<<BIT_TP1)

// TP6
#define TRIS_TP6                                (_TRISD12)
#define PORT_TP6                                (PORTD)
#define BIT_TP6                                 (12)
#define BITMASK_TP6                             (0x0001U<<BIT_TP6)

// Relay Watchdog
//#define TRIS_RELAY_WATCHDOG                     (_TRISD3)
//#define LAT_RELAY_WATCHDOG                      (_LATD3)
//#define LAT_SET_RELAY_WATCHDOG                  (LATDSET)
//#define LAT_CLR_RELAY_WATCHDOG                  (LATDCLR)
//#define LAT_READ_RELAY_WATCHDOG                 (LATD)
//#define BIT_RELAY_WATCHDOG                      (3)
//#define BITMAS_RELAY_WATCHDOG                   (0x0001U<<BIT_RELAY_WATCHDOG)

#endif // UC_PERIPHERAL_MAP_H
