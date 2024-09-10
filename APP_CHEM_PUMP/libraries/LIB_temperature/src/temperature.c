//------------------------------------------------------------------------------ 
//
// File:		Temperature.c        
//
// Purpose:     Routines for initialization and support of temperature reading on
//              the Graco GLC 4400 Controller.
//
// Programmer:  William R. Ockert
//				OMNI Engineering Services
//				370 West Second Street
//				Suite 100
//				Winona, MN  55987
//				507.454.5293
//
// Revision:	1/26/2009			Initial    
//
// Copyright (C) 2009 Graco Inc, All Rights Reserved
//------------------------------------------------------------------------------ 
#include "typedef.h"						// Compiler specific type definitions
#include "p32mx795f512l.h"
#include "io_pin.h"							// Switchable IO pin prototypes
#include "out_digital.h"
#include "temperature.h"


// Defines
#define TEMPERATURE_AVERAGE_COUNT   5

// The LM20 temperature sensor is relatively close to linear over its operating range of
// -55C to +130C.  Because of this rather than using the relatively complex equation that
// gives the exact temperature a simple t = m*x + b type equation can be used for a close
// approximation.  The points of interest are
//
// 130C -  303.0mV
//   0C - 1863.9mV
// -55C - 2485.0mV
//
// Looking at these from an A/D reading point of view with the 3.3V supply as the reference
// we end up with the following A/D readings
//
// 130C - (.303 / 3.3) * 1023 =    94
//   0C - (1.8639 / 3.3) * 1023 = 578
// -55C - (2.485 / 3.3) * 1023 =  770
//
// Using these values in x = m*t + b ==> t = (x - b) / m
// b = 578
// m = (-55 - 130) / (770 - 94) = -(185 / 676)
//
// The processor is fairly overpowered but lacks division so the ratio
// will be adjusted to an aritmatic shift can be used instead of division.
// Calculation will be then be done directly in fixed point arithmatic.
// The numbers work out to be few degrees at the ends of the scales, just
// fine for turning a heater on and off.
// -(185 * 512) / 676 = -140
#define TEMPERATURE_OFFSET          (578)
#define TEMPERATURE_MULTIPLIER      (-140)
#define TEMPERATURE_DATA_SHIFT      (9)

// Configuration definitions for the pic32mx795f512l
#define ADC_IDLE_CONTINUE           (0<<13)
#define ADC_FORMAT_INTG             (0b100<<8)
#define ADC_CLK_AUTO                (0b111<<5)
#define ADC_AUTO_SAMPLING_OFF       (0<<2)
#define ADC_CH0_POS_SAMPLEA_AN8     (8u<<16)
#define ADC_SCAN_OFF                (0<<10)
#define ADC_INTR_EACH_CONV          (0b0000<<2)
#define ADC_ALT_BUF_OFF             (0<<1)
#define ADC_ALT_INPUT_OFF           (0<<0)
#define ADC_CONV_CLK_SYSTEM         (1<<15)
#define ADC_SAMPLE_TIME_31          (0b11111<<8)
#define ADC_CONV_CLK_3Tcy           (0b00000001<<0)


// Private functions
static void HeaterPower(bool heaterOn);

// Private variables
static sint8 gHeaterOnTemperature = 0;
static sint8 gHeaterOffTemperature = 0;

//------------------------------------------------------------------------------ 
//  FUNCTION:       TemperatureInit ()
//
//  PARAMETERS:     heaterOn - temperature in degrees C at which to turn the heater on
//                  heaterOff - temperature in degrees C at which to turn the heater off
//
//  DESCRIPTION:    Initializes the temperature subsystem and starts the first
//                  reading
//
//  RETURNS:        None 
//
// Copyright (C) 2009 Graco Inc, All Rights Reserved
//------------------------------------------------------------------------------ 
void TEMPERATURE_Init(sint8 heaterOnTemperatureC, sint8 heaterOffTemperatureC)
{
    gHeaterOnTemperature = heaterOnTemperatureC;
    gHeaterOffTemperature = heaterOffTemperatureC;

    // Configure A/D for always on, interger output, internal counter starts conversion,
    // and auto sampling off.  This also clears done bit.
    AD1CON1 =       ADC_IDLE_CONTINUE       |
                    ADC_FORMAT_INTG         |
                    ADC_CLK_AUTO            |
                    ADC_AUTO_SAMPLING_OFF;
 
    // Set AN8 as analog input
    AD1PCFGbits.PCFG8 = 0;                      //lint !e115

    // Set to convert AN8
    AD1CHS =         ADC_CH0_POS_SAMPLEA_AN8 ;

    // Configure A/D for scan off, alternate buffer and input off as well as interupt each conversion
    //  This also sets Vref to AVdd and AVss
    AD1CON2 =       ADC_SCAN_OFF            |
                    ADC_INTR_EACH_CONV      |
                    ADC_ALT_BUF_OFF         | 
                    ADC_ALT_INPUT_OFF;

    // Set A/D from system clock, Sample time = 31Tad, Tad = 3 Tcy
    AD1CON3 =       ADC_CONV_CLK_SYSTEM     |
                    ADC_SAMPLE_TIME_31      |
                    ADC_CONV_CLK_3Tcy;      

    // Turn on A/D
    AD1CON1bits.ADON = 1;                   //lint !e115

    // Start the sample
    AD1CON1bits.SAMP = 1;                   //lint !e115
}

//------------------------------------------------------------------------------ 
//  FUNCTION:       ReadTemperatureC ()
//
//  PARAMETERS:     None
//
//  DESCRIPTION:    Reads temperature subsystem and starts the next reading
//
//  RETURNS:        INT16S      Current Temperature in Degrees C
//
// Copyright (C) 2009 Graco Inc, All Rights Reserved
//------------------------------------------------------------------------------ 
sint16 TEMPERATURE_ReadDegreesC(void)
{
    static uint16 temperatureHistory[TEMPERATURE_AVERAGE_COUNT];
    static uint8 historyCount = 0;

    uint8 i;
    sint32 meanTemperature = 0;

    // If we have no reading wait until there will be at least one
    if (!(historyCount))
    {
        while (!(AD1CON1bits.DONE));            //lint !e115
    }

    // This is done repetitively so if new reading is not done return the old one
    if (AD1CON1bits.DONE)                       //lint !e115
    {
        // Roll the readings
        for (i = 0; i < (TEMPERATURE_AVERAGE_COUNT - 1); i++)
        {
            temperatureHistory[i + 1] = temperatureHistory[i];
        }
    
        // Read in the new value
        temperatureHistory[0] = ADC1BUF0;
    
        // Start the new conversion 
        AD1CON1bits.SAMP = 1;                       //lint !e115
    
        // Average based on how many samples we have
        if (historyCount < TEMPERATURE_AVERAGE_COUNT)
        {
            historyCount++;
        }
    }
    
    // Average 
    for (i = 0; i < historyCount; i++)
    {
        meanTemperature += temperatureHistory[i];
    }

    meanTemperature /= historyCount;
    
    // Convert to Degrees C
    meanTemperature -= TEMPERATURE_OFFSET;
    meanTemperature *= TEMPERATURE_MULTIPLIER;
    
    if (meanTemperature > 0)
    {
        meanTemperature >>= TEMPERATURE_DATA_SHIFT;
    }
    else
    {
        meanTemperature = -meanTemperature;
        meanTemperature >>= TEMPERATURE_DATA_SHIFT;
        meanTemperature = -meanTemperature;
    }
    
  
    return ((sint16)meanTemperature);
}

//------------------------------------------------------------------------------ 
//  FUNCTION:       ControlLCDHeater ()
//
//  PARAMETERS:     None
//
//  DESCRIPTION:    Reads temperature and controls heater based on results
//
//  RETURNS:        bool - True if the heater is now on
//
// Copyright (C) 2009 Graco Inc, All Rights Reserved
//------------------------------------------------------------------------------ 
bool TEMPERATURE_MonitorHeat(void)
{
    sint16 Temperature;
    IOrtn_digital_t currentPower;

    Temperature = TEMPERATURE_ReadDegreesC();

    // Set heater state.  The lack of an "else" at the end of this
    // if causes hysteresis in the LCD Heater on off control
    if (Temperature < gHeaterOnTemperature)
    {
        HeaterPower(TRUE);
    }
    else if (Temperature > gHeaterOffTemperature)
    {
        HeaterPower(FALSE);
    }

    currentPower = OUT_Digital_Latch_Get(IOPIN_HEAT_EN);

    return currentPower.state;
}


//------------------------------------------------------------------------------
//  FUNCTION:       HeaterPower
//
//  PARAMETERS:     heaterOn - true if heater should turn on, false if off
//
//  DESCRIPTION:    Control the heater power
//
//  RETURNS:        void
//------------------------------------------------------------------------------
static void HeaterPower(bool heaterOn)
{
    IOState_t pinPower = (heaterOn) ? ASSERTED : NOT_ASSERTED;

    (void)OUT_Digital_Latch_Set(IOPIN_HEAT_EN, pinPower);
}
    
       
    

